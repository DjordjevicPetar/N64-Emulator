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

void VR4300::execute_next_instruction()
{
    read_next_instruction();

    bool should_branch = branch_pending_;
    u64 target = branch_target_;
    branch_pending_ = false;

    auto& instruction_entry = instruction_table_.lookup(current_instruction_);
    instruction_entry.execute(*this, current_instruction_);

    if (should_branch) {
        pc_ = target;
    }
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
