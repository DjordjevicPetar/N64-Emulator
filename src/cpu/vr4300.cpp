#include "vr4300.hpp"

namespace n64::cpu {

VR4300::VR4300(Memory& memory)
    : memory_(memory)
{
}

void VR4300::read_next_instruction()
{
    current_instruction_ = Instruction(read_memory(pc_, IO_SIZE::IO_SIZE_WORD));
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

u64 VR4300::read_memory(u64 address, IO_SIZE size) const
{
    u32 translated_address = translate_address(address);
    return memory_.read_memory(translated_address, size);
}

void VR4300::write_memory(u64 address, u64 value, IO_SIZE size)
{
    u32 translated_address = translate_address(address);
    memory_.write_memory(translated_address, value, size);
}

}
