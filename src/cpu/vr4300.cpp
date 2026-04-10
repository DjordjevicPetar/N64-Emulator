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

u32 VR4300::translate_address(u64 virtual_address, bool is_write)
{
    return cp0_.translate_address(virtual_address, is_write);
}

u32 VR4300::execute_next_instruction()
{
    exception_pending_ = false;
    should_branch = branch_pending_;
    u32 cycles = 1;
    u64 target = branch_target_;
    branch_pending_ = false;

    read_next_instruction();

    static u32 consecutive_nops = 0;
    static bool nop_slide_logged = false;
    static u64 last_real_pc = 0;
    static u32 last_real_instr = 0;
    if (current_instruction_.raw == 0x00000000) {
        consecutive_nops++;
        if (consecutive_nops == 16 && !nop_slide_logged) {
            nop_slide_logged = true;
            fprintf(stderr, "[NOP-SLIDE] Detected at PC=0x%08llX (16 consecutive NOPs)\n"
                    "  Last real instruction: PC=0x%08llX instr=0x%08X\n"
                    "  $ra=0x%08llX $sp=0x%08llX $at=0x%08llX\n"
                    "  $t0=0x%08llX $t1=0x%08llX $v0=0x%08llX\n",
                    (unsigned long long)(pc_ - 4),
                    (unsigned long long)last_real_pc, last_real_instr,
                    (unsigned long long)gpr_[31], (unsigned long long)gpr_[29],
                    (unsigned long long)gpr_[1],
                    (unsigned long long)gpr_[8], (unsigned long long)gpr_[9],
                    (unsigned long long)gpr_[2]);
        }
    } else {
        if (consecutive_nops >= 16 && nop_slide_logged) {
            fprintf(stderr, "[NOP-SLIDE] Ended at PC=0x%08llX after %u NOPs, "
                    "now instr=0x%08X\n",
                    (unsigned long long)(pc_ - 4), consecutive_nops,
                    current_instruction_.raw);
            nop_slide_logged = false;
        }
        consecutive_nops = 0;
        last_real_pc = pc_ - 4;
        last_real_instr = current_instruction_.raw;
    }

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
        static u32 ri_count = 0;
        if (ri_count++ < 10)
            fprintf(stderr, "[RI] Unimplemented instr=0x%08X op=%u rs=%u rt=%u funct=%u PC=0x%08llX\n",
                    current_instruction_.raw, (unsigned)current_instruction_.i_type.opcode,
                    (unsigned)current_instruction_.i_type.rs, (unsigned)current_instruction_.i_type.rt,
                    (unsigned)current_instruction_.r_type.funct, (unsigned long long)(pc_ - 4));
        cp0_.raise_exception(ExceptionCode::RI);
    }

    cp0_.handle_random_register();
    cp0_.handle_count_register(cycles);

    if (exception_pending_) {
        // An exception was raised during instruction execution (TLB miss,
        // address error, etc.). Don't check interrupts or take branches -
        // the PC is already set to the exception vector.
        exception_pending_ = false;
        interrupt_inhibit_ = false;
    } else if (interrupt_inhibit_) {
        interrupt_inhibit_ = false;
        if (should_branch) {
            pc_ = target;
        }
    } else {
        cp0_.check_interrupts();
        if (should_branch) {
            pc_ = target;
        }
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
T VR4300::read_memory(u64 address)
{
    if (exception_pending_) return T{0};
    u32 translated_address = translate_address(address, false);
    if (exception_pending_) return T{0};
    return memory_.read<T>(translated_address);
}

template <typename T>
void VR4300::write_memory(u64 address, T value)
{
    if (exception_pending_) return;
    u32 translated_address = translate_address(address, true);
    if (exception_pending_) return;
    memory_.write<T>(translated_address, value);
}

template u8 VR4300::read_memory<u8>(u64 address);
template u16 VR4300::read_memory<u16>(u64 address);
template u32 VR4300::read_memory<u32>(u64 address);
template u64 VR4300::read_memory<u64>(u64 address);
template void VR4300::write_memory<u8>(u64 address, u8 value);
template void VR4300::write_memory<u16>(u64 address, u16 value);
template void VR4300::write_memory<u32>(u64 address, u32 value);
template void VR4300::write_memory<u64>(u64 address, u64 value);

}
