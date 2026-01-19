#include "vu_ops.hpp"
#include "rsp.hpp"

namespace n64::rcp {

// COP2 Multiply instructions
u8 VMULF(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMULU(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VRNDP(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMULQ(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMUDL(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMUDM(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMUDN(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMUDH(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMACF(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMACU(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VRNDN(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMACQ(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMADL(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMADM(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMADN(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMADH(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

// COP2 Add/Sub instructions
u8 VADD(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    for (u32 i = 0; i < 8; i++) {
        s32 element_vs = sign_extend16(static_cast<s16>(rsp.vu().read_element(vs, i)));
        s32 element_vt = sign_extend16(static_cast<s16>(rsp.vu().get_vt_element(vt, i, e)));
        s32 carry = (rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCO) >> i) & 1;

        s32 result = element_vs + element_vt + carry;

        if (result > 32767) {
            result = 32767;
        } else if (result < -32768) {
            result = -32768;
        }

        rsp.vu().write_element(vd, i, result);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, 0);
    return 1;
}

u8 VSUB(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    for (u32 i = 0; i < 8; i++) {
        s32 element_vs = sign_extend16(static_cast<s16>(rsp.vu().read_element(vs, i)));
        s32 element_vt = sign_extend16(static_cast<s16>(rsp.vu().get_vt_element(vt, i, e)));
        s32 carry = (rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCO) >> i) & 1;

        s32 result = element_vs - element_vt - carry;

        if (result > 32767) {
            result = 32767;
        } else if (result < -32768) {
            result = -32768;
        }
        rsp.vu().write_element(vd, i, result);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, 0);
    return 1;
}

u8 VABS(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        s16 result;
        if (element_vs < 0) {
            result = (element_vt == -32768) ? 32767 : -element_vt;
        } else if (element_vs > 0) {
            result = element_vt;
        } else {
            result = 0;
        }
        rsp.vu().write_element(vd, i, result);
        rsp.vu().set_accumulator_low(i, result);
    }
    return 1;
}

u8 VADDC(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    u16 new_vc0 = 0;
    for (u32 i = 0; i < 8; i++) {
        u32 element_vs = rsp.vu().read_element(vs, i);
        u32 element_vt = rsp.vu().get_vt_element(vt, i, e);
        u32 result = element_vs + element_vt;

        if (result > 0xFFFF) {
            new_vc0 |= 1 << i;
        }
        if ((result & 0xFFFF) != 0) {
            new_vc0 |= 1 << (i + 8);
        }
        rsp.vu().write_element(vd, i, result);
        rsp.vu().set_accumulator_low(i, result);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, new_vc0);
    return 1;
}

u8 VSUBC(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    u16 new_vc0 = 0;
    for (u32 i = 0; i < 8; i++) {
        s32 element_vs = sign_extend16(static_cast<s16>(rsp.vu().read_element(vs, i)));
        s32 element_vt = sign_extend16(static_cast<s16>(rsp.vu().get_vt_element(vt, i, e)));
        s32 result = element_vs - element_vt;

        if (result < 0) {
            new_vc0 |= 1 << i;
        }
        if ((result & 0xFFFF) != 0) {
            new_vc0 |= 1 << (i + 8);
        }
        rsp.vu().write_element(vd, i, result);
        rsp.vu().set_accumulator_low(i, result);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, new_vc0);
    return 1;
}

// COP2 Select/Compare instructions
u8 VLT(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VEQ(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VNE(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VGE(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VCL(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VCH(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VCR(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMRG(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

// COP2 Logical instructions
u8 VAND(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        rsp.vu().write_element(vd, i, element_vs & element_vt);
    }
    return 1;
}

u8 VNAND(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        rsp.vu().write_element(vd, i, ~(element_vs & element_vt));
    }
    return 1;
}

u8 VOR(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        rsp.vu().write_element(vd, i, element_vs | element_vt);
    }
    return 1;
}

u8 VNOR(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        rsp.vu().write_element(vd, i, ~(element_vs | element_vt));
    }
    return 1;
}

u8 VXOR(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        rsp.vu().write_element(vd, i, element_vs ^ element_vt);
    }
    return 1;
}

u8 VNXOR(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        rsp.vu().write_element(vd, i, ~(element_vs ^ element_vt));
    }
    return 1;
}

// COP2 Accumulator instructions
u8 VSAR(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

// COP2 Divide/Reciprocal instructions
u8 VRCP(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VRCPL(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VRCPH(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VMOV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VRSQ(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VRSQL(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VRSQH(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 VNOP(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

// LWC2 - Vector loads
u8 LBV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LSV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LLV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LDV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LQV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LRV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LPV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LUV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LHV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LFV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 LTV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

// SWC2 - Vector stores
u8 SBV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SSV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SLV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SDV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SQV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SRV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SPV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SUV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SHV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SFV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 SWV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

u8 STV(RSP& rsp, const RSPInstruction& instr)
{
    return 0;
}

} // namespace n64::rcp
