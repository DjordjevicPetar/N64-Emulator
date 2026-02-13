#include "vr4300.hpp"

namespace n64::cpu {

VR4300::VR4300(memory::MemoryMap& memory)
    : memory_(memory)
{
}

void VR4300::read_next_instruction()
{
    current_instruction_ = Instruction(read_memory<u32>(pc_));
    pc_ += 4;
}

void VR4300::delay_branch(u64 target)
{
    branch_pending_ = true;
    branch_target_ = target;
}

u32 VR4300::translate_address(u64 virtual_address) const
{
    return cp0_.translate_address(virtual_address);
}

u32 VR4300::execute_next_instruction()
{

    should_branch = branch_pending_;
    u32 cycles = 1;
    u64 target = branch_target_;
    branch_pending_ = false;

    read_next_instruction();
    u8 opcode = current_instruction_.i_type.opcode;
    // COP0 is always accessible in kernel mode; CU0 check only applies in user mode
    // Kernel mode = when EXL=0 && ERL=0 && KSU=0, OR when EXL=1 or ERL=1
    if (opcode == 0x10 && cp0_.status().cu0 == 0) {
        bool kernel_mode = (cp0_.status().exl == 1) || (cp0_.status().erl == 1) || (cp0_.status().ksu == 0);
        if (!kernel_mode) {
            cp0_.raise_exception(ExceptionCode::CPU, 0);
            return cycles;
        }
    } else if ((opcode == 0x11 || opcode == 0x31 || opcode == 0x35 || 
        opcode == 0x39 || opcode == 0x3D) && cp0_.status().cu1 == 0) {
        cp0_.raise_exception(ExceptionCode::CPU, 1);
        return cycles;
    }

    auto& instruction_entry = instruction_table_.lookup(current_instruction_);
    if (instruction_entry.execute != nullptr) {
        cycles = instruction_entry.execute(*this, current_instruction_);
    } else {
        cp0_.raise_exception(ExceptionCode::RI);
    }

    cp0_.handle_random_register();
    cp0_.handle_count_register(cycles);
    cp0_.check_interrupts();

    if (should_branch) {
        pc_ = target;
    }

    return cycles;
}

bool VR4300::check_address_exception(u64 address, u8 word_size, bool is_load)
{
    if ((address & (word_size - 1)) == 0) return false;

    if (is_load) {
        cp0_.raise_address_exception(ExceptionCode::ADEL, address);
    } else {
        cp0_.raise_address_exception(ExceptionCode::ADES, address);
    }
    return true;
}

template <typename T>
T VR4300::read_memory(u64 address) const
{
    u32 translated_address = translate_address(address);
    return memory_.read<T>(translated_address);
}

template <typename T>
void VR4300::write_memory(u64 address, T value)
{
    u32 translated_address = translate_address(address);
    memory_.write<T>(translated_address, value);
}

template u8 VR4300::read_memory<u8>(u64 address) const;
template u16 VR4300::read_memory<u16>(u64 address) const;
template u32 VR4300::read_memory<u32>(u64 address) const;
template u64 VR4300::read_memory<u64>(u64 address) const;
template void VR4300::write_memory<u8>(u64 address, u8 value);
template void VR4300::write_memory<u16>(u64 address, u16 value);
template void VR4300::write_memory<u32>(u64 address, u32 value);
template void VR4300::write_memory<u64>(u64 address, u64 value);

}
