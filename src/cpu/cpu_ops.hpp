#pragma once

#include "instruction.hpp"
#include "../utils/types.hpp"

namespace n64::cpu {

class VR4300;  // Forward declaration

// ============================================================================
// Main Table Instructions (by opcode)
// ============================================================================

// Jump instructions
u32 J(VR4300& cpu, const Instruction& instr);
u32 JAL(VR4300& cpu, const Instruction& instr);

// Branch instructions
u32 BEQ(VR4300& cpu, const Instruction& instr);
u32 BNE(VR4300& cpu, const Instruction& instr);
u32 BLEZ(VR4300& cpu, const Instruction& instr);
u32 BGTZ(VR4300& cpu, const Instruction& instr);
u32 BEQL(VR4300& cpu, const Instruction& instr);
u32 BNEL(VR4300& cpu, const Instruction& instr);
u32 BLEZL(VR4300& cpu, const Instruction& instr);
u32 BGTZL(VR4300& cpu, const Instruction& instr);

// Arithmetic immediate
u32 ADDI(VR4300& cpu, const Instruction& instr);
u32 ADDIU(VR4300& cpu, const Instruction& instr);
u32 SLTI(VR4300& cpu, const Instruction& instr);
u32 SLTIU(VR4300& cpu, const Instruction& instr);
u32 DADDI(VR4300& cpu, const Instruction& instr);
u32 DADDIU(VR4300& cpu, const Instruction& instr);

// Logical immediate
u32 ANDI(VR4300& cpu, const Instruction& instr);
u32 ORI(VR4300& cpu, const Instruction& instr);
u32 XORI(VR4300& cpu, const Instruction& instr);
u32 LUI(VR4300& cpu, const Instruction& instr);

// Load instructions
u32 LB(VR4300& cpu, const Instruction& instr);
u32 LBU(VR4300& cpu, const Instruction& instr);
u32 LH(VR4300& cpu, const Instruction& instr);
u32 LHU(VR4300& cpu, const Instruction& instr);
u32 LW(VR4300& cpu, const Instruction& instr);
u32 LWU(VR4300& cpu, const Instruction& instr);
u32 LWL(VR4300& cpu, const Instruction& instr);
u32 LWR(VR4300& cpu, const Instruction& instr);
u32 LD(VR4300& cpu, const Instruction& instr);
u32 LDL(VR4300& cpu, const Instruction& instr);
u32 LDR(VR4300& cpu, const Instruction& instr);
u32 LL(VR4300& cpu, const Instruction& instr);
u32 LLD(VR4300& cpu, const Instruction& instr);

// Store instructions
u32 SB(VR4300& cpu, const Instruction& instr);
u32 SH(VR4300& cpu, const Instruction& instr);
u32 SW(VR4300& cpu, const Instruction& instr);
u32 SWL(VR4300& cpu, const Instruction& instr);
u32 SWR(VR4300& cpu, const Instruction& instr);
u32 SD(VR4300& cpu, const Instruction& instr);
u32 SDL(VR4300& cpu, const Instruction& instr);
u32 SDR(VR4300& cpu, const Instruction& instr);
u32 SC(VR4300& cpu, const Instruction& instr);
u32 SCD(VR4300& cpu, const Instruction& instr);

// FPU Load/Store
u32 LWC1(VR4300& cpu, const Instruction& instr);
u32 LDC1(VR4300& cpu, const Instruction& instr);
u32 SWC1(VR4300& cpu, const Instruction& instr);
u32 SDC1(VR4300& cpu, const Instruction& instr);

// Cache
u32 CACHE(VR4300& cpu, const Instruction& instr);

// ============================================================================
// Special Table Instructions (opcode 0x00, by funct)
// ============================================================================

// Shift instructions
u32 SLL(VR4300& cpu, const Instruction& instr);
u32 SRL(VR4300& cpu, const Instruction& instr);
u32 SRA(VR4300& cpu, const Instruction& instr);
u32 SLLV(VR4300& cpu, const Instruction& instr);
u32 SRLV(VR4300& cpu, const Instruction& instr);
u32 SRAV(VR4300& cpu, const Instruction& instr);
u32 DSLL(VR4300& cpu, const Instruction& instr);
u32 DSRL(VR4300& cpu, const Instruction& instr);
u32 DSRA(VR4300& cpu, const Instruction& instr);
u32 DSLLV(VR4300& cpu, const Instruction& instr);
u32 DSRLV(VR4300& cpu, const Instruction& instr);
u32 DSRAV(VR4300& cpu, const Instruction& instr);
u32 DSLL32(VR4300& cpu, const Instruction& instr);
u32 DSRL32(VR4300& cpu, const Instruction& instr);
u32 DSRA32(VR4300& cpu, const Instruction& instr);

// Jump register
u32 JR(VR4300& cpu, const Instruction& instr);
u32 JALR(VR4300& cpu, const Instruction& instr);

// System
u32 SYSCALL(VR4300& cpu, const Instruction& instr);
u32 BREAK(VR4300& cpu, const Instruction& instr);
u32 SYNC(VR4300& cpu, const Instruction& instr);

// Move from/to HI/LO
u32 MFHI(VR4300& cpu, const Instruction& instr);
u32 MTHI(VR4300& cpu, const Instruction& instr);
u32 MFLO(VR4300& cpu, const Instruction& instr);
u32 MTLO(VR4300& cpu, const Instruction& instr);

// Multiply/Divide
u32 MULT(VR4300& cpu, const Instruction& instr);
u32 MULTU(VR4300& cpu, const Instruction& instr);
u32 DIV(VR4300& cpu, const Instruction& instr);
u32 DIVU(VR4300& cpu, const Instruction& instr);
u32 DMULT(VR4300& cpu, const Instruction& instr);
u32 DMULTU(VR4300& cpu, const Instruction& instr);
u32 DDIV(VR4300& cpu, const Instruction& instr);
u32 DDIVU(VR4300& cpu, const Instruction& instr);

// Arithmetic
u32 ADD(VR4300& cpu, const Instruction& instr);
u32 ADDU(VR4300& cpu, const Instruction& instr);
u32 SUB(VR4300& cpu, const Instruction& instr);
u32 SUBU(VR4300& cpu, const Instruction& instr);
u32 DADD(VR4300& cpu, const Instruction& instr);
u32 DADDU(VR4300& cpu, const Instruction& instr);
u32 DSUB(VR4300& cpu, const Instruction& instr);
u32 DSUBU(VR4300& cpu, const Instruction& instr);

// Logical
u32 AND(VR4300& cpu, const Instruction& instr);
u32 OR(VR4300& cpu, const Instruction& instr);
u32 XOR(VR4300& cpu, const Instruction& instr);
u32 NOR(VR4300& cpu, const Instruction& instr);

// Set less than
u32 SLT(VR4300& cpu, const Instruction& instr);
u32 SLTU(VR4300& cpu, const Instruction& instr);

// Trap
u32 TGE(VR4300& cpu, const Instruction& instr);
u32 TGEU(VR4300& cpu, const Instruction& instr);
u32 TLT(VR4300& cpu, const Instruction& instr);
u32 TLTU(VR4300& cpu, const Instruction& instr);
u32 TEQ(VR4300& cpu, const Instruction& instr);
u32 TNE(VR4300& cpu, const Instruction& instr);

// ============================================================================
// REGIMM Table Instructions (opcode 0x01, by rt)
// ============================================================================

u32 BLTZ(VR4300& cpu, const Instruction& instr);
u32 BGEZ(VR4300& cpu, const Instruction& instr);
u32 BLTZL(VR4300& cpu, const Instruction& instr);
u32 BGEZL(VR4300& cpu, const Instruction& instr);
u32 BLTZAL(VR4300& cpu, const Instruction& instr);
u32 BGEZAL(VR4300& cpu, const Instruction& instr);
u32 BLTZALL(VR4300& cpu, const Instruction& instr);
u32 BGEZALL(VR4300& cpu, const Instruction& instr);

// Trap immediate
u32 TGEI(VR4300& cpu, const Instruction& instr);
u32 TGEIU(VR4300& cpu, const Instruction& instr);
u32 TLTI(VR4300& cpu, const Instruction& instr);
u32 TLTIU(VR4300& cpu, const Instruction& instr);
u32 TEQI(VR4300& cpu, const Instruction& instr);
u32 TNEI(VR4300& cpu, const Instruction& instr);

// ============================================================================
// COP0 Instructions
// ============================================================================

// Move from/to CP0
u32 MFC0(VR4300& cpu, const Instruction& instr);
u32 DMFC0(VR4300& cpu, const Instruction& instr);
u32 MTC0(VR4300& cpu, const Instruction& instr);
u32 DMTC0(VR4300& cpu, const Instruction& instr);

// TLB instructions
u32 TLBR(VR4300& cpu, const Instruction& instr);
u32 TLBWI(VR4300& cpu, const Instruction& instr);
u32 TLBWR(VR4300& cpu, const Instruction& instr);
u32 TLBP(VR4300& cpu, const Instruction& instr);
u32 ERET(VR4300& cpu, const Instruction& instr);

// COP0 Branch
u32 BC0F(VR4300& cpu, const Instruction& instr);
u32 BC0T(VR4300& cpu, const Instruction& instr);
u32 BC0FL(VR4300& cpu, const Instruction& instr);
u32 BC0TL(VR4300& cpu, const Instruction& instr);

// ============================================================================
// COP1 (FPU) Instructions
// ============================================================================

// Move from/to CP1
u32 MFC1(VR4300& cpu, const Instruction& instr);
u32 DMFC1(VR4300& cpu, const Instruction& instr);
u32 CFC1(VR4300& cpu, const Instruction& instr);
u32 MTC1(VR4300& cpu, const Instruction& instr);
u32 DMTC1(VR4300& cpu, const Instruction& instr);
u32 CTC1(VR4300& cpu, const Instruction& instr);

// COP1 Branch
u32 BC1F(VR4300& cpu, const Instruction& instr);
u32 BC1T(VR4300& cpu, const Instruction& instr);
u32 BC1FL(VR4300& cpu, const Instruction& instr);
u32 BC1TL(VR4300& cpu, const Instruction& instr);

// FPU Arithmetic (format determined by rs field)
u32 ADD_FMT(VR4300& cpu, const Instruction& instr);
u32 SUB_FMT(VR4300& cpu, const Instruction& instr);
u32 MUL_FMT(VR4300& cpu, const Instruction& instr);
u32 DIV_FMT(VR4300& cpu, const Instruction& instr);
u32 SQRT_FMT(VR4300& cpu, const Instruction& instr);
u32 ABS_FMT(VR4300& cpu, const Instruction& instr);
u32 MOV_FMT(VR4300& cpu, const Instruction& instr);
u32 NEG_FMT(VR4300& cpu, const Instruction& instr);

// FPU Round/Truncate/Ceil/Floor to Long
u32 ROUND_L_FMT(VR4300& cpu, const Instruction& instr);
u32 TRUNC_L_FMT(VR4300& cpu, const Instruction& instr);
u32 CEIL_L_FMT(VR4300& cpu, const Instruction& instr);
u32 FLOOR_L_FMT(VR4300& cpu, const Instruction& instr);

// FPU Round/Truncate/Ceil/Floor to Word
u32 ROUND_W_FMT(VR4300& cpu, const Instruction& instr);
u32 TRUNC_W_FMT(VR4300& cpu, const Instruction& instr);
u32 CEIL_W_FMT(VR4300& cpu, const Instruction& instr);
u32 FLOOR_W_FMT(VR4300& cpu, const Instruction& instr);

// FPU Convert
u32 CVT_S_FMT(VR4300& cpu, const Instruction& instr);
u32 CVT_D_FMT(VR4300& cpu, const Instruction& instr);
u32 CVT_W_FMT(VR4300& cpu, const Instruction& instr);
u32 CVT_L_FMT(VR4300& cpu, const Instruction& instr);

// FPU Compare
u32 C_F_FMT(VR4300& cpu, const Instruction& instr);
u32 C_UN_FMT(VR4300& cpu, const Instruction& instr);
u32 C_EQ_FMT(VR4300& cpu, const Instruction& instr);
u32 C_UEQ_FMT(VR4300& cpu, const Instruction& instr);
u32 C_OLT_FMT(VR4300& cpu, const Instruction& instr);
u32 C_ULT_FMT(VR4300& cpu, const Instruction& instr);
u32 C_OLE_FMT(VR4300& cpu, const Instruction& instr);
u32 C_ULE_FMT(VR4300& cpu, const Instruction& instr);
u32 C_SF_FMT(VR4300& cpu, const Instruction& instr);
u32 C_NGLE_FMT(VR4300& cpu, const Instruction& instr);
u32 C_SEQ_FMT(VR4300& cpu, const Instruction& instr);
u32 C_NGL_FMT(VR4300& cpu, const Instruction& instr);
u32 C_LT_FMT(VR4300& cpu, const Instruction& instr);
u32 C_NGE_FMT(VR4300& cpu, const Instruction& instr);
u32 C_LE_FMT(VR4300& cpu, const Instruction& instr);
u32 C_NGT_FMT(VR4300& cpu, const Instruction& instr);

} // namespace n64::cpu
