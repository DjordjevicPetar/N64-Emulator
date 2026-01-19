#include "su_ops.hpp"
#include "rsp.hpp"
#include "../../memory/memory_constants.hpp"

namespace n64::rcp {

// Main table - Branches & Jumps
u8 J(RSP& rsp, const RSPInstruction& instr)
{
    u32 target = (instr.j_type.target_address & 0x000003FF) << 2;
    rsp.delay_branch(target);
    return 1;
}

u8 JAL(RSP& rsp, const RSPInstruction& instr)
{
    u32 target = (instr.j_type.target_address & 0x000003FF) << 2;
    rsp.su().write_gpr(31, rsp.pc() + 4);
    rsp.delay_branch(target);
    return 1;
}

u8 BEQ(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    u32 rt = rsp.su().read_gpr(instr.i_type.rt);
    if (rs == rt) {
        s16 offset = sign_extend16(instr.i_type.immediate) << 2;
        u32 target = rsp.pc() + offset;
        rsp.delay_branch(target);
    }
    return 1;
}

u8 BNE(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    u32 rt = rsp.su().read_gpr(instr.i_type.rt);
    if (rs != rt) {
        s16 offset = sign_extend16(instr.i_type.immediate) << 2;
        u32 target = rsp.pc() + offset;
        rsp.delay_branch(target);
    }
    return 1;
}

u8 BLEZ(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    if (static_cast<s32>(rs) <= 0) {
        s16 offset = sign_extend16(instr.i_type.immediate) << 2;
        u32 target = rsp.pc() + offset;
        rsp.delay_branch(target);
    }
    return 1;
}

u8 BGTZ(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    if (static_cast<s32>(rs) > 0) {
        s16 offset = sign_extend16(instr.i_type.immediate) << 2;
        u32 target = rsp.pc() + offset;
        rsp.delay_branch(target);
    }
    return 1;
}

// Main table - Immediate arithmetic
u8 ADDI(RSP& rsp, const RSPInstruction& instr)
{
    s32 imm = sign_extend16(instr.i_type.immediate);
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.i_type.rs));
    rsp.su().write_gpr(instr.i_type.rt, rs + imm);
    return 1;
}

u8 ADDIU(RSP& rsp, const RSPInstruction& instr)
{
    s32 imm = sign_extend16(instr.i_type.immediate);
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.i_type.rs));
    rsp.su().write_gpr(instr.i_type.rt, rs + imm);
    return 1;
}

u8 SLTI(RSP& rsp, const RSPInstruction& instr)
{
    s32 imm = sign_extend16(instr.i_type.immediate);
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.i_type.rs));
    rsp.su().write_gpr(instr.i_type.rt, rs < imm);
    return 1;
}

u8 SLTIU(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = static_cast<u32>(sign_extend16(instr.i_type.immediate));
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    rsp.su().write_gpr(instr.i_type.rt, rs < imm);
    return 1;
}

u8 ANDI(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = instr.i_type.immediate;
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    rsp.su().write_gpr(instr.i_type.rt, rs & imm);
    return 1;
}

u8 ORI(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = instr.i_type.immediate;
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    rsp.su().write_gpr(instr.i_type.rt, rs | imm);
    return 1;
}

u8 XORI(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = instr.i_type.immediate;
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    rsp.su().write_gpr(instr.i_type.rt, rs ^ imm);
    return 1;
}

u8 LUI(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = instr.i_type.immediate;
    rsp.su().write_gpr(instr.i_type.rt, imm << 16);
    return 1;
}

// Main table - Load/Store
u8 LB(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = sign_extend16(instr.i_type.immediate);
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    u32 address = rs + imm;
    u8 value = rsp.read<u8>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
    rsp.su().write_gpr(instr.i_type.rt, sign_extend8(value));
    return 1;
}

u8 LH(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = sign_extend16(instr.i_type.immediate);
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    u32 address = rs + imm;
    u16 value = rsp.read<u16>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
    rsp.su().write_gpr(instr.i_type.rt, sign_extend16(value));
    return 1;
}

u8 LW(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = sign_extend16(instr.i_type.immediate);
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    u32 address = rs + imm;
    u32 value = rsp.read<u32>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
    rsp.su().write_gpr(instr.i_type.rt, value);
    return 1;
}

u8 LBU(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = sign_extend16(instr.i_type.immediate);
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    u32 address = rs + imm;
    u8 value = rsp.read<u8>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
    rsp.su().write_gpr(instr.i_type.rt, extend8(value));
    return 1;
}

u8 LHU(RSP& rsp, const RSPInstruction& instr)
{
    u32 imm = sign_extend16(instr.i_type.immediate);
    u32 rs = rsp.su().read_gpr(instr.i_type.rs);
    u32 address = rs + imm;
    u16 value = rsp.read<u16>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
    rsp.su().write_gpr(instr.i_type.rt, extend16(value));
    return 1;
}

u8 SB(RSP& rsp, const RSPInstruction& instr)
{
    s32 imm = sign_extend16(instr.i_type.immediate);
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.i_type.rs));
    u32 address = rs + imm;
    u8 value = rsp.su().read_gpr(instr.i_type.rt);
    rsp.write<u8>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    return 1;
}

u8 SH(RSP& rsp, const RSPInstruction& instr)
{
    s32 imm = sign_extend16(instr.i_type.immediate);
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.i_type.rs));
    u32 address = rs + imm;
    u16 value = rsp.su().read_gpr(instr.i_type.rt);
    rsp.write<u16>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    return 1;
}

u8 SW(RSP& rsp, const RSPInstruction& instr)
{
    s32 imm = sign_extend16(instr.i_type.immediate);
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.i_type.rs));
    u32 address = rs + imm;
    u32 value = rsp.su().read_gpr(instr.i_type.rt);
    rsp.write<u32>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    return 1;
}

// Special table - Shifts
u8 SLL(RSP& rsp, const RSPInstruction& instr)
{
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    u32 sa = instr.r_type.shift_amount;
    u32 value = rt << sa;
    rsp.su().write_gpr(instr.r_type.rd, value);
    return 1;
}

u8 SRL(RSP& rsp, const RSPInstruction& instr)
{
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    u32 sa = instr.r_type.shift_amount;
    u32 value = rt >> sa;
    rsp.su().write_gpr(instr.r_type.rd, value);
    return 1;
}

u8 SRA(RSP& rsp, const RSPInstruction& instr)
{
    s32 rt = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rt));
    u32 sa = instr.r_type.shift_amount;
    s32 value = rt >> sa;
    rsp.su().write_gpr(instr.r_type.rd, value);
    return 1;
}

u8 SLLV(RSP& rsp, const RSPInstruction& instr)
{
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    u32 sa = rsp.su().read_gpr(instr.r_type.rs) & 0x1F;
    u32 value = rt << sa;
    rsp.su().write_gpr(instr.r_type.rd, value);
    return 1;
}

u8 SRLV(RSP& rsp, const RSPInstruction& instr)
{
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    u32 sa = rsp.su().read_gpr(instr.r_type.rs) & 0x1F;
    u32 value = rt >> sa;
    rsp.su().write_gpr(instr.r_type.rd, value);
    return 1;
}

u8 SRAV(RSP& rsp, const RSPInstruction& instr)
{
    s32 rt = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rt));
    u32 sa = rsp.su().read_gpr(instr.r_type.rs) & 0x1F;
    s32 value = rt >> sa;
    rsp.su().write_gpr(instr.r_type.rd, value);
    return 1;
}

// Special table - Jumps
u8 JR(RSP& rsp, const RSPInstruction& instr)
{
    u32 target = rsp.su().read_gpr(instr.r_type.rs);
    rsp.delay_branch(target & 0x00000FFF);
    return 1;
}

u8 JALR(RSP& rsp, const RSPInstruction& instr)
{
    u32 target = rsp.su().read_gpr(instr.r_type.rs);
    rsp.su().write_gpr(instr.r_type.rd, rsp.pc() + 4);
    rsp.delay_branch(target & 0x00000FFF);
    return 1;
}

// Special table - System
u8 BREAK(RSP& rsp, const RSPInstruction& instr)
{
    rsp.set_breakpoint();
    return 1;
}

// Special table - Arithmetic
u8 ADD(RSP& rsp, const RSPInstruction& instr)
{
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rs));
    s32 rt = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rt));
    rsp.su().write_gpr(instr.r_type.rd, rs + rt);
    return 1;
}

u8 ADDU(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.r_type.rs);
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    rsp.su().write_gpr(instr.r_type.rd, rs + rt);
    return 1;
}

u8 SUB(RSP& rsp, const RSPInstruction& instr)
{
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rs));
    s32 rt = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rt));
    rsp.su().write_gpr(instr.r_type.rd, rs - rt);
    return 1;
}

u8 SUBU(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.r_type.rs);
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    rsp.su().write_gpr(instr.r_type.rd, rs - rt);
    return 1;
}

// Special table - Logical
u8 AND(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.r_type.rs);
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    rsp.su().write_gpr(instr.r_type.rd, rs & rt);
    return 1;
}

u8 OR(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.r_type.rs);
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    rsp.su().write_gpr(instr.r_type.rd, rs | rt);
    return 1;
}

u8 XOR(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.r_type.rs);
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    rsp.su().write_gpr(instr.r_type.rd, rs ^ rt);
    return 1;
}

u8 NOR(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.r_type.rs);
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    rsp.su().write_gpr(instr.r_type.rd, ~(rs | rt));
    return 1;
}

// Special table - Set
u8 SLT(RSP& rsp, const RSPInstruction& instr)
{
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rs));
    s32 rt = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rt));
    rsp.su().write_gpr(instr.r_type.rd, rs < rt);
    return 1;
}

u8 SLTU(RSP& rsp, const RSPInstruction& instr)
{
    u32 rs = rsp.su().read_gpr(instr.r_type.rs);
    u32 rt = rsp.su().read_gpr(instr.r_type.rt);
    rsp.su().write_gpr(instr.r_type.rd, rs < rt);
    return 1;
}

// Regimm table
u8 BLTZ(RSP& rsp, const RSPInstruction& instr)
{
    s32 target = sign_extend16(instr.i_type.immediate) << 2;
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rs));
    if (rs < 0) {
        rsp.delay_branch(rsp.pc() + target);
    }
    return 1;
}

u8 BGEZ(RSP& rsp, const RSPInstruction& instr)
{
    s32 target = sign_extend16(instr.i_type.immediate) << 2;
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rs));
    if (rs >= 0) {
        rsp.delay_branch(rsp.pc() + target);
    }
    return 1;
}

u8 BLTZAL(RSP& rsp, const RSPInstruction& instr)
{
    s32 target = sign_extend16(instr.i_type.immediate) << 2;
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rs));
    rsp.su().write_gpr(31, rsp.pc() + 4);
    if (rs < 0) {
        rsp.delay_branch(rsp.pc() + target);
    }
    return 1;
}

u8 BGEZAL(RSP& rsp, const RSPInstruction& instr)
{
    s32 target = sign_extend16(instr.i_type.immediate) << 2;
    s32 rs = static_cast<s32>(rsp.su().read_gpr(instr.r_type.rs));
    rsp.su().write_gpr(31, rsp.pc() + 4);
    if (rs >= 0) {
        rsp.delay_branch(rsp.pc() + target);
    }
    return 1;
}

// COP0 table
u8 MFC0(RSP& rsp, const RSPInstruction& instr)
{
    u32 value = rsp.read_cop0(instr.r_type.rd);
    rsp.su().write_gpr(instr.r_type.rt, value);
    return 1;
}

u8 MTC0(RSP& rsp, const RSPInstruction& instr)
{
    u32 value = rsp.su().read_gpr(instr.r_type.rt);
    rsp.write_cop0(instr.r_type.rd, value);
    return 1;
}

// COP2 COFUN table (scalar moves to/from vector regs)
u8 MFC2(RSP& rsp, const RSPInstruction& instr)
{
    u32 rd = instr.r_type.rd;
    u32 e = (instr.r_type.shift_amount >> 1) & 0x07;
    rsp.su().write_gpr(instr.r_type.rt, sign_extend16(rsp.vu().read_element(rd, e)));
    return 1;
}

u8 CFC2(RSP& rsp, const RSPInstruction& instr)
{
    rsp.su().write_gpr(instr.r_type.rt, rsp.vu().read_control_register(instr.r_type.rd));
    return 1;
}

u8 MTC2(RSP& rsp, const RSPInstruction& instr)
{
    u16 value = rsp.su().read_gpr(instr.r_type.rt);
    u32 e = (instr.r_type.shift_amount >> 1) & 0x07;
    u32 rd = instr.r_type.rd;
    rsp.vu().write_element(rd, e, value);
    return 1;
}

u8 CTC2(RSP& rsp, const RSPInstruction& instr)
{
    rsp.vu().write_control_register(instr.r_type.rd, rsp.su().read_gpr(instr.r_type.rt));
    return 1;
}

} // namespace n64::rcp
