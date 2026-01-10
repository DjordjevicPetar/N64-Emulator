#include "cpu_ops.hpp"
#include "vr4300.hpp"
#include "cp0.hpp"

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
    u64 value = cpu.read_memory(address, IO_SIZE::IO_SIZE_BYTE);
    cpu.set_gpr(instr.i_type.rt, sign_extend8(value));
    return 1;
}

u32 LBU(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 value = cpu.read_memory(address, IO_SIZE::IO_SIZE_BYTE);
    cpu.set_gpr(instr.i_type.rt, extend8(value));
    return 1;
}

u32 LH(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 value = cpu.read_memory(address, IO_SIZE::IO_SIZE_HALF_WORD);
    cpu.set_gpr(instr.i_type.rt, sign_extend16(value));
    return 1;
}

u32 LHU(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 value = cpu.read_memory(address, IO_SIZE::IO_SIZE_HALF_WORD);
    cpu.set_gpr(instr.i_type.rt, extend16(value));
    return 1;
}

u32 LW(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 value = cpu.read_memory(address, IO_SIZE::IO_SIZE_WORD);
    cpu.set_gpr(instr.i_type.rt, sign_extend32(value));
    return 1;
}

u32 LWU(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 value = cpu.read_memory(address, IO_SIZE::IO_SIZE_WORD);
    cpu.set_gpr(instr.i_type.rt, extend32(value));
    return 1;
}

u32 LWL(VR4300& cpu, const Instruction& instr) {
    // Load Word Left - učitava bajtove u GORNJI deo registra
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(3);
    u32 offset = static_cast<u32>(addr & 3);
    
    u32 mem = static_cast<u32>(cpu.read_memory(aligned, IO_SIZE::IO_SIZE_WORD));
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
    
    u32 mem = static_cast<u32>(cpu.read_memory(aligned, IO_SIZE::IO_SIZE_WORD));
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
    u64 value = cpu.read_memory(address, IO_SIZE::IO_SIZE_DOUBLE_WORD);
    cpu.set_gpr(instr.i_type.rt, value);
    return 1;
}

u32 LDL(VR4300& cpu, const Instruction& instr) {
    // Load Doubleword Left - učitava bajtove u GORNJI deo registra
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(7);  // Poravnaj na 8 bajtova
    u32 offset = static_cast<u32>(addr & 7);
    
    u64 mem = cpu.read_memory(aligned, IO_SIZE::IO_SIZE_DOUBLE_WORD);
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
    
    u64 mem = cpu.read_memory(aligned, IO_SIZE::IO_SIZE_DOUBLE_WORD);
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
    u64 value = cpu.read_memory(address, IO_SIZE::IO_SIZE_WORD);
    cpu.set_gpr(instr.i_type.rt, sign_extend32(value));
    cpu.set_LLbit(true);
    cpu.cp0().set_reg(CP0Reg::LL_ADDR, cpu.cp0().translate_address(address));
    return 1;
}

u32 LLD(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 value = cpu.read_memory(address, IO_SIZE::IO_SIZE_DOUBLE_WORD);
    cpu.set_gpr(instr.i_type.rt, value);
    cpu.set_LLbit(true);
    cpu.cp0().set_reg(CP0Reg::LL_ADDR, cpu.cp0().translate_address(address));
    return 1;
}

// Store instructions
u32 SB(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    cpu.write_memory(address, cpu.gpr(instr.i_type.rt), IO_SIZE::IO_SIZE_BYTE);
    return 1;
}

u32 SH(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    cpu.write_memory(address, cpu.gpr(instr.i_type.rt), IO_SIZE::IO_SIZE_HALF_WORD);
    return 1;
}

u32 SW(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    cpu.write_memory(address, cpu.gpr(instr.i_type.rt), IO_SIZE::IO_SIZE_WORD);
    return 1;
}

u32 SWL(VR4300& cpu, const Instruction& instr) {
    // Store Word Left - upisuje GORNJE bajtove registra u memoriju
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(3);
    u32 offset = static_cast<u32>(addr & 3);
    
    u32 rt = static_cast<u32>(cpu.gpr(instr.i_type.rt));
    u32 mem = static_cast<u32>(cpu.read_memory(aligned, IO_SIZE::IO_SIZE_WORD));
    u32 result = 0;
    
    switch (offset) {
        case 0: result = rt; break;
        case 1: result = (mem & 0xFF000000) | (rt >> 8); break;
        case 2: result = (mem & 0xFFFF0000) | (rt >> 16); break;
        case 3: result = (mem & 0xFFFFFF00) | (rt >> 24); break;
    }
    
    cpu.write_memory(aligned, result, IO_SIZE::IO_SIZE_WORD);
    return 1;
}

u32 SWR(VR4300& cpu, const Instruction& instr) {
    // Store Word Right - upisuje DONJE bajtove registra u memoriju
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(3);
    u32 offset = static_cast<u32>(addr & 3);
    
    u32 rt = static_cast<u32>(cpu.gpr(instr.i_type.rt));
    u32 mem = static_cast<u32>(cpu.read_memory(aligned, IO_SIZE::IO_SIZE_WORD));
    u32 result = 0;
    
    switch (offset) {
        case 0: result = (mem & 0x00FFFFFF) | (rt << 24); break;
        case 1: result = (mem & 0x0000FFFF) | (rt << 16); break;
        case 2: result = (mem & 0x000000FF) | (rt << 8); break;
        case 3: result = rt; break;
    }
    
    cpu.write_memory(aligned, result, IO_SIZE::IO_SIZE_WORD);
    return 1;
}

u32 SD(VR4300& cpu, const Instruction& instr) {
    u64 address = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    cpu.write_memory(address, cpu.gpr(instr.i_type.rt), IO_SIZE::IO_SIZE_DOUBLE_WORD);
    return 1;
}

u32 SDL(VR4300& cpu, const Instruction& instr) {
    // Store Doubleword Left - upisuje GORNJE bajtove registra u memoriju
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(7);
    u32 offset = static_cast<u32>(addr & 7);
    
    u64 rt = cpu.gpr(instr.i_type.rt);
    u64 mem = cpu.read_memory(aligned, IO_SIZE::IO_SIZE_DOUBLE_WORD);
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
    
    cpu.write_memory(aligned, result, IO_SIZE::IO_SIZE_DOUBLE_WORD);
    return 1;
}

u32 SDR(VR4300& cpu, const Instruction& instr) {
    // Store Doubleword Right - upisuje DONJE bajtove registra u memoriju
    u64 addr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u64 aligned = addr & ~static_cast<u64>(7);
    u32 offset = static_cast<u32>(addr & 7);
    
    u64 rt = cpu.gpr(instr.i_type.rt);
    u64 mem = cpu.read_memory(aligned, IO_SIZE::IO_SIZE_DOUBLE_WORD);
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
    
    cpu.write_memory(aligned, result, IO_SIZE::IO_SIZE_DOUBLE_WORD);
    return 1;
}

u32 SC(VR4300& cpu, const Instruction& instr) {
    u64 vaddr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u32 paddr = cpu.cp0().translate_address(vaddr);
    u32 ll_addr = static_cast<u32>(cpu.cp0().get_reg(CP0Reg::LL_ADDR));
    
    bool success = cpu.get_LLbit() && (ll_addr == paddr);
    if (success) {
        cpu.write_memory(vaddr, cpu.gpr(instr.i_type.rt), IO_SIZE::IO_SIZE_WORD);
    }
    cpu.set_gpr(instr.i_type.rt, success ? 1 : 0);
    cpu.set_LLbit(false);
    return 1;
}

u32 SCD(VR4300& cpu, const Instruction& instr) {
    u64 vaddr = cpu.gpr(instr.i_type.rs) + sign_extend16(instr.i_type.immediate);
    u32 paddr = cpu.cp0().translate_address(vaddr);
    u32 ll_addr = static_cast<u32>(cpu.cp0().get_reg(CP0Reg::LL_ADDR));
    
    bool success = cpu.get_LLbit() && (ll_addr == paddr);
    if (success) {
        cpu.write_memory(vaddr, cpu.gpr(instr.i_type.rt), IO_SIZE::IO_SIZE_DOUBLE_WORD);
    }
    cpu.set_gpr(instr.i_type.rt, success ? 1 : 0);
    cpu.set_LLbit(false);
    return 1;
}

// FPU Load/Store
u32 LWC1(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 LDC1(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 SWC1(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 SDC1(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
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
    // TODO: Implement
    return 1;
}

u32 BREAK(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
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
    // TODO: Implement
    return 1;
}

u32 TGEU(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TLT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TLTU(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TEQ(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TNE(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
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
    // TODO: Implement
    return 1;
}

u32 TGEIU(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TLTI(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TLTIU(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TEQI(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TNEI(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

// ============================================================================
// COP0 Instructions
// ============================================================================

// Move from/to CP0
u32 MFC0(VR4300& cpu, const Instruction& instr) {
    cpu.set_gpr(instr.r_type.rt, static_cast<u32>(cpu.cp0().get_reg(instr.r_type.rd)));
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
    u64 status = cpu.cp0().get_reg(CP0Reg::STATUS);
    if (status & 0x4) {
        cpu.cp0().set_reg(CP0Reg::STATUS, status & ~0x4);
        cpu.set_pc(cpu.cp0().get_reg(CP0Reg::ERROR_EPC));
    } else {
        cpu.cp0().set_reg(CP0Reg::STATUS, status & ~0x2);
        cpu.set_pc(cpu.cp0().get_reg(CP0Reg::EPC));
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
    // TODO: Implement
    return 1;
}

u32 DMFC1(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 CFC1(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 MTC1(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 DMTC1(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 CTC1(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

// COP1 Branch
u32 BC1F(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 BC1T(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 BC1FL(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 BC1TL(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

// FPU Arithmetic (format determined by rs field)
u32 ADD_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement - check instr.i_type.rs for format (S/D)
    return 1;
}

u32 SUB_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 MUL_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 DIV_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 SQRT_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 ABS_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 MOV_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 NEG_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

// FPU Round/Truncate/Ceil/Floor to Long
u32 ROUND_L_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TRUNC_L_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 CEIL_L_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 FLOOR_L_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

// FPU Round/Truncate/Ceil/Floor to Word
u32 ROUND_W_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 TRUNC_W_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 CEIL_W_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 FLOOR_W_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

// FPU Convert
u32 CVT_S_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 CVT_D_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 CVT_W_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 CVT_L_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

// FPU Compare
u32 C_F_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_UN_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_EQ_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_UEQ_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_OLT_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_ULT_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_OLE_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_ULE_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_SF_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_NGLE_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_SEQ_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_NGL_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_LT_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_NGE_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_LE_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

u32 C_NGT_FMT(VR4300& cpu, const Instruction& instr) {
    // TODO: Implement
    return 1;
}

} // namespace n64::cpu
