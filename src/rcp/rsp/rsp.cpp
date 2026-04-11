#include "rsp.hpp"
#include "../rdp/rdp.hpp"
#include "../../memory/memory_constants.hpp"
#include <cstdio>

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
    , pending_spmem_(0)
    , pending_rdram_(0)
    , current_spmem_(0)
    , current_rdram_(0)
    , current_len_(0xFF8)
    , status_{.raw = 0}
    , semaphore_(0)
    , pc_(0)
    , delay_pc_(0)
    , delay_branch_pending_(false)
    , dma_(*this, rdram)
{
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

    return T{};
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

    return;
}

u32 RSP::read_register(u32 address) const {
    switch (address) {
        case RSP_REGISTERS_ADDRESS::RSP_DMA_SPADDR:
            return current_spmem_;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RAMADDR:
            return current_rdram_;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RDLEN:
        case RSP_REGISTERS_ADDRESS::RSP_DMA_WRLEN:
            return current_len_;
        case RSP_REGISTERS_ADDRESS::RSP_STATUS:
            return status_.raw;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_FULL:
            return status_.dma_full;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_BUSY:
            return status_.dma_busy;
        case RSP_REGISTERS_ADDRESS::RSP_SEMAPHORE: {
            u32 val = semaphore_;
            semaphore_ = 1;
            return val;
        }
        case RSP_REGISTERS_ADDRESS::RSP_PC:
            return pc_;
        default:
            return 0;
    }
}

void RSP::write_register(u32 address, u32 value) {
    switch (address) {
        case RSP_REGISTERS_ADDRESS::RSP_DMA_SPADDR:
            pending_spmem_ = value & 0x00001FF8;
            return;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RAMADDR:
            pending_rdram_ = value & 0x00FFFFF8;
            return;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RDLEN: {
            RSPDmaLen len;
            len.raw = value & 0xFF8FFFF8;
            current_spmem_ = pending_spmem_;
            current_rdram_ = pending_rdram_;
            bool is_imem = (pending_spmem_ >> 12) & 1;
            u32 sp_addr = pending_spmem_ & 0xFFF;
            u32 rdram_addr = pending_rdram_ & 0xFFFFFF;
            dma_.add_request(DMARequest(
                true,                       // is_read: RDRAM -> SP
                is_imem,
                sp_addr,
                rdram_addr,
                len.length + 8,             // 8-byte aligned transfer size
                len.count,
                len.skip
            ));
            return;
        }
        case RSP_REGISTERS_ADDRESS::RSP_DMA_WRLEN: {
            RSPDmaLen len;
            len.raw = value & 0xFF8FFFF8;
            current_spmem_ = pending_spmem_;
            current_rdram_ = pending_rdram_;
            bool is_imem = (pending_spmem_ >> 12) & 1;
            u32 sp_addr = pending_spmem_ & 0xFFF;
            u32 rdram_addr = pending_rdram_ & 0xFFFFFF;
            dma_.add_request(DMARequest(
                false,                      // is_read: SP -> RDRAM
                is_imem,
                sp_addr,
                rdram_addr,
                len.length + 8,             // 8-byte aligned transfer size
                len.count,
                len.skip
            ));
            return;
        }
        case RSP_REGISTERS_ADDRESS::RSP_STATUS: {
            bool was_halted = status_.halt;
            // clr/set pairs - if both set, no change
            clear_set_resolver(status_.raw, get_bit(value, 0),  get_bit(value, 1),  0);  // halt
            if (get_bit(value, 2)) status_.broke = 0;  // clear broke (no set pair)
            if (get_bit(value, 3) && !get_bit(value, 4)) mi_.clear_interrupt(interfaces::MI_INTERRUPT_SP);
            if (get_bit(value, 4) && !get_bit(value, 3)) mi_.set_interrupt(interfaces::MI_INTERRUPT_SP);
            clear_set_resolver(status_.raw, get_bit(value, 5),  get_bit(value, 6),  5);  // single_step
            clear_set_resolver(status_.raw, get_bit(value, 7),  get_bit(value, 8),  6);  // interrupt_on_break
            clear_set_resolver(status_.raw, get_bit(value, 9),  get_bit(value, 10), 7);  // signal0
            clear_set_resolver(status_.raw, get_bit(value, 11), get_bit(value, 12), 8);  // signal1
            clear_set_resolver(status_.raw, get_bit(value, 13), get_bit(value, 14), 9);  // signal2
            clear_set_resolver(status_.raw, get_bit(value, 15), get_bit(value, 16), 10); // signal3
            clear_set_resolver(status_.raw, get_bit(value, 17), get_bit(value, 18), 11); // signal4
            clear_set_resolver(status_.raw, get_bit(value, 19), get_bit(value, 20), 12); // signal5
            clear_set_resolver(status_.raw, get_bit(value, 21), get_bit(value, 22), 13); // signal6
            clear_set_resolver(status_.raw, get_bit(value, 23), get_bit(value, 24), 14); // signal7
            if (was_halted && !status_.halt)
                fprintf(stderr, "[RSP] Started (halt cleared), PC=0x%03X\n", pc_);
            return;
        }
        case RSP_REGISTERS_ADDRESS::RSP_SEMAPHORE:
            semaphore_ = 0;
            return;
        case RSP_REGISTERS_ADDRESS::RSP_PC:
            pc_ = value & 0x00000FFC;
            return;
        default:
            return;
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
    if (instruction_entry.execute) {
        instruction_entry.execute(*this, instruction);
    } else {
        static u32 rsp_ri_count = 0;
        if (rsp_ri_count++ < 10)
            fprintf(stderr, "[RSP] Unimplemented instr=0x%08X op=%u rs=%u rt=%u funct=%u PC=0x%03X\n",
                    instruction.raw, (unsigned)instruction.i_type.opcode,
                    (unsigned)instruction.r_type.rs, (unsigned)instruction.r_type.rt,
                    (unsigned)instruction.r_type.funct, (pc_ - 4) & 0xFFF);
    }

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
    delay_pc_ = target & 0xFFF;
}

void RSP::set_breakpoint()
{
    status_.halt = 1;
    status_.broke = 1;
    fprintf(stderr, "[RSP] BREAK at PC=0x%03X, interrupt_on_break=%u\n",
            pc_, (unsigned)status_.interrupt_on_break);
    if (status_.interrupt_on_break) {
        mi_.set_interrupt(interfaces::MI_INTERRUPT_SP);
    }
}

u32 RSP::read_cop0(u32 reg) const
{
    switch (reg) {
        case 0: return current_spmem_;
        case 1: return current_rdram_;
        case 2: return current_len_;
        case 3: return current_len_;
        case 4: return status_.raw;
        case 5: return status_.dma_full;
        case 6: return status_.dma_busy;
        case 7: {
            u32 val = semaphore_;
            semaphore_ = 1;
            return val;
        }
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
        instruction = (instruction << 8) | imem_[pc_];
        pc_ = (pc_ + 1) & 0xFFF;
    }
    return RSPInstruction(instruction);
}

void RSP::on_dma_complete(u32 final_sp_addr, u32 final_rdram_addr, bool is_imem, u32 skip)
{
    current_spmem_ = (static_cast<u32>(is_imem) << 12) | (final_sp_addr & 0xFF8);
    current_rdram_ = final_rdram_addr & 0x00FFFFF8;
    current_len_ = (skip << 20) | 0xFF8;
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