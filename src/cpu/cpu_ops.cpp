#include "cpu_ops.hpp"
#include "vr4300.hpp"
#include "cp0.hpp"
#include <climits>
#include <cmath>
#include <cfenv>

// TODO: CPU instruction implementation needs work:
// 
// MISSING INSTRUCTIONS:
// TODO: Implement full FPU instruction set (~40 instructions) - LWC1, SWC1, FPU arithmetic
// TODO: Implement trap instructions with exception generation (TGE, TLT, TEQ, etc.)
// TODO: Implement TLB instructions (TLBR, TLBWI, TLBWR, TLBP)
// TODO: Implement CACHE instruction with actual cache operations
//
// EXCEPTION HANDLING:
// TODO: Implement overflow exception for ADDI, ADD, SUB, DADDI, DADD, DSUB
// TODO: Implement address error exceptions for unaligned memory access
// TODO: Implement TLB miss exceptions
// TODO: Implement SYSCALL and BREAK exceptions properly
// TODO: Implement exception framework (save EPC, set CAUSE, update STATUS)
//
// CYCLE ACCURACY:
// TODO: Verify cycle counts for all instructions
// TODO: Add memory access latency cycles (cache hit/miss)
// TODO: Implement pipeline stalls for multiply/divide (MULT=3, DIV=35, etc.)
// TODO: Branch taken vs not taken may have different cycle counts

namespace n64::cpu {

// ============================================================================
// Main Table Instructions (by opcode)
// ============================================================================

// Jump instructions
u32 J(VR4300& cpu, const Instruction& instr) {
    u64 new_pc = set_bits(cpu.pc(), 27, 0, instr.j_type.target_address << 2);
    cpu.delay_branch(new_pc);
    return 1;
}
    
u32 JAL(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(31, cpu.pc() + 8);
    u64 new_pc = set_bits(cpu.pc(), 27, 0, instr.j_type.target_address << 2);
    cpu.delay_branch(new_pc);
    return 1;
}

// Branch instructions
u32 BEQ(VR4300& cpu, const Instruction& instr) {
    if (cpu.gpr(instr.i_type.rs) == cpu.gpr(instr.i_type.rt)) {
        u64 new_pc = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
        cpu.delay_branch(new_pc);
    }
    return 1;
}

u32 BNE(VR4300& cpu, const Instruction& instr) {
    if (cpu.gpr(instr.i_type.rs) != cpu.gpr(instr.i_type.rt)) {
        u64 new_pc = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
        cpu.delay_branch(new_pc);
    }
    return 1;
}

u32 BLEZ(VR4300& cpu, const Instruction& instr) {
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) <= 0) {
        u64 new_pc = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
        cpu.delay_branch(new_pc);
    }
    return 1;
}

u32 BGTZ(VR4300& cpu, const Instruction& instr) {
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) > 0) {
        u64 new_pc = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
        cpu.delay_branch(new_pc);
    }
    return 1;
}

u32 BEQL(VR4300& cpu, const Instruction& instr) {
    if (cpu.gpr(instr.i_type.rs) == cpu.gpr(instr.i_type.rt)) {
        u64 new_pc = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
        cpu.delay_branch(new_pc);
    } else {
        cpu.set_pc(cpu.pc() + 4);
    }
    return 1;
}

u32 BNEL(VR4300& cpu, const Instruction& instr) {
    if (cpu.gpr(instr.i_type.rs) != cpu.gpr(instr.i_type.rt)) {
        u64 new_pc = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
        cpu.delay_branch(new_pc);
    } else {
        cpu.set_pc(cpu.pc() + 4);
    }
    return 1;
}

u32 BLEZL(VR4300& cpu, const Instruction& instr) {
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) <= 0) {
        u64 new_pc = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
        cpu.delay_branch(new_pc);
    } else {
        cpu.set_pc(cpu.pc() + 4);
    }
    return 1;
}

u32 BGTZL(VR4300& cpu, const Instruction& instr) {
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) > 0) {
        u64 new_pc = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
        cpu.delay_branch(new_pc);
    } else {
        cpu.set_pc(cpu.pc() + 4);
    }
    return 1;
}

// Arithmetic immediate
u32 ADDI(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement overflow exception
    s32 rs = static_cast<s32>(cpu.gpr(instr.i_type.rs));
    s32 imm = static_cast<s16>(instr.i_type.immediate);
    s32 result = rs + imm;
    cpu.set_gpr(instr.i_type.rt, sign_extend32(static_cast<u32>(result)));
    return 1;
}

u32 ADDIU(VR4300& cpu, const Instruction& instr) {
    s32 rs = static_cast<s32>(cpu.gpr(instr.i_type.rs));
    s32 imm = static_cast<s16>(instr.i_type.immediate);
    s32 result = rs + imm;
    cpu.set_gpr(instr.i_type.rt, sign_extend32(static_cast<u32>(result)));
    return 1;
}

u32 SLTI(VR4300& cpu, const Instruction& instr) {
    s64 rs = static_cast<s64>(cpu.gpr(instr.i_type.rs));
    s64 imm = sign_extend16(instr.i_type.immediate);
    cpu.set_gpr(instr.i_type.rt, rs < imm ? 1 : 0);
    return 1;
}

u32 SLTIU(VR4300& cpu, const Instruction& instr) {
    u64 rs = cpu.gpr(instr.i_type.rs);
    u64 imm = static_cast<u64>(sign_extend16(instr.i_type.immediate));
    cpu.set_gpr(instr.i_type.rt, rs < imm ? 1 : 0);
    return 1;
}

u32 DADDI(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement overflow exception
    s64 rs = static_cast<s64>(cpu.gpr(instr.i_type.rs));
    s64 imm = sign_extend16(instr.i_type.immediate);
    cpu.set_gpr(instr.i_type.rt, static_cast<u64>(rs + imm));
    return 1;
}

u32 DADDIU(VR4300& cpu, const Instruction& instr) {
    s64 rs = static_cast<s64>(cpu.gpr(instr.i_type.rs));
    s64 imm = sign_extend16(instr.i_type.immediate);
    cpu.set_gpr(instr.i_type.rt, static_cast<u64>(rs + imm));
    return 1;
}

// Logical immediate
u32 ANDI(VR4300& cpu, const Instruction& instr) {
    u64 result = cpu.gpr(instr.i_type.rs) & extend16(instr.i_type.immediate);
    cpu.set_gpr(instr.i_type.rt, result);
    return 1;
}

u32 ORI(VR4300& cpu, const Instruction& instr) {
    u64 result = cpu.gpr(instr.i_type.rs) | extend16(instr.i_type.immediate);
    cpu.set_gpr(instr.i_type.rt, result);
    return 1;
}

u32 XORI(VR4300& cpu, const Instruction& instr) {
    u64 result = cpu.gpr(instr.i_type.rs) ^ extend16(instr.i_type.immediate);
    cpu.set_gpr(instr.i_type.rt, result);
    return 1;
}

u32 LUI(VR4300& cpu, const Instruction& instr) {
    // Load upper immediate: imm << 16, then sign-extend to 64-bit
    u32 upper = static_cast<u32>(instr.i_type.immediate) << 16;
    cpu.set_gpr(instr.i_type.rt, sign_extend32(upper));
    return 1;
}

// Load instructions
u32 LB(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 value = cpu.read_memory<u8>(address);
    if (cpu.check_address_exception(address, 1, true)) return 1;
    cpu.set_gpr(instr.i_type.rt, sign_extend8(value));
    return 1;
}

u32 LBU(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 value = cpu.read_memory<u8>(address);
    if (cpu.check_address_exception(address, 1, true)) return 1;
    cpu.set_gpr(instr.i_type.rt, extend8(value));
    return 1;
}

u32 LH(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 2, true)) return 1;
    u64 value = cpu.read_memory<u16>(address);
    cpu.set_gpr(instr.i_type.rt, sign_extend16(value));
    return 1;
}

u32 LHU(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 2, true)) return 1;
    u64 value = cpu.read_memory<u16>(address);
    cpu.set_gpr(instr.i_type.rt, extend16(value));
    return 1;
}

u32 LW(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 4, true)) return 1;
    u64 value = cpu.read_memory<u32>(address);
    cpu.set_gpr(instr.i_type.rt, sign_extend32(value));
    return 1;
}

u32 LWU(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 4, true)) return 1;
    u64 value = cpu.read_memory<u32>(address);
    cpu.set_gpr(instr.i_type.rt, extend32(value));
    return 1;
}

u32 LWL(VR4300& cpu, const Instruction& instr) {
    // Load Word Left - učitava bajtove u GORNJI deo registra
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(3);
    u32 offset = static_cast<u32>(addr & 3);
    
    u32 mem = static_cast<u32>(cpu.read_memory<u32>(aligned));
    u32 rt = static_cast<u32>(cpu.gpr(instr.i_type.rt));
    u32 result = 0;
    
    switch (offset) {
        case 0: result = mem; break;
        case 1: result = (mem << 8)  | (rt & 0x000000FF); break;
        case 2: result = (mem << 16) | (rt & 0x0000FFFF); break;
        case 3: result = (mem << 24) | (rt & 0x00FFFFFF); break;
    }
    
    cpu.set_gpr(instr.i_type.rt, sign_extend32(result));
    return 1;
}

u32 LWR(VR4300& cpu, const Instruction& instr) {
    // Load Word Right - učitava bajtove u DONJI deo registra
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(3);
    u32 offset = static_cast<u32>(addr & 3);
    
    u32 mem = static_cast<u32>(cpu.read_memory<u32>(aligned));
    u32 rt = static_cast<u32>(cpu.gpr(instr.i_type.rt));
    u32 result = 0;
    
    switch (offset) {
        case 0: result = (rt & 0xFFFFFF00) | (mem >> 24); break;
        case 1: result = (rt & 0xFFFF0000) | (mem >> 16); break;
        case 2: result = (rt & 0xFF000000) | (mem >> 8); break;
        case 3: result = mem; break;
    }
    
    cpu.set_gpr(instr.i_type.rt, sign_extend32(result));
    return 1;
}

u32 LD(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 8, true)) return 1;
    u64 value = cpu.read_memory<u64>(address);
    cpu.set_gpr(instr.i_type.rt, value);
    return 1;
}

u32 LDL(VR4300& cpu, const Instruction& instr) {
    // Load Doubleword Left - učitava bajtove u GORNJI deo registra
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(7);  // Poravnaj na 8 bajtova
    u32 offset = static_cast<u32>(addr & 7);
    
    u64 mem = cpu.read_memory<u64>(aligned);
    u64 rt = cpu.gpr(instr.i_type.rt);
    u64 result = 0;
    
    switch (offset) {
        case 0: result = mem; break;
        case 1: result = (mem << 8)  | (rt & 0x00000000000000FFULL); break;
        case 2: result = (mem << 16) | (rt & 0x000000000000FFFFULL); break;
        case 3: result = (mem << 24) | (rt & 0x0000000000FFFFFFULL); break;
        case 4: result = (mem << 32) | (rt & 0x00000000FFFFFFFFULL); break;
        case 5: result = (mem << 40) | (rt & 0x000000FFFFFFFFFFULL); break;
        case 6: result = (mem << 48) | (rt & 0x0000FFFFFFFFFFFFULL); break;
        case 7: result = (mem << 56) | (rt & 0x00FFFFFFFFFFFFFFULL); break;
    }
    
    cpu.set_gpr(instr.i_type.rt, result);
    return 1;
}

u32 LDR(VR4300& cpu, const Instruction& instr) {
    // Load Doubleword Right - učitava bajtove u DONJI deo registra
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(7);
    u32 offset = static_cast<u32>(addr & 7);
    
    u64 mem = cpu.read_memory<u64>(aligned);
    u64 rt = cpu.gpr(instr.i_type.rt);
    u64 result = 0;
    
    switch (offset) {
        case 0: result = (rt & 0xFFFFFFFFFFFFFF00ULL) | (mem >> 56); break;
        case 1: result = (rt & 0xFFFFFFFFFFFF0000ULL) | (mem >> 48); break;
        case 2: result = (rt & 0xFFFFFFFFFF000000ULL) | (mem >> 40); break;
        case 3: result = (rt & 0xFFFFFFFF00000000ULL) | (mem >> 32); break;
        case 4: result = (rt & 0xFFFFFF0000000000ULL) | (mem >> 24); break;
        case 5: result = (rt & 0xFFFF000000000000ULL) | (mem >> 16); break;
        case 6: result = (rt & 0xFF00000000000000ULL) | (mem >> 8); break;
        case 7: result = mem; break;
    }
    
    cpu.set_gpr(instr.i_type.rt, result);
    return 1;
}

u32 LL(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 4, true)) return 1;
    u64 value = cpu.read_memory<u32>(address);
    cpu.set_gpr(instr.i_type.rt, sign_extend32(value));
    cpu.set_LLbit(true);
    // LL_ADDR stores physical address >> 4 (cache line granularity)
    u32 paddr = cpu.cp0().translate_address(address);
    cpu.cp0().set_reg(CP0Reg::LL_ADDR, paddr >> 4);
    return 1;
}

u32 LLD(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 8, true)) return 1;
    u64 value = cpu.read_memory<u64>(address);
    cpu.set_gpr(instr.i_type.rt, value);
    cpu.set_LLbit(true);
    // LL_ADDR stores physical address >> 4 (cache line granularity)
    u32 paddr = cpu.cp0().translate_address(address);
    cpu.cp0().set_reg(CP0Reg::LL_ADDR, paddr >> 4);
    return 1;
}

// Store instructions
u32 SB(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 1, false)) return 1;
    cpu.write_memory<u8>(address, static_cast<u8>(cpu.gpr(instr.i_type.rt)));
    return 1;
}

u32 SH(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 2, false)) return 1;
    cpu.write_memory<u16>(address, static_cast<u16>(cpu.gpr(instr.i_type.rt)));
    return 1;
}

u32 SW(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 4, false)) return 1;
    cpu.write_memory<u32>(address, static_cast<u32>(cpu.gpr(instr.i_type.rt)));
    return 1;
}

u32 SWL(VR4300& cpu, const Instruction& instr) {
    // Store Word Left - upisuje GORNJE bajtove registra u memoriju
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(3);
    u32 offset = static_cast<u32>(addr & 3);
    
    u32 rt = static_cast<u32>(cpu.gpr(instr.i_type.rt));
    u32 mem = static_cast<u32>(cpu.read_memory<u32>(aligned));
    u32 result = 0;
    
    switch (offset) {
        case 0: result = rt; break;
        case 1: result = (mem & 0xFF000000) | (rt >> 8); break;
        case 2: result = (mem & 0xFFFF0000) | (rt >> 16); break;
        case 3: result = (mem & 0xFFFFFF00) | (rt >> 24); break;
    }
    
    cpu.write_memory<u32>(aligned, result);
    return 1;
}

u32 SWR(VR4300& cpu, const Instruction& instr) {
    // Store Word Right - upisuje DONJE bajtove registra u memoriju
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(3);
    u32 offset = static_cast<u32>(addr & 3);
    
    u32 rt = static_cast<u32>(cpu.gpr(instr.i_type.rt));
    u32 mem = static_cast<u32>(cpu.read_memory<u32>(aligned));
    u32 result = 0;
    
    switch (offset) {
        case 0: result = (mem & 0x00FFFFFF) | (rt << 24); break;
        case 1: result = (mem & 0x0000FFFF) | (rt << 16); break;
        case 2: result = (mem & 0x000000FF) | (rt << 8); break;
        case 3: result = rt; break;
    }
    
    cpu.write_memory<u32>(aligned, result);
    return 1;
}

u32 SD(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 8, false)) return 1;
    cpu.write_memory<u64>(address, cpu.gpr(instr.i_type.rt));
    return 1;
}

u32 SDL(VR4300& cpu, const Instruction& instr) {
    // Store Doubleword Left - upisuje GORNJE bajtove registra u memoriju
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(7);
    u32 offset = static_cast<u32>(addr & 7);
    
    u64 rt = cpu.gpr(instr.i_type.rt);
    u64 mem = cpu.read_memory<u64>(aligned);
    u64 result = 0;
    
    switch (offset) {
        case 0: result = rt; break;
        case 1: result = (mem & 0xFF00000000000000ULL) | (rt >> 8); break;
        case 2: result = (mem & 0xFFFF000000000000ULL) | (rt >> 16); break;
        case 3: result = (mem & 0xFFFFFF0000000000ULL) | (rt >> 24); break;
        case 4: result = (mem & 0xFFFFFFFF00000000ULL) | (rt >> 32); break;
        case 5: result = (mem & 0xFFFFFFFFFF000000ULL) | (rt >> 40); break;
        case 6: result = (mem & 0xFFFFFFFFFFFF0000ULL) | (rt >> 48); break;
        case 7: result = (mem & 0xFFFFFFFFFFFFFF00ULL) | (rt >> 56); break;
    }
    
    cpu.write_memory<u64>(aligned, result);
    return 1;
}

u32 SDR(VR4300& cpu, const Instruction& instr) {
    // Store Doubleword Right - upisuje DONJE bajtove registra u memoriju
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(7);
    u32 offset = static_cast<u32>(addr & 7);
    
    u64 rt = cpu.gpr(instr.i_type.rt);
    u64 mem = cpu.read_memory<u64>(aligned);
    u64 result = 0;
    
    switch (offset) {
        case 0: result = (mem & 0x00FFFFFFFFFFFFFFULL) | (rt << 56); break;
        case 1: result = (mem & 0x0000FFFFFFFFFFFFULL) | (rt << 48); break;
        case 2: result = (mem & 0x000000FFFFFFFFFFULL) | (rt << 40); break;
        case 3: result = (mem & 0x00000000FFFFFFFFULL) | (rt << 32); break;
        case 4: result = (mem & 0x0000000000FFFFFFULL) | (rt << 24); break;
        case 5: result = (mem & 0x000000000000FFFFULL) | (rt << 16); break;
        case 6: result = (mem & 0x00000000000000FFULL) | (rt << 8); break;
        case 7: result = rt; break;
    }
    
    cpu.write_memory<u64>(aligned, result);
    return 1;
}

u32 SC(VR4300& cpu, const Instruction& instr) {
    u64 vaddr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(vaddr, 4, false)) return 1;
    // On N64 (single-processor), SC only checks LLbit, not address
    // Address comparison is handled by cache coherency on multi-processor systems
    bool success = cpu.get_LLbit();
    if (success) {
        cpu.write_memory<u32>(vaddr, static_cast<u32>(cpu.gpr(instr.i_type.rt)));
    }
    cpu.set_gpr(instr.i_type.rt, success ? 1 : 0);
    cpu.set_LLbit(false);
    return 1;
}

u32 SCD(VR4300& cpu, const Instruction& instr) {
    u64 vaddr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(vaddr, 8, false)) return 1;
    // On N64 (single-processor), SCD only checks LLbit, not address
    bool success = cpu.get_LLbit();
    if (success) {
        cpu.write_memory<u64>(vaddr, cpu.gpr(instr.i_type.rt));
    }
    cpu.set_gpr(instr.i_type.rt, success ? 1 : 0);
    cpu.set_LLbit(false);
    return 1;
}

// FPU Load/Store
u32 LWC1(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 4, true)) return 1;
    u32 value = cpu.read_memory<u32>(address);
    u8 ft = instr.i_type.rt;
    
    if (cpu.cp0().get_fr_bit()) {
        // FR=1: each register is independent 64-bit, sign-extend 32-bit value
        cpu.cp1().set_fpr_64(ft, sign_extend32(value));
    } else {
        // FR=0: even/odd pairs share physical register
        // set_fpr_32 handles the mapping automatically
        cpu.cp1().set_fpr_32(ft, value);
    }
    return 1;
}

u32 LDC1(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 8, true)) return 1;
    u64 value = cpu.read_memory<u64>(address);
    u8 ft = instr.i_type.rt;
    cpu.cp1().set_fpr_64(ft, value);
    return 1;
}

u32 SWC1(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 4, false)) return 1;
    u8 ft = instr.i_type.rt;
    
    if (cpu.cp0().get_fr_bit()) {
        // FR=1: each register is independent 64-bit, sign-extend 32-bit value
        cpu.write_memory<u32>(address, static_cast<u32>(cpu.cp1().get_fpr_64(ft)));
    } else {
        // FR=0: even/odd pairs share physical register
        // set_fpr_32 handles the mapping automatically
        cpu.write_memory<u32>(address, cpu.cp1().get_fpr_32(ft));
    }
    return 1;
}

u32 SDC1(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    if (cpu.check_address_exception(address, 8, false)) return 1;
    u8 ft = instr.i_type.rt;
    if (cpu.cp0().get_fr_bit()) {
        cpu.write_memory<u64>(address, cpu.cp1().get_fpr_64(ft));
    } else if ((ft & 1) == 0) {
        u64 value = static_cast<u64>(cpu.cp1().get_fpr_32(ft + 1)) << 32;
        value |= static_cast<u64>(cpu.cp1().get_fpr_32(ft));
        cpu.write_memory<u64>(address, value);
    }
    return 1;
}

// Cache
u32 CACHE(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement on TLB
    return 1;
}

// ============================================================================
// Special Table Instructions (opcode 0x00, by funct)
// ============================================================================

// Shift instructions
u32 SLL(VR4300& cpu, const Instruction& instr) {
    u32 value = static_cast<u32>(cpu.gpr(instr.r_type.rt)) << instr.r_type.shift_amount;
    cpu.set_gpr(instr.r_type.rd, sign_extend32(value));
    return 1;
}

u32 SRL(VR4300& cpu, const Instruction& instr) {
    u32 value = static_cast<u32>(cpu.gpr(instr.r_type.rt)) >> instr.r_type.shift_amount;
    cpu.set_gpr(instr.r_type.rd, sign_extend32(value));
    return 1;
}

u32 SRA(VR4300& cpu, const Instruction& instr) {
    s32 value = static_cast<s32>(cpu.gpr(instr.r_type.rt)) >> instr.r_type.shift_amount;
    cpu.set_gpr(instr.r_type.rd, sign_extend32(value));
    return 1;
}

u32 SLLV(VR4300& cpu, const Instruction& instr) {
    u32 value = static_cast<u32>(cpu.gpr(instr.r_type.rt)) << (cpu.gpr(instr.r_type.rs) & 0x1F);
    cpu.set_gpr(instr.r_type.rd, sign_extend32(value));
    return 1;
}

u32 SRLV(VR4300& cpu, const Instruction& instr) {
    u32 value = static_cast<u32>(cpu.gpr(instr.r_type.rt)) >> (cpu.gpr(instr.r_type.rs) & 0x1F);
    cpu.set_gpr(instr.r_type.rd, sign_extend32(value));
    return 1;
}

u32 SRAV(VR4300& cpu, const Instruction& instr) {
    s32 value = static_cast<s32>(cpu.gpr(instr.r_type.rt)) >> (cpu.gpr(instr.r_type.rs) & 0x1F);
    cpu.set_gpr(instr.r_type.rd, sign_extend32(value));
    return 1;
}

u32 DSLL(VR4300& cpu, const Instruction& instr) {
    u64 value = cpu.gpr(instr.r_type.rt) << instr.r_type.shift_amount;
    cpu.set_gpr(instr.r_type.rd, value);
    return 1;
}

u32 DSRL(VR4300& cpu, const Instruction& instr) {
    u64 value = cpu.gpr(instr.r_type.rt) >> instr.r_type.shift_amount;
    cpu.set_gpr(instr.r_type.rd, value);
    return 1;
}

u32 DSRA(VR4300& cpu, const Instruction& instr) {
    s64 value = static_cast<s64>(cpu.gpr(instr.r_type.rt)) >> instr.r_type.shift_amount;
    cpu.set_gpr(instr.r_type.rd, value);
    return 1;
}

u32 DSLLV(VR4300& cpu, const Instruction& instr) {
    u64 value = cpu.gpr(instr.r_type.rt) << (cpu.gpr(instr.r_type.rs) & 0x3F);
    cpu.set_gpr(instr.r_type.rd, value);
    return 1;
}

u32 DSRLV(VR4300& cpu, const Instruction& instr) {
    u64 value = cpu.gpr(instr.r_type.rt) >> (cpu.gpr(instr.r_type.rs) & 0x3F);
    cpu.set_gpr(instr.r_type.rd, value);
    return 1;
}

u32 DSRAV(VR4300& cpu, const Instruction& instr) {
    s64 value = static_cast<s64>(cpu.gpr(instr.r_type.rt)) >> (cpu.gpr(instr.r_type.rs) & 0x3F);
    cpu.set_gpr(instr.r_type.rd, value);
    return 1;
}

u32 DSLL32(VR4300& cpu, const Instruction& instr) {
    u64 value = cpu.gpr(instr.r_type.rt) << (instr.r_type.shift_amount + 32);
    cpu.set_gpr(instr.r_type.rd, value);
    return 1;
}

u32 DSRL32(VR4300& cpu, const Instruction& instr) {
    u64 value = cpu.gpr(instr.r_type.rt) >> (instr.r_type.shift_amount + 32);
    cpu.set_gpr(instr.r_type.rd, value);
    return 1;
}

u32 DSRA32(VR4300& cpu, const Instruction& instr) {
    s64 value = static_cast<s64>(cpu.gpr(instr.r_type.rt)) >> (instr.r_type.shift_amount + 32);
    cpu.set_gpr(instr.r_type.rd, value);
    return 1;
}

// Jump register
u32 JR(VR4300& cpu, const Instruction& instr) {
    u64 new_pc = cpu.gpr(instr.r_type.rs);
    cpu.delay_branch(new_pc);
    return 1;
}

u32 JALR(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rd, cpu.pc() + 8);
    u64 new_pc = cpu.gpr(instr.r_type.rs);
    cpu.delay_branch(new_pc);
    return 1;
}

// System
u32 SYSCALL(VR4300& cpu, const Instruction& instr) {
    cpu.cp0().raise_exception(ExceptionCode::SYS);
    return 1;
}

u32 BREAK(VR4300& cpu, const Instruction& instr) {
    cpu.cp0().raise_exception(ExceptionCode::BP);
    return 1;
}

u32 SYNC(VR4300& cpu, const Instruction& instr) {
    return 1;
}

// Move from/to HI/LO
u32 MFHI(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rd, cpu.hi());
    return 1;
}

u32 MTHI(VR4300& cpu, const Instruction& instr) {
    cpu.set_hi(cpu.gpr(instr.r_type.rs));
    return 1;
}

u32 MFLO(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rd, cpu.lo());
    return 1;
}

u32 MTLO(VR4300& cpu, const Instruction& instr) {
    cpu.set_lo(cpu.gpr(instr.r_type.rs));
    return 1;
}

// Multiply/Divide
u32 MULT(VR4300& cpu, const Instruction& instr) {
    s64 rs = static_cast<s32>(cpu.gpr(instr.r_type.rs));
    s64 rt = static_cast<s32>(cpu.gpr(instr.r_type.rt));
    s64 result = rs * rt;
    cpu.set_lo(sign_extend32(static_cast<u32>(result)));
    cpu.set_hi(sign_extend32(static_cast<u32>(result >> 32)));
    return 3;
}

u32 MULTU(VR4300& cpu, const Instruction& instr) {
    u64 rs = static_cast<u32>(cpu.gpr(instr.r_type.rs));
    u64 rt = static_cast<u32>(cpu.gpr(instr.r_type.rt));
    u64 result = rs * rt;
    cpu.set_lo(sign_extend32(static_cast<u32>(result)));
    cpu.set_hi(sign_extend32(static_cast<u32>(result >> 32)));
    return 3;
}

u32 DIV(VR4300& cpu, const Instruction& instr) {
    s32 rs = static_cast<s32>(cpu.gpr(instr.r_type.rs));
    s32 rt = static_cast<s32>(cpu.gpr(instr.r_type.rt));
    if (rt == 0) {
        // Division by zero: LO = (rs >= 0) ? -1 : 1, HI = rs
        cpu.set_lo(rs >= 0 ? -1LL : 1LL);
        cpu.set_hi(sign_extend32(static_cast<u32>(rs)));
        return 35;
    }
    // Handle overflow: MIN_INT / -1
    if (rs == INT32_MIN && rt == -1) {
        cpu.set_lo(sign_extend32(static_cast<u32>(rs)));
        cpu.set_hi(0);
        return 35;
    }
    cpu.set_lo(sign_extend32(static_cast<u32>(rs / rt)));   // Quotient
    cpu.set_hi(sign_extend32(static_cast<u32>(rs % rt)));   // Remainder
    return 35;
}

u32 DIVU(VR4300& cpu, const Instruction& instr) {
    u32 rs = static_cast<u32>(cpu.gpr(instr.r_type.rs));
    u32 rt = static_cast<u32>(cpu.gpr(instr.r_type.rt));
    if (rt == 0) {
        // Division by zero: LO = -1, HI = rs
        cpu.set_lo(-1LL);
        cpu.set_hi(sign_extend32(rs));
        return 35;
    }
    cpu.set_lo(sign_extend32(rs / rt));   // Quotient
    cpu.set_hi(sign_extend32(rs % rt));   // Remainder
    return 35;
}

u32 DMULT(VR4300& cpu, const Instruction& instr) {
    __int128 rs = static_cast<s64>(cpu.gpr(instr.r_type.rs));
    __int128 rt = static_cast<s64>(cpu.gpr(instr.r_type.rt));
    __int128 result = rs * rt;
    cpu.set_lo(static_cast<u64>(result));
    cpu.set_hi(static_cast<u64>(result >> 64));
    return 8;
}

u32 DMULTU(VR4300& cpu, const Instruction& instr) {
    unsigned __int128 rs = cpu.gpr(instr.r_type.rs);
    unsigned __int128 rt = cpu.gpr(instr.r_type.rt);
    unsigned __int128 result = rs * rt;
    cpu.set_lo(static_cast<u64>(result));
    cpu.set_hi(static_cast<u64>(result >> 64));
    return 8;
}

u32 DDIV(VR4300& cpu, const Instruction& instr) {
    s64 rs = static_cast<s64>(cpu.gpr(instr.r_type.rs));
    s64 rt = static_cast<s64>(cpu.gpr(instr.r_type.rt));
    if (rt == 0) {
        // Division by zero: LO = (rs >= 0) ? -1 : 1, HI = rs
        cpu.set_lo(rs >= 0 ? -1LL : 1LL);
        cpu.set_hi(rs);
        return 69;
    }
    // Handle overflow: MIN_INT / -1
    if (rs == INT64_MIN && rt == -1) {
        cpu.set_lo(rs);
        cpu.set_hi(0);
        return 69;
    }
    cpu.set_lo(rs / rt);
    cpu.set_hi(rs % rt);
    return 69;
}

u32 DDIVU(VR4300& cpu, const Instruction& instr) {
    u64 rs = cpu.gpr(instr.r_type.rs);
    u64 rt = cpu.gpr(instr.r_type.rt);
    if (rt == 0) {
        // Division by zero: LO = -1, HI = rs
        cpu.set_lo(-1ULL);
        cpu.set_hi(rs);
        return 69;
    }
    cpu.set_lo(rs / rt);
    cpu.set_hi(rs % rt);
    return 69;
}

// Arithmetic (32-bit)
u32 ADD(VR4300& cpu, const Instruction& instr) {
    // TODO: Overflow exception
    u32 rs = static_cast<u32>(cpu.gpr(instr.r_type.rs));
    u32 rt = static_cast<u32>(cpu.gpr(instr.r_type.rt));
    cpu.set_gpr(instr.r_type.rd, sign_extend32(rs + rt));
    return 1;
}

u32 ADDU(VR4300& cpu, const Instruction& instr) {
    u32 rs = static_cast<u32>(cpu.gpr(instr.r_type.rs));
    u32 rt = static_cast<u32>(cpu.gpr(instr.r_type.rt));
    cpu.set_gpr(instr.r_type.rd, sign_extend32(rs + rt));
    return 1;
}

u32 SUB(VR4300& cpu, const Instruction& instr) {
    // TODO: Overflow exception
    u32 rs = static_cast<u32>(cpu.gpr(instr.r_type.rs));
    u32 rt = static_cast<u32>(cpu.gpr(instr.r_type.rt));
    cpu.set_gpr(instr.r_type.rd, sign_extend32(rs - rt));
    return 1;
}

u32 SUBU(VR4300& cpu, const Instruction& instr) {
    u32 rs = static_cast<u32>(cpu.gpr(instr.r_type.rs));
    u32 rt = static_cast<u32>(cpu.gpr(instr.r_type.rt));
    cpu.set_gpr(instr.r_type.rd, sign_extend32(rs - rt));
    return 1;
}

// Arithmetic (64-bit)
u32 DADD(VR4300& cpu, const Instruction& instr) {
    // TODO: Overflow exception
    cpu.set_gpr(instr.r_type.rd, cpu.gpr(instr.r_type.rs) + cpu.gpr(instr.r_type.rt));
    return 1;
}

u32 DADDU(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rd, cpu.gpr(instr.r_type.rs) + cpu.gpr(instr.r_type.rt));
    return 1;
}

u32 DSUB(VR4300& cpu, const Instruction& instr) {
    // TODO: Overflow exception
    cpu.set_gpr(instr.r_type.rd, cpu.gpr(instr.r_type.rs) - cpu.gpr(instr.r_type.rt));
    return 1;
}

u32 DSUBU(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rd, cpu.gpr(instr.r_type.rs) - cpu.gpr(instr.r_type.rt));
    return 1;
}

// Logical
u32 AND(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rd, cpu.gpr(instr.r_type.rs) & cpu.gpr(instr.r_type.rt));
    return 1;
}

u32 OR(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rd, cpu.gpr(instr.r_type.rs) | cpu.gpr(instr.r_type.rt));
    return 1;
}

u32 XOR(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rd, cpu.gpr(instr.r_type.rs) ^ cpu.gpr(instr.r_type.rt));
    return 1;
}

u32 NOR(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rd, ~(cpu.gpr(instr.r_type.rs) | cpu.gpr(instr.r_type.rt)));
    return 1;
}

// Set less than
u32 SLT(VR4300& cpu, const Instruction& instr) {
    s64 rs = static_cast<s64>(cpu.gpr(instr.r_type.rs));
    s64 rt = static_cast<s64>(cpu.gpr(instr.r_type.rt));
    cpu.set_gpr(instr.r_type.rd, rs < rt ? 1 : 0);
    return 1;
}

u32 SLTU(VR4300& cpu, const Instruction& instr) {
    u64 rs = cpu.gpr(instr.r_type.rs);
    u64 rt = cpu.gpr(instr.r_type.rt);
    cpu.set_gpr(instr.r_type.rd, rs < rt ? 1 : 0);
    return 1;
}

// Trap
u32 TGE(VR4300& cpu, const Instruction& instr) {
    s64 value_rs = static_cast<s64>(cpu.gpr(instr.r_type.rs));
    s64 value_rt = static_cast<s64>(cpu.gpr(instr.r_type.rt));
    if (value_rs >= value_rt) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TGEU(VR4300& cpu, const Instruction& instr) {
    u64 value_rs = cpu.gpr(instr.r_type.rs);
    u64 value_rt = cpu.gpr(instr.r_type.rt);
    if (value_rs >= value_rt) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TLT(VR4300& cpu, const Instruction& instr) {
    s64 value_rs = static_cast<s64>(cpu.gpr(instr.r_type.rs));
    s64 value_rt = static_cast<s64>(cpu.gpr(instr.r_type.rt));
    if (value_rs < value_rt) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TLTU(VR4300& cpu, const Instruction& instr) {
    u64 value_rs = cpu.gpr(instr.r_type.rs);
    u64 value_rt = cpu.gpr(instr.r_type.rt);
    if (value_rs < value_rt) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TEQ(VR4300& cpu, const Instruction& instr) {
    s64 value_rs = static_cast<s64>(cpu.gpr(instr.r_type.rs));
    s64 value_rt = static_cast<s64>(cpu.gpr(instr.r_type.rt));
    if (value_rs == value_rt) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TNE(VR4300& cpu, const Instruction& instr) {
    s64 value_rs = static_cast<s64>(cpu.gpr(instr.r_type.rs));
    s64 value_rt = static_cast<s64>(cpu.gpr(instr.r_type.rt));
    if (value_rs != value_rt) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

// ============================================================================
// REGIMM Table Instructions (opcode 0x01, by rt)
// ============================================================================

u32 BLTZ(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) < 0) {
        cpu.delay_branch(address);
    }
    return 1;
}

u32 BGEZ(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) >= 0) {
        cpu.delay_branch(address);
    }
    return 1;
}

u32 BLTZL(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) < 0) {
        cpu.delay_branch(address);
    } else {
        cpu.set_pc(cpu.pc() + 4);
    }
    return 1;
}

u32 BGEZL(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) >= 0) {
        cpu.delay_branch(address);
    } else {
        cpu.set_pc(cpu.pc() + 4);
    }
    return 1;
}

u32 BLTZAL(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    cpu.set_gpr(31, cpu.pc() + 8);
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) < 0) {
        cpu.delay_branch(address);
    }
    return 1;
}

u32 BGEZAL(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    cpu.set_gpr(31, cpu.pc() + 8);
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) >= 0) {
        cpu.delay_branch(address);
    }
    return 1;
}

u32 BLTZALL(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) < 0) {
        cpu.set_gpr(31, cpu.pc() + 8);
        cpu.delay_branch(address);
    } else {
        cpu.set_pc(cpu.pc() + 4);
    }
    return 1;
}

u32 BGEZALL(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    if (static_cast<s64>(cpu.gpr(instr.i_type.rs)) >= 0) {
        cpu.set_gpr(31, cpu.pc() + 8);
        cpu.delay_branch(address);
    } else {
        cpu.set_pc(cpu.pc() + 4);
    }
    return 1;
}

// Trap immediate
u32 TGEI(VR4300& cpu, const Instruction& instr) {
    s64 value_rs = static_cast<s64>(cpu.gpr(instr.i_type.rs));
    s64 value_imm = sign_extend16(instr.i_type.immediate);
    if (value_rs >= value_imm) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TGEIU(VR4300& cpu, const Instruction& instr) {
    u64 value_rs = cpu.gpr(instr.i_type.rs);
    u64 value_imm = static_cast<u64>(sign_extend16(instr.i_type.immediate));
    if (value_rs >= value_imm) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TLTI(VR4300& cpu, const Instruction& instr) {
    s64 value_rs = static_cast<s64>(cpu.gpr(instr.i_type.rs));
    s64 value_imm = sign_extend16(instr.i_type.immediate);
    if (value_rs < value_imm) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TLTIU(VR4300& cpu, const Instruction& instr) {
    u64 value_rs = cpu.gpr(instr.i_type.rs);
    u64 value_imm = static_cast<u64>(sign_extend16(instr.i_type.immediate));
    if (value_rs < value_imm) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TEQI(VR4300& cpu, const Instruction& instr) {
    s64 value_rs = static_cast<s64>(cpu.gpr(instr.i_type.rs));
    s64 value_imm = sign_extend16(instr.i_type.immediate);
    if (value_rs == value_imm) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

u32 TNEI(VR4300& cpu, const Instruction& instr) {
    s64 value_rs = static_cast<s64>(cpu.gpr(instr.i_type.rs));
    s64 value_imm = sign_extend16(instr.i_type.immediate);
    if (value_rs != value_imm) {
        cpu.cp0().raise_exception(ExceptionCode::TR);
    }
    return 1;
}

// ============================================================================
// COP0 Instructions
// ============================================================================

// Move from/to CP0
u32 MFC0(VR4300& cpu, const Instruction& instr) {
    u32 value = static_cast<u32>(cpu.cp0().get_reg(instr.r_type.rd));
    cpu.set_gpr(instr.r_type.rt, sign_extend32(value));  // Sign-extend to 64-bit
    return 1;
}

u32 DMFC0(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rt, cpu.cp0().get_reg(instr.r_type.rd));
    return 1;
}

u32 MTC0(VR4300& cpu, const Instruction& instr) {
    cpu.cp0().set_reg(instr.r_type.rd, static_cast<u32>(cpu.gpr(instr.r_type.rt)));
    return 1;
}

u32 DMTC0(VR4300& cpu, const Instruction& instr) {
    cpu.cp0().set_reg(instr.r_type.rd, cpu.gpr(instr.r_type.rt));
    return 1;
}

// TLB instructions
u32 TLBR(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TLBWI(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TLBWR(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TLBP(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 ERET(VR4300& cpu, const Instruction& instr) {
    CP0Status& status = cpu.cp0().status();
    if (status.erl) {
        status.erl = 0;
        cpu.set_pc(cpu.cp0().error_epc());
    } else {
        status.exl = 0;
        cpu.set_pc(cpu.cp0().epc());
    }
    cpu.set_LLbit(false);
    return 1;
}

// COP0 Branch
u32 BC0F(VR4300& cpu, const Instruction& instr) {
    u64 target = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    cpu.delay_branch(target);
    return 1;
}

u32 BC0T(VR4300& cpu, const Instruction& instr) {
    return 1;
}

u32 BC0FL(VR4300& cpu, const Instruction& instr) {
    u64 target = cpu.pc() + (sign_extend16(instr.i_type.immediate) << 2);
    cpu.delay_branch(target);
    return 1;
}

u32 BC0TL(VR4300& cpu, const Instruction& instr) {
    cpu.set_pc(cpu.pc() + 4);
    return 1;
}

// ============================================================================
// COP1 (FPU) Instructions
// ============================================================================

// Move from/to CP1
u32 MFC1(VR4300& cpu, const Instruction& instr) {
    u8 rt = instr.r_type.rt;
    u8 fs = instr.r_type.rd;
    
    u32 value;
    if (cpu.cp0().get_fr_bit()) {
        // FR=1: each register is independent 64-bit
        value = static_cast<u32>(cpu.cp1().get_fpr_64(fs));
    } else {
        // FR=0: even/odd pairs share physical register
        value = cpu.cp1().get_fpr_32(fs);
    }
    // Sign-extend 32-bit value to 64-bit GPR
    cpu.set_gpr(rt, sign_extend32(value));
    return 2;
}

u32 DMFC1(VR4300& cpu, const Instruction& instr) {
    u8 rt = instr.r_type.rt;
    u8 fs = instr.r_type.rd;
    
    if (!cpu.cp0().get_fr_bit() && (fs & 1) == 1) {
        // FR=0, odd register: undefined behavior for 64-bit operations
        // TODO: Reserved instruction exception
    }
    // FR=0 even register or FR=1 any register: move full 64 bits
    cpu.set_gpr(rt, cpu.cp1().get_fpr_64(fs));
    return 2;
}

u32 CFC1(VR4300& cpu, const Instruction& instr) {
    u8 rt = instr.r_type.rt;
    u8 fs = instr.r_type.rd;

    // Sign-extend 32-bit FCR value to 64-bit GPR
    cpu.set_gpr(rt, sign_extend32(cpu.cp1().get_fcr(fs)));

    return 1;
}

u32 MTC1(VR4300& cpu, const Instruction& instr) {
    u8 rt = instr.r_type.rt;
    u8 fs = instr.r_type.rd;
    
    // Take lower 32 bits of GPR
    u32 value = static_cast<u32>(cpu.gpr(rt));
    if (cpu.cp0().get_fr_bit()) {
        // FR=1: each register is independent 64-bit, store as sign-extended
        cpu.cp1().set_fpr_64(fs, sign_extend32(value));
    } else {
        // FR=0: even/odd pairs share physical register
        cpu.cp1().set_fpr_32(fs, value);
    }
    return 2;
}

u32 DMTC1(VR4300& cpu, const Instruction& instr) {
    u8 rt = instr.r_type.rt;
    u8 fs = instr.r_type.rd;
    
    if (!cpu.cp0().get_fr_bit() && (fs & 1) == 1) {
        // FR=0, odd register: undefined behavior for 64-bit operations
        // TODO: Reserved instruction exception
    }
    // FR=0 even register or FR=1 any register: move full 64 bits
    cpu.cp1().set_fpr_64(fs, cpu.gpr(rt));
    return 2;
}

u32 CTC1(VR4300& cpu, const Instruction& instr) {
    u8 rt = instr.r_type.rt;
    u8 fs = instr.r_type.rd;
    
    cpu.cp1().set_fcr(fs, cpu.gpr(rt));

    // TODO: Enable exceptions for written bits
    return 1;
}

// COP1 Branch
u32 BC1F(VR4300& cpu, const Instruction& instr) {
    // Branch if FPU condition bit is false
    if (!cpu.cp1().get_condition_bit()) {
        u64 offset = sign_extend16(instr.i_type.immediate) << 2;
        cpu.delay_branch(cpu.pc() + offset);
    }
    return 1;
}

u32 BC1T(VR4300& cpu, const Instruction& instr) {
    // Branch if FPU condition bit is true
    if (cpu.cp1().get_condition_bit()) {
        u64 offset = sign_extend16(instr.i_type.immediate) << 2;
        cpu.delay_branch(cpu.pc() + offset);
    }
    return 1;
}

u32 BC1FL(VR4300& cpu, const Instruction& instr) {
    // Branch if FPU condition bit is false (likely - nullify delay slot if not taken)
    if (!cpu.cp1().get_condition_bit()) {
        u64 offset = sign_extend16(instr.i_type.immediate) << 2;
        cpu.delay_branch(cpu.pc() + offset);
    } else {
        cpu.set_pc(cpu.pc() + 4);  // Skip delay slot
    }
    return 1;
}

u32 BC1TL(VR4300& cpu, const Instruction& instr) {
    // Branch if FPU condition bit is true (likely - nullify delay slot if not taken)
    if (cpu.cp1().get_condition_bit()) {
        u64 offset = sign_extend16(instr.i_type.immediate) << 2;
        cpu.delay_branch(cpu.pc() + offset);
    } else {
        cpu.set_pc(cpu.pc() + 4);  // Skip delay slot
    }
    return 1;
}

// FPU Arithmetic (format determined by rs field)
u32 ADD_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;
    u8 ft = instr.cop1_type.ft;
    // TODO: FPU exceptions - check for overflow, underflow, inexact, invalid (NaN operands)
    if (fmt == 16) {
        // ADD.S - single precision (32-bit)
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        f32 result = f1 + f2;
        cpu.cp1().set_fpr_single(fd, result);
    } else if (fmt == 17) {
        // ADD.D - double precision (64-bit)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        f64 result = f1 + f2;
        cpu.cp1().set_fpr_double(fd, result);
    }
    return 3;
}

u32 SUB_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;
    u8 ft = instr.cop1_type.ft;

    // TODO: FPU exceptions - check for overflow, underflow, inexact, invalid (NaN operands)
    if (fmt == 16) {
        // SUB.S - single precision (32-bit)
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        f32 result = f1 - f2;
        cpu.cp1().set_fpr_single(fd, result);
    } else if (fmt == 17) {
        // SUB.D - double precision (64-bit)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        f64 result = f1 - f2;
        cpu.cp1().set_fpr_double(fd, result);
    }
    return 3;
}

u32 MUL_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;
    u8 ft = instr.cop1_type.ft;

    // TODO: FPU exceptions - check for overflow, underflow, inexact, invalid (NaN/Inf * 0)
    if (fmt == 16) {
        // MUL.S - single precision (32-bit)
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        f32 result = f1 * f2;
        cpu.cp1().set_fpr_single(fd, result);
        return 5;
    } else if (fmt == 17) {
        // MUL.D - double precision (64-bit)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        f64 result = f1 * f2;
        cpu.cp1().set_fpr_double(fd, result);
        return 8;
    }
    return 1;
}

u32 DIV_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;
    u8 ft = instr.cop1_type.ft;

    // TODO: FPU exceptions - check for divide by zero, overflow, underflow, inexact, invalid (0/0, Inf/Inf)
    if (fmt == 16) {
        // DIV.S - single precision (32-bit)
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        f32 result = f1 / f2;
        cpu.cp1().set_fpr_single(fd, result);
        return 29;
    } else if (fmt == 17) {
        // DIV.D - double precision (64-bit)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        f64 result = f1 / f2;
        cpu.cp1().set_fpr_double(fd, result);
        return 58;
    }
    return 1;
}

u32 SQRT_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - check for invalid (negative operand), inexact
    if (fmt == 16) {
        // SQRT.S - single precision (32-bit)
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        // TODO: if (f1 < 0) raise invalid operation exception
        f32 result = std::sqrt(f1);
        cpu.cp1().set_fpr_single(fd, result);
        return 29;
    } else if (fmt == 17) {
        // SQRT.D - double precision (64-bit)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        // TODO: if (f1 < 0) raise invalid operation exception
        f64 result = std::sqrt(f1);
        cpu.cp1().set_fpr_double(fd, result);
        return 58;
    }
    return 1;
}

u32 ABS_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // ABS does not generate arithmetic exceptions (just clears sign bit)
    if (fmt == 16) {
        // ABS.S - single precision (32-bit)
        u32 bits = cpu.cp1().get_fpr_32(fs);
        bits = bits & 0x7FFFFFFF;  // Clear sign bit
        cpu.cp1().set_fpr_32(fd, bits);
    } else if (fmt == 17) {
        // ABS.D - double precision (64-bit)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        u64 bits = cpu.cp1().get_fpr_64(fs);
        bits = bits & 0x7FFFFFFFFFFFFFFFULL;  // Clear sign bit
        cpu.cp1().set_fpr_64(fd, bits);
    }
    return 1;
}

u32 MOV_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;
    
    if (fmt == 16) {
        // MOV.S - single precision (32-bit)
        if (cpu.cp0().get_fr_bit()) {
            cpu.cp1().set_fpr_64(fd, cpu.cp1().get_fpr_64(fs));
        } else {
            cpu.cp1().set_fpr_32(fd, cpu.cp1().get_fpr_32(fs));
        }
    } else if (fmt == 17) {
        // MOV.D - double precision (64-bit)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            // FR=0, odd register: undefined
        }
        cpu.cp1().set_fpr_64(fd, cpu.cp1().get_fpr_64(fs));
    }
    return 1;
}

u32 NEG_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // NEG does not generate arithmetic exceptions (just flips sign bit)
    if (fmt == 16) {
        // NEG.S - single precision (32-bit)
        u32 bits = cpu.cp1().get_fpr_32(fs);
        bits = bits ^ 0x80000000;  // Flip sign bit
        cpu.cp1().set_fpr_32(fd, bits);
    } else if (fmt == 17) {
        // NEG.D - double precision (64-bit)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        u64 bits = cpu.cp1().get_fpr_64(fs);
        bits = bits ^ 0x8000000000000000ULL;  // Flip sign bit
        cpu.cp1().set_fpr_64(fd, bits);
    }
    return 1;
}

// FPU Round/Truncate/Ceil/Floor to Long
// Round to 64-bit integer (Long)
u32 ROUND_L_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - check for overflow (value out of s64 range), inexact
    std::fesetround(FE_TONEAREST);
    if (fmt == 16) {
        // ROUND.L.S - round single to 64-bit integer
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s64 result = static_cast<s64>(std::nearbyint(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    } else if (fmt == 17) {
        // ROUND.L.D - round double to 64-bit integer
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s64 result = static_cast<s64>(std::nearbyint(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    }
    return 1;
}

u32 TRUNC_L_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - check for overflow (value out of s64 range), inexact
    if (fmt == 16) {
        // TRUNC.L.S - truncate single to 64-bit integer
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s64 result = static_cast<s64>(std::trunc(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    } else if (fmt == 17) {
        // TRUNC.L.D - truncate double to 64-bit integer
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s64 result = static_cast<s64>(std::trunc(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    }
    return 1;
}

u32 CEIL_L_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - check for overflow (value out of s64 range), inexact
    if (fmt == 16) {
        // CEIL.L.S - ceiling of single to 64-bit integer
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s64 result = static_cast<s64>(std::ceil(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    } else if (fmt == 17) {
        // CEIL.L.D - ceiling of double to 64-bit integer
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s64 result = static_cast<s64>(std::ceil(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    }
    return 1;
}

u32 FLOOR_L_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - check for overflow (value out of s64 range), inexact
    if (fmt == 16) {
        // FLOOR.L.S - floor of single to 64-bit integer
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s64 result = static_cast<s64>(std::floor(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    } else if (fmt == 17) {
        // FLOOR.L.D - floor of double to 64-bit integer
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s64 result = static_cast<s64>(std::floor(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    }
    return 1;
}

// FPU Round/Truncate/Ceil/Floor to Word
u32 ROUND_W_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - check for overflow (value out of s32 range), inexact
    std::fesetround(FE_TONEAREST);
    if (fmt == 16) {
        // ROUND.W.S - round single to 32-bit integer
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s32 result = static_cast<s32>(std::nearbyint(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    } else if (fmt == 17) {
        // ROUND.W.D - round double to 32-bit integer
        if (!cpu.cp0().get_fr_bit() && (fs & 1)) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s32 result = static_cast<s32>(std::nearbyint(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    }
    return 1;
}

u32 TRUNC_W_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - check for overflow (value out of s32 range), inexact
    std::fesetround(FE_TONEAREST);
    if (fmt == 16) {
        // TRUNC.W.S - truncate single to 32-bit integer
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s32 result = static_cast<s32>(std::trunc(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    } else if (fmt == 17) {
        // TRUNC.W.D - truncate double to 32-bit integer
        if (!cpu.cp0().get_fr_bit() && (fs & 1)) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s32 result = static_cast<s32>(std::trunc(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    }
    return 1;
}

u32 CEIL_W_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - check for overflow (value out of s32 range), inexact
    std::fesetround(FE_TONEAREST);
    if (fmt == 16) {
        // CEIL.W.S - ceiling of single to 32-bit integer
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s32 result = static_cast<s32>(std::ceil(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    } else if (fmt == 17) {
        // CEIL.W.D - ceiling of double to 32-bit integer
        if (!cpu.cp0().get_fr_bit() && (fs & 1)) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s32 result = static_cast<s32>(std::ceil(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    }
    return 1;
}

u32 FLOOR_W_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - check for overflow (value out of s32 range), inexact
    std::fesetround(FE_TONEAREST);
    if (fmt == 16) {
        // FLOOR.W.S - floor of single to 32-bit integer
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s32 result = static_cast<s32>(std::floor(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    } else if (fmt == 17) {
        // FLOOR.W.D - floor of double to 32-bit integer
        if (!cpu.cp0().get_fr_bit() && (fs & 1)) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s32 result = static_cast<s32>(std::floor(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    }
    return 1;
}

// FPU Convert
u32 CVT_S_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - overflow (D->S), inexact (large integers lose precision)
    if (fmt == 17) {
        // CVT.S.D - convert double to single
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        cpu.cp1().set_fpr_single(fd, static_cast<f32>(f1));
        return 2;
    } else if (fmt == 20) {
        // CVT.S.W - convert word to single
        if (!cpu.cp0().get_fr_bit() && (fs & 1)) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        s32 f1 = static_cast<s32>(cpu.cp1().get_fpr_32(fs));
        cpu.cp1().set_fpr_single(fd, static_cast<f32>(f1));
        return 5;
    } else if (fmt == 21) {
        // CVT.S.L - convert long to single
        if (!cpu.cp0().get_fr_bit() && (fs & 1)) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        s64 f1 = static_cast<s64>(cpu.cp1().get_fpr_64(fs));
        cpu.cp1().set_fpr_single(fd, static_cast<f32>(f1));
        return 5;
    }
    return 1;
}

u32 CVT_D_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - inexact (large L integers lose precision)
    if (fmt == 16) {
        // CVT.D.S - convert single to double
        if (!cpu.cp0().get_fr_bit() && (fd & 1)) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        cpu.cp1().set_fpr_double(fd, static_cast<f64>(f1));
        return 1;
    } else if (fmt == 20) {
        // CVT.D.W - convert word to double
        if (!cpu.cp0().get_fr_bit() && (fd & 1)) {
            return 1;
        }
        s32 f1 = static_cast<s32>(cpu.cp1().get_fpr_32(fs));
        cpu.cp1().set_fpr_double(fd, static_cast<f64>(f1));
        return 5;
    } else if (fmt == 21) {
        // CVT.D.L - convert long to double
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        s64 f1 = static_cast<s64>(cpu.cp1().get_fpr_64(fs));
        cpu.cp1().set_fpr_double(fd, static_cast<f64>(f1));
        return 5;
    }
    return 1;
}

u32 CVT_W_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - invalid (NaN), overflow (value out of s32 range), inexact
    if (fmt == 16) {
        // CVT.W.S - convert single to word
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s32 result = static_cast<s32>(std::nearbyint(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    } else if (fmt == 17) {
        // CVT.W.D - convert double to word
        if (!cpu.cp0().get_fr_bit() && (fs & 1)) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s32 result = static_cast<s32>(std::nearbyint(f1));
        cpu.cp1().set_fpr_32(fd, static_cast<u32>(result));
        return 5;
    }
    return 1;
}

u32 CVT_L_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 fd = instr.cop1_type.fd;

    // TODO: FPU exceptions - invalid (NaN), overflow (value out of s64 range), inexact
    if (fmt == 16) {
        // CVT.L.S - convert single to long
        if (!cpu.cp0().get_fr_bit() && (fd & 1)) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        s64 result = static_cast<s64>(std::nearbyint(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    } else if (fmt == 17) {
        // CVT.L.D - convert double to long
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (fd & 1))) {
            return 1;
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        s64 result = static_cast<s64>(std::nearbyint(f1));
        cpu.cp1().set_fpr_64(fd, static_cast<u64>(result));
        return 5;
    }
    return 1;
}

// FPU Compare
u32 C_F_FMT(VR4300& cpu, const Instruction& instr) {
    cpu.cp1().set_condition_bit(false);
    return 1;
}

u32 C_UN_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.UN does not signal for NaN (quiet compare)
    if (fmt == 16) {
        // C.UN.S - unordered single comparison
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        condition = std::isnan(f1) || std::isnan(f2);
    } else if (fmt == 17) {
        // C.UN.D - unordered double comparison
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        condition = std::isnan(f1) || std::isnan(f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_EQ_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.EQ does not signal for NaN (quiet compare)
    if (fmt == 16) {
        // C.EQ.S - equal single comparison
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        condition = (f1 == f2);
    } else if (fmt == 17) {
        // C.EQ.D - equal double comparison
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        condition = (f1 == f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_UEQ_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.UEQ does not signal for NaN (quiet compare)
    if (fmt == 16) {
        // C.UEQ.S - unordered single comparison
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        condition = (f1 == f2) || (std::isnan(f1) || std::isnan(f2));
    } else if (fmt == 17) {
        // C.UEQ.D - unordered double comparison
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        condition = (f1 == f2) || (std::isnan(f1) || std::isnan(f2));
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_OLT_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.OLT does not signal for NaN (quiet compare)
    if (fmt == 16) {
        // C.OLT.S - ordered less than single
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        condition = (f1 < f2);
    } else if (fmt == 17) {
        // C.OLT.D - ordered less than double
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        condition = (f1 < f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_ULT_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.ULT does not signal for NaN (quiet compare)
    if (fmt == 16) {
        // C.ULT.S - unordered or less than single
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        condition = (f1 < f2) || std::isnan(f1) || std::isnan(f2);
    } else if (fmt == 17) {
        // C.ULT.D - unordered or less than double
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        condition = (f1 < f2) || std::isnan(f1) || std::isnan(f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_OLE_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.OLE does not signal for NaN (quiet compare)
    if (fmt == 16) {
        // C.OLE.S - ordered less or equal single
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        condition = (f1 <= f2);
    } else if (fmt == 17) {
        // C.OLE.D - ordered less or equal double
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        condition = (f1 <= f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_ULE_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.ULE does not signal for NaN (quiet compare)
    if (fmt == 16) {
        // C.ULE.S - unordered or less or equal single
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        condition = (f1 <= f2) || std::isnan(f1) || std::isnan(f2);
    } else if (fmt == 17) {
        // C.ULE.D - unordered or less or equal double
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        condition = (f1 <= f2) || std::isnan(f1) || std::isnan(f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_SF_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;

    // C.SF signals Invalid exception for NaN
    if (fmt == 16) {
        // C.SF.S - signaling false single
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
    } else if (fmt == 17) {
        // C.SF.D - signaling false double
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
    }
    cpu.cp1().set_condition_bit(false);
    return 1;
}

u32 C_NGLE_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.NGLE signals Invalid exception for NaN
    if (fmt == 16) {
        // C.NGLE.S - not greater/less/equal single (signaling unordered)
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = std::isnan(f1) || std::isnan(f2);
    } else if (fmt == 17) {
        // C.NGLE.D - not greater/less/equal double (signaling unordered)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = std::isnan(f1) || std::isnan(f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_SEQ_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.SEQ signals Invalid exception for NaN
    if (fmt == 16) {
        // C.SEQ.S - signaling equal single
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 == f2);
    } else if (fmt == 17) {
        // C.SEQ.D - signaling equal double
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 == f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_NGL_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.NGL signals Invalid exception for NaN
    if (fmt == 16) {
        // C.NGL.S - not greater or less single (signaling equal or unordered)
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 == f2) || std::isnan(f1) || std::isnan(f2);
    } else if (fmt == 17) {
        // C.NGL.D - not greater or less double (signaling equal or unordered)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 == f2) || std::isnan(f1) || std::isnan(f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_LT_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.LT signals Invalid exception for NaN
    if (fmt == 16) {
        // C.LT.S - signaling less than single
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 < f2);
    } else if (fmt == 17) {
        // C.LT.D - signaling less than double
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 < f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_NGE_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.NGE signals Invalid exception for NaN
    if (fmt == 16) {
        // C.NGE.S - not greater or equal single (signaling less than or unordered)
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 < f2) || std::isnan(f1) || std::isnan(f2);
    } else if (fmt == 17) {
        // C.NGE.D - not greater or equal double (signaling less than or unordered)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 < f2) || std::isnan(f1) || std::isnan(f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_LE_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.LE signals Invalid exception for NaN
    if (fmt == 16) {
        // C.LE.S - signaling less or equal single
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 <= f2);
    } else if (fmt == 17) {
        // C.LE.D - signaling less or equal double
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 <= f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

u32 C_NGT_FMT(VR4300& cpu, const Instruction& instr) {
    u8 fmt = instr.cop1_type.fmt;
    u8 fs = instr.cop1_type.fs;
    u8 ft = instr.cop1_type.ft;
    bool condition = false;

    // C.NGT signals Invalid exception for NaN
    if (fmt == 16) {
        // C.NGT.S - not greater than single (signaling less or equal or unordered)
        f32 f1 = cpu.cp1().get_fpr_single(fs);
        f32 f2 = cpu.cp1().get_fpr_single(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 <= f2) || std::isnan(f1) || std::isnan(f2);
    } else if (fmt == 17) {
        // C.NGT.D - not greater than double (signaling less or equal or unordered)
        if (!cpu.cp0().get_fr_bit() && ((fs & 1) || (ft & 1))) {
            // TODO: Reserved instruction exception for FR=0 with odd registers
        }
        f64 f1 = cpu.cp1().get_fpr_double(fs);
        f64 f2 = cpu.cp1().get_fpr_double(ft);
        if (std::isnan(f1) || std::isnan(f2)) {
            // TODO: Invalid operation exception
        }
        condition = (f1 <= f2) || std::isnan(f1) || std::isnan(f2);
    }
    cpu.cp1().set_condition_bit(condition);
    return 1;
}

} // namespace n64::cpu
