#include "rsp.hpp"
#include "../rdp/rdp.hpp"
#include "../../memory/memory_constants.hpp"
#include <stdexcept>
#include <string>

namespace n64::rcp {

RSP::RSP(interfaces::MI& mi, rdp::RDP& rdp)
    : mi_(mi)
    , rdp_(rdp)
    , instruction_table_()
    , su_()
    , vu_()
    , dmem_()
    , imem_()
    , dma_spaddr_(0)
    , dma_ramaddr_(0)
    , dma_rdlen_(0)
    , dma_wrlen_(0)
    , status_(0)
    , dma_full_(0)
    , dma_busy_(0)
    , semaphore_(0)
    , pc_(0)
    , delay_pc_(0)
    , delay_branch_pending_(false)
{
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
            return dma_spaddr_;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RAMADDR:
            return dma_ramaddr_;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RDLEN:
            return dma_rdlen_;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_WRLEN:
            return dma_wrlen_;
        case RSP_REGISTERS_ADDRESS::RSP_STATUS:
            return status_;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_FULL:
            return dma_full_;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_BUSY:
            return dma_busy_;
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
            dma_spaddr_ = value & 0x00001FF8;
            return;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RAMADDR:
            dma_ramaddr_ = value & 0x00FFFFF8;
            return;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_RDLEN:
            dma_rdlen_ = value & 0xFF8FFFF8;
            return;
        case RSP_REGISTERS_ADDRESS::RSP_DMA_WRLEN:
            dma_wrlen_ = value & 0xFF8FFFF8;
            return;
        case RSP_REGISTERS_ADDRESS::RSP_STATUS: {
            bool set_sig7 = get_bit(value, 24);
            bool clr_sig7 = get_bit(value, 23);
            bool set_sig6 = get_bit(value, 22);
            bool clr_sig6 = get_bit(value, 21);
            bool set_sig5 = get_bit(value, 20);
            bool clr_sig5 = get_bit(value, 19);
            bool set_sig4 = get_bit(value, 18);
            bool clr_sig4 = get_bit(value, 17);
            bool set_sig3 = get_bit(value, 16);
            bool clr_sig3 = get_bit(value, 15);
            bool set_sig2 = get_bit(value, 14);
            bool clr_sig2 = get_bit(value, 13);
            bool set_sig1 = get_bit(value, 12);
            bool clr_sig1 = get_bit(value, 11);
            bool set_sig0 = get_bit(value, 10);
            bool clr_sig0 = get_bit(value, 9);
            bool set_intbreak = get_bit(value, 8);
            bool clr_intbreak = get_bit(value, 7);
            bool set_sstep = get_bit(value, 6);
            bool clr_sstep = get_bit(value, 5);
            bool set_intr = get_bit(value, 4);
            bool clr_intr = get_bit(value, 3);
            bool clr_broke = get_bit(value, 2);
            bool set_halt = get_bit(value, 1);
            bool clr_halt = get_bit(value, 0);
            if (set_sig7 && !clr_sig7) {
                status_ = set_bit(status_, 14, true);
            }
            if (clr_sig7 && !set_sig7) {
                status_ = set_bit(status_, 14, false);
            }
            if (set_sig6 && !clr_sig6) {
                status_ = set_bit(status_, 13, true);
            }
            if (clr_sig6 && !set_sig6) {
                status_ = set_bit(status_, 13, false);
            }
            if (set_sig5 && !clr_sig5) {
                status_ = set_bit(status_, 12, true);
            }
            if (clr_sig5 && !set_sig5) {
                status_ = set_bit(status_, 12, false);
            }
            if (set_sig4 && !clr_sig4) {
                status_ = set_bit(status_, 11, true);
            }
            if (clr_sig4 && !set_sig4) {
                status_ = set_bit(status_, 11, false);
            }
            if (set_sig3 && !clr_sig3) {
                status_ = set_bit(status_, 10, true);
            }
            if (clr_sig3 && !set_sig3) {
                status_ = set_bit(status_, 10, false);
            }
            if (set_sig2 && !clr_sig2) {
                status_ = set_bit(status_, 9, true);
            }
            if (clr_sig2 && !set_sig2) {
                status_ = set_bit(status_, 9, false);
            }
            if (set_sig1 && !clr_sig1) {
                status_ = set_bit(status_, 8, true);
            }
            if (clr_sig1 && !set_sig1) {
                status_ = set_bit(status_, 8, false);
            }
            if (set_sig0 && !clr_sig0) {
                status_ = set_bit(status_, 7, true);
            }
            if (clr_sig0 && !set_sig0) {
                status_ = set_bit(status_, 7, false);
            }
            if (set_intbreak && !clr_intbreak) {
                status_ = set_bit(status_, 6, true);
            }
            if (clr_intbreak && !set_intbreak) {
                status_ = set_bit(status_, 6, false);
            }
            if (set_sstep && !clr_sstep) {
                status_ = set_bit(status_, 5, true);
            }
            if (clr_sstep && !set_sstep) {
                status_ = set_bit(status_, 5, false);
            }
            if (set_intr && !clr_intr) {
                mi_.set_interrupt(interfaces::MI_INTERRUPT_SP);
            }
            if (clr_intr && !set_intr) {
                mi_.clear_interrupt(interfaces::MI_INTERRUPT_SP);
            }
            if (clr_broke) {
                status_ = set_bit(status_, 1, false);
            }
            if (set_halt && !clr_halt) {
                status_ = set_bit(status_, 0, true);
            }
            if (clr_halt && !set_halt) {
                status_ = set_bit(status_, 0, false);
            }
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
    auto instruction = fetch_instruction();
    bool should_branch = delay_branch_pending_;
    u32 target = delay_pc_;
    delay_branch_pending_ = false;

    auto& instruction_entry = instruction_table_.lookup(instruction);
    instruction_entry.execute(*this, instruction);

    if (should_branch) {
        pc_ = target;
    }
}

void RSP::delay_branch(u32 target)
{
    delay_branch_pending_ = true;
    delay_pc_ = target;
}

void RSP::set_breakpoint()
{
    status_ = set_bit(status_, 1, true);
}

u32 RSP::read_cop0(u32 reg) const
{
    switch (reg) {
        case 0: return dma_spaddr_;
        case 1: return dma_ramaddr_;
        case 2: return dma_rdlen_;
        case 3: return dma_wrlen_;
        case 4: return status_;
        case 5: return dma_full_;
        case 6: return dma_busy_;
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
        case 0: dma_spaddr_ = value & 0x00001FF8; break;
        case 1: dma_ramaddr_ = value & 0x00FFFFF8; break;
        case 2: dma_rdlen_ = value & 0xFF8FFFF8; break;
        case 3: dma_wrlen_ = value & 0xFF8FFFF8; break;
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