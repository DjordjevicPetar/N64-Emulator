#include "rsp.hpp"
#include "../rdp/rdp.hpp"
#include "../../memory/memory_constants.hpp"
#include <stdexcept>
#include <string>

// TODO: RSP needs work on cycle accuracy:
// TODO: Verify exact CPU-to-RSP cycle ratio (2/3) - current float accumulation may drift
// TODO: DMA should run in parallel with RSP execution, not sequentially
// TODO: Many VU instructions may take multiple cycles - verify cycle counts
// TODO: Verify RSP halt/unhalt timing behavior
// TODO: Implement RSP breakpoint behavior more accurately

namespace n64::rcp {

RSP::RSP(interfaces::MI& mi, rdp::RDP& rdp, memory::RDRAM& rdram)
    : mi_(mi)
    , rdp_(rdp)
    , instruction_table_()
    , su_()
    , vu_()
    , dmem_()
    , imem_()
    , dma_spaddr_{.raw = 0}
    , dma_ramaddr_{.raw = 0}
    , dma_rdlen_{.raw = 0}
    , dma_wrlen_{.raw = 0}
    , status_{.raw = 0}
    , semaphore_(0)
    , pc_(0)
    , delay_pc_(0)
    , delay_branch_pending_(false)
    , dma_(*this, rdram)
{
    // RSP starts halted - CPU must explicitly start it
    status_.halt = 1;
}
RSP::~RSP() {}

template<typename T>
T RSP::read(u32 address) const {

    if (address >= memory::RSP_DATA_MEMORY_START_ADDRESS && address <= memory::RSP_DATA_MEMORY_END_ADDRESS) {
        T result = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            result = (result << 8) | dmem_[address - memory::RSP_DATA_MEMORY_START_ADDRESS + i];
        }
        return result;
    }
    if (address >= memory::RSP_INSTRUCTION_MEMORY_START_ADDRESS && address <= memory::RSP_INSTRUCTION_MEMORY_END_ADDRESS) {
        T result = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            result = (result << 8) | imem_[address - memory::RSP_INSTRUCTION_MEMORY_START_ADDRESS + i];
        }
        return result;
    }
    if (address >= memory::RSP_REGISTER_START_ADDRESS && address <= memory::RSP_REGISTER_END_ADDRESS) {
        return read_register(address);
    }   

    throw std::runtime_error("Invalid RSP address: " + std::to_string(address));
}

template<typename T>
void RSP::write(u32 address, T value) {
    if (address >= memory::RSP_DATA_MEMORY_START_ADDRESS && address <= memory::RSP_DATA_MEMORY_END_ADDRESS) {
        for (size_t i = 0; i < sizeof(T); i++) {
            dmem_[address - memory::RSP_DATA_MEMORY_START_ADDRESS + i] = static_cast<u8>(value >> ((sizeof(T) - 1 - i) * 8));
        }
        return;
    }
    if (address >= memory::RSP_INSTRUCTION_MEMORY_START_ADDRESS && address <= memory::RSP_INSTRUCTION_MEMORY_END_ADDRESS) {
        for (size_t i = 0; i < sizeof(T); i++) {
            imem_[address - memory::RSP_INSTRUCTION_MEMORY_START_ADDRESS + i] = static_cast<u8>(value >> ((sizeof(T) - 1 - i) * 8));
        }
        return;
    }
    if (address >= memory::RSP_REGISTER_START_ADDRESS && address <= memory::RSP_REGISTER_END_ADDRESS) {
        write_register(address, value);
        return;
    }

    throw std::runtime_error("Invalid RSP address: " + std::to_string(address));
}

u32 RSP::read_register(u32 address) const {
    switch (address) {
        case RSP_REGISTERS_ADDRESS::RSP_DMA_SPADDR:
            return dma_spaddr_.raw;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RAMADDR:
            return dma_ramaddr_.raw;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RDLEN:
            return dma_rdlen_.raw;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_WRLEN:
            return dma_wrlen_.raw;
        case RSP_REGISTERS_ADDRESS::RSP_STATUS:
            return status_.raw;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_FULL:
            return status_.dma_full;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_BUSY:
            return status_.dma_busy;
        case RSP_REGISTERS_ADDRESS::RSP_SEMAPHORE:
            return semaphore_;
        case RSP_REGISTERS_ADDRESS::RSP_PC:
            return pc_;
        default:
            throw std::runtime_error("Invalid RSP register address: " + std::to_string(address));
    }
}

void RSP::write_register(u32 address, u32 value) {
    switch (address) {
        case RSP_REGISTERS_ADDRESS::RSP_DMA_SPADDR:
            dma_spaddr_.raw = value & 0x00001FF8;  // 8-byte aligned
            return;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RAMADDR:
            dma_ramaddr_.raw = value & 0x00FFFFF8;  // 8-byte aligned
            return;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RDLEN:
            dma_rdlen_.raw = value;
            dma_.add_request(DMARequest(
                true,                       // is_read: RDRAM -> SP
                dma_spaddr_.bank,           // is_imem
                dma_spaddr_.address,        // sp_address
                dma_ramaddr_.address,       // rdram_address  
                dma_rdlen_.length + 1,      // start_length
                dma_rdlen_.count + 1,       // count
                dma_rdlen_.skip             // skip
            ));
            return;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_WRLEN:
            dma_wrlen_.raw = value;
            dma_.add_request(DMARequest(
                false,                      // is_read: SP -> RDRAM
                dma_spaddr_.bank,           // is_imem
                dma_spaddr_.address,        // sp_address
                dma_ramaddr_.address,       // rdram_address
                dma_wrlen_.length + 1,      // start_length
                dma_wrlen_.count + 1,       // count
                dma_wrlen_.skip             // skip
            ));
            return;
        case RSP_REGISTERS_ADDRESS::RSP_STATUS: {
            // clr/set pairs - if both set, no change
            if (get_bit(value, 0) && !get_bit(value, 1)) status_.halt = 0;
            if (get_bit(value, 1) && !get_bit(value, 0)) status_.halt = 1;
            if (get_bit(value, 2)) status_.broke = 0;
            if (get_bit(value, 3) && !get_bit(value, 4)) mi_.clear_interrupt(interfaces::MI_INTERRUPT_SP);
            if (get_bit(value, 4) && !get_bit(value, 3)) mi_.set_interrupt(interfaces::MI_INTERRUPT_SP);
            if (get_bit(value, 5) && !get_bit(value, 6)) status_.single_step = 0;
            if (get_bit(value, 6) && !get_bit(value, 5)) status_.single_step = 1;
            if (get_bit(value, 7) && !get_bit(value, 8)) status_.interrupt_on_break = 0;
            if (get_bit(value, 8) && !get_bit(value, 7)) status_.interrupt_on_break = 1;
            if (get_bit(value, 9) && !get_bit(value, 10)) status_.signal0 = 0;
            if (get_bit(value, 10) && !get_bit(value, 9)) status_.signal0 = 1;
            if (get_bit(value, 11) && !get_bit(value, 12)) status_.signal1 = 0;
            if (get_bit(value, 12) && !get_bit(value, 11)) status_.signal1 = 1;
            if (get_bit(value, 13) && !get_bit(value, 14)) status_.signal2 = 0;
            if (get_bit(value, 14) && !get_bit(value, 13)) status_.signal2 = 1;
            if (get_bit(value, 15) && !get_bit(value, 16)) status_.signal3 = 0;
            if (get_bit(value, 16) && !get_bit(value, 15)) status_.signal3 = 1;
            if (get_bit(value, 17) && !get_bit(value, 18)) status_.signal4 = 0;
            if (get_bit(value, 18) && !get_bit(value, 17)) status_.signal4 = 1;
            if (get_bit(value, 19) && !get_bit(value, 20)) status_.signal5 = 0;
            if (get_bit(value, 20) && !get_bit(value, 19)) status_.signal5 = 1;
            if (get_bit(value, 21) && !get_bit(value, 22)) status_.signal6 = 0;
            if (get_bit(value, 22) && !get_bit(value, 21)) status_.signal6 = 1;
            if (get_bit(value, 23) && !get_bit(value, 24)) status_.signal7 = 0;
            if (get_bit(value, 24) && !get_bit(value, 23)) status_.signal7 = 1;
            return;
        }
        case RSP_REGISTERS_ADDRESS::RSP_SEMAPHORE:
            semaphore_ = value & 0x00000001;
            return;
        case RSP_REGISTERS_ADDRESS::RSP_PC:
            pc_ = value & 0x00000FFC;
            return;
        default:
            throw std::runtime_error("Invalid RSP register address: " + std::to_string(address));
    }
}

void RSP::execute_next_instruction()
{
    if (status_.halt) {
        return;
    }
    
    auto instruction = fetch_instruction();
    
    bool should_branch = delay_branch_pending_;
    u32 target = delay_pc_;
    delay_branch_pending_ = false;

    auto& instruction_entry = instruction_table_.lookup(instruction);
    instruction_entry.execute(*this, instruction);

    if (should_branch) {
        pc_ = target;
    }

    if (status_.single_step) {
        status_.halt = 1;
    }
}

void RSP::process_passed_cycles(u32 cycles)
{
    // DMA i RSP rade paralelno
    dma_.process_transfer(cycles);
    
    if (!status_.halt) {
        // RSP radi na 2/3 brzine CPU-a
        rsp_cycle_accumulator_ += cycles * (2.0f / 3.0f);
        
        while (rsp_cycle_accumulator_ >= 1.0f) {
            rsp_cycle_accumulator_ -= 1.0f;
            execute_next_instruction();
            
            // Proveri da li je RSP haltovan (BREAK)
            if (status_.halt) break;
        }
    }
}

void RSP::delay_branch(u32 target)
{
    delay_branch_pending_ = true;
    delay_pc_ = target;
}

void RSP::set_breakpoint()
{
    status_.halt = 1;
    status_.broke = 1;
    if (status_.interrupt_on_break) {
        mi_.set_interrupt(interfaces::MI_INTERRUPT_SP);
    }
}

u32 RSP::read_cop0(u32 reg) const
{
    switch (reg) {
        case 0: return dma_spaddr_.raw;
        case 1: return dma_ramaddr_.raw;
        case 2: return dma_rdlen_.raw;
        case 3: return dma_wrlen_.raw;
        case 4: return status_.raw;
        case 5: return status_.dma_full;
        case 6: return status_.dma_busy;
        case 7: return semaphore_;
        // RDP registers (8-15)
        case 8:  return rdp_.read_register(rdp::DPC_START);
        case 9:  return rdp_.read_register(rdp::DPC_END);
        case 10: return rdp_.read_register(rdp::DPC_CURRENT);
        case 11: return rdp_.read_register(rdp::DPC_STATUS);
        case 12: return rdp_.read_register(rdp::DPC_CLOCK);
        case 13: return rdp_.read_register(rdp::DPC_BUF_BUSY);
        case 14: return rdp_.read_register(rdp::DPC_PIPE_BUSY);
        case 15: return rdp_.read_register(rdp::DPC_TMEM_BUSY);
        default: return 0;
    }
}

void RSP::write_cop0(u32 reg, u32 value)
{
    switch (reg) {
        case 0: write_register(RSP_DMA_SPADDR, value); break;  // 8-byte aligned
        case 1: write_register(RSP_DMA_RAMADDR, value); break; // 8-byte aligned
        case 2: write_register(RSP_DMA_RDLEN, value); break;  // triggers DMA
        case 3: write_register(RSP_DMA_WRLEN, value); break;  // triggers DMA
        case 4: write_register(RSP_STATUS, value); break;
        case 7: semaphore_ = value & 0x00000001; break;
        // RDP registers (8-15)
        case 8:  rdp_.write_register(rdp::DPC_START, value); break;
        case 9:  rdp_.write_register(rdp::DPC_END, value); break;
        case 11: rdp_.write_register(rdp::DPC_STATUS, value); break;
        // Registers 5, 6, 10, 12-15 are read-only
        default: break;
    }
}

RSPInstruction RSP::fetch_instruction()
{
    u32 instruction = 0;
    for (size_t i = 0; i < sizeof(u32); i++) {
        instruction = (instruction << 8) | imem_[pc_ + i];
    }
    pc_ += 4;
    return RSPInstruction(instruction);
}

template u8 RSP::read<u8>(u32) const;
template u16 RSP::read<u16>(u32) const;
template u32 RSP::read<u32>(u32) const;
template u64 RSP::read<u64>(u32) const;
template void RSP::write<u8>(u32, u8);
template void RSP::write<u16>(u32, u16);
template void RSP::write<u32>(u32, u32);
template void RSP::write<u64>(u32, u64);

} // namespace n64::rcp