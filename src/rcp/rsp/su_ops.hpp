#pragma once

#include "../../utils/types.hpp"
#include "rsp_instruction.hpp"

namespace n64::rcp {

class RSP;

// Main table - Branches & Jumps
u8 J(RSP& rsp, const RSPInstruction& instr);
u8 JAL(RSP& rsp, const RSPInstruction& instr);
u8 BEQ(RSP& rsp, const RSPInstruction& instr);
u8 BNE(RSP& rsp, const RSPInstruction& instr);
u8 BLEZ(RSP& rsp, const RSPInstruction& instr);
u8 BGTZ(RSP& rsp, const RSPInstruction& instr);

// Main table - Immediate arithmetic
u8 ADDI(RSP& rsp, const RSPInstruction& instr);
u8 ADDIU(RSP& rsp, const RSPInstruction& instr);
u8 SLTI(RSP& rsp, const RSPInstruction& instr);
u8 SLTIU(RSP& rsp, const RSPInstruction& instr);
u8 ANDI(RSP& rsp, const RSPInstruction& instr);
u8 ORI(RSP& rsp, const RSPInstruction& instr);
u8 XORI(RSP& rsp, const RSPInstruction& instr);
u8 LUI(RSP& rsp, const RSPInstruction& instr);

// Main table - Load/Store
u8 LB(RSP& rsp, const RSPInstruction& instr);
u8 LH(RSP& rsp, const RSPInstruction& instr);
u8 LW(RSP& rsp, const RSPInstruction& instr);
u8 LBU(RSP& rsp, const RSPInstruction& instr);
u8 LHU(RSP& rsp, const RSPInstruction& instr);
u8 SB(RSP& rsp, const RSPInstruction& instr);
u8 SH(RSP& rsp, const RSPInstruction& instr);
u8 SW(RSP& rsp, const RSPInstruction& instr);

// Special table - Shifts
u8 SLL(RSP& rsp, const RSPInstruction& instr);
u8 SRL(RSP& rsp, const RSPInstruction& instr);
u8 SRA(RSP& rsp, const RSPInstruction& instr);
u8 SLLV(RSP& rsp, const RSPInstruction& instr);
u8 SRLV(RSP& rsp, const RSPInstruction& instr);
u8 SRAV(RSP& rsp, const RSPInstruction& instr);

// Special table - Jumps
u8 JR(RSP& rsp, const RSPInstruction& instr);
u8 JALR(RSP& rsp, const RSPInstruction& instr);

// Special table - System
u8 BREAK(RSP& rsp, const RSPInstruction& instr);

// Special table - Arithmetic
u8 ADD(RSP& rsp, const RSPInstruction& instr);
u8 ADDU(RSP& rsp, const RSPInstruction& instr);
u8 SUB(RSP& rsp, const RSPInstruction& instr);
u8 SUBU(RSP& rsp, const RSPInstruction& instr);

// Special table - Logical
u8 AND(RSP& rsp, const RSPInstruction& instr);
u8 OR(RSP& rsp, const RSPInstruction& instr);
u8 XOR(RSP& rsp, const RSPInstruction& instr);
u8 NOR(RSP& rsp, const RSPInstruction& instr);

// Special table - Set
u8 SLT(RSP& rsp, const RSPInstruction& instr);
u8 SLTU(RSP& rsp, const RSPInstruction& instr);

// Regimm table
u8 BLTZ(RSP& rsp, const RSPInstruction& instr);
u8 BGEZ(RSP& rsp, const RSPInstruction& instr);
u8 BLTZAL(RSP& rsp, const RSPInstruction& instr);
u8 BGEZAL(RSP& rsp, const RSPInstruction& instr);

// COP0 table
u8 MFC0(RSP& rsp, const RSPInstruction& instr);
u8 MTC0(RSP& rsp, const RSPInstruction& instr);

// COP2 COFUN table (scalar moves to/from vector regs)
u8 MFC2(RSP& rsp, const RSPInstruction& instr);
u8 CFC2(RSP& rsp, const RSPInstruction& instr);
u8 MTC2(RSP& rsp, const RSPInstruction& instr);
u8 CTC2(RSP& rsp, const RSPInstruction& instr);

} // namespace n64::rcp
