#pragma once

#include "../../utils/types.hpp"
#include "rsp_instruction.hpp"

namespace n64::rcp {

class RSP;

// COP2 Multiply instructions
u8 VMULF(RSP& rsp, const RSPInstruction& instr);
u8 VMULU(RSP& rsp, const RSPInstruction& instr);
u8 VRNDP(RSP& rsp, const RSPInstruction& instr);
u8 VMULQ(RSP& rsp, const RSPInstruction& instr);
u8 VMUDL(RSP& rsp, const RSPInstruction& instr);
u8 VMUDM(RSP& rsp, const RSPInstruction& instr);
u8 VMUDN(RSP& rsp, const RSPInstruction& instr);
u8 VMUDH(RSP& rsp, const RSPInstruction& instr);
u8 VMACF(RSP& rsp, const RSPInstruction& instr);
u8 VMACU(RSP& rsp, const RSPInstruction& instr);
u8 VRNDN(RSP& rsp, const RSPInstruction& instr);
u8 VMACQ(RSP& rsp, const RSPInstruction& instr);
u8 VMADL(RSP& rsp, const RSPInstruction& instr);
u8 VMADM(RSP& rsp, const RSPInstruction& instr);
u8 VMADN(RSP& rsp, const RSPInstruction& instr);
u8 VMADH(RSP& rsp, const RSPInstruction& instr);

// COP2 Add/Sub instructions
u8 VADD(RSP& rsp, const RSPInstruction& instr);
u8 VSUB(RSP& rsp, const RSPInstruction& instr);
u8 VABS(RSP& rsp, const RSPInstruction& instr);
u8 VADDC(RSP& rsp, const RSPInstruction& instr);
u8 VSUBC(RSP& rsp, const RSPInstruction& instr);

// COP2 Select/Compare instructions
u8 VLT(RSP& rsp, const RSPInstruction& instr);
u8 VEQ(RSP& rsp, const RSPInstruction& instr);
u8 VNE(RSP& rsp, const RSPInstruction& instr);
u8 VGE(RSP& rsp, const RSPInstruction& instr);
u8 VCL(RSP& rsp, const RSPInstruction& instr);
u8 VCH(RSP& rsp, const RSPInstruction& instr);
u8 VCR(RSP& rsp, const RSPInstruction& instr);
u8 VMRG(RSP& rsp, const RSPInstruction& instr);

// COP2 Logical instructions
u8 VAND(RSP& rsp, const RSPInstruction& instr);
u8 VNAND(RSP& rsp, const RSPInstruction& instr);
u8 VOR(RSP& rsp, const RSPInstruction& instr);
u8 VNOR(RSP& rsp, const RSPInstruction& instr);
u8 VXOR(RSP& rsp, const RSPInstruction& instr);
u8 VNXOR(RSP& rsp, const RSPInstruction& instr);

// COP2 Accumulator instructions
u8 VSAR(RSP& rsp, const RSPInstruction& instr);

// COP2 Divide/Reciprocal instructions
u8 VRCP(RSP& rsp, const RSPInstruction& instr);
u8 VRCPL(RSP& rsp, const RSPInstruction& instr);
u8 VRCPH(RSP& rsp, const RSPInstruction& instr);
u8 VMOV(RSP& rsp, const RSPInstruction& instr);
u8 VRSQ(RSP& rsp, const RSPInstruction& instr);
u8 VRSQL(RSP& rsp, const RSPInstruction& instr);
u8 VRSQH(RSP& rsp, const RSPInstruction& instr);
u8 VNOP(RSP& rsp, const RSPInstruction& instr);

// LWC2 - Vector loads
u8 LBV(RSP& rsp, const RSPInstruction& instr);
u8 LSV(RSP& rsp, const RSPInstruction& instr);
u8 LLV(RSP& rsp, const RSPInstruction& instr);
u8 LDV(RSP& rsp, const RSPInstruction& instr);
u8 LQV(RSP& rsp, const RSPInstruction& instr);
u8 LRV(RSP& rsp, const RSPInstruction& instr);
u8 LPV(RSP& rsp, const RSPInstruction& instr);
u8 LUV(RSP& rsp, const RSPInstruction& instr);
u8 LHV(RSP& rsp, const RSPInstruction& instr);
u8 LFV(RSP& rsp, const RSPInstruction& instr);
u8 LTV(RSP& rsp, const RSPInstruction& instr);

// SWC2 - Vector stores
u8 SBV(RSP& rsp, const RSPInstruction& instr);
u8 SSV(RSP& rsp, const RSPInstruction& instr);
u8 SLV(RSP& rsp, const RSPInstruction& instr);
u8 SDV(RSP& rsp, const RSPInstruction& instr);
u8 SQV(RSP& rsp, const RSPInstruction& instr);
u8 SRV(RSP& rsp, const RSPInstruction& instr);
u8 SPV(RSP& rsp, const RSPInstruction& instr);
u8 SUV(RSP& rsp, const RSPInstruction& instr);
u8 SHV(RSP& rsp, const RSPInstruction& instr);
u8 SFV(RSP& rsp, const RSPInstruction& instr);
u8 SWV(RSP& rsp, const RSPInstruction& instr);
u8 STV(RSP& rsp, const RSPInstruction& instr);

} // namespace n64::rcp
