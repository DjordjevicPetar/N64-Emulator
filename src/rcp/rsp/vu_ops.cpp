#include "vu_ops.hpp"
#include "rsp.hpp"
#include "../../memory/memory_constants.hpp"

namespace n64::rcp {

// COP2 Multiply instructions
u8 VMULF(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);
        
        s64 acc = (static_cast<s64>(product) << 1) + 0x8000;

        rsp.vu().set_accumulator_high(i, (acc >> 32) & 0xFFFF);
        rsp.vu().set_accumulator_mid(i, (acc >> 16) & 0xFFFF);
        rsp.vu().set_accumulator_low(i, acc & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed(acc >> 16));
    }
    return 1;
}

u8 VMULU(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);
        
        s64 acc = (static_cast<s64>(product) << 1) + 0x8000;

        rsp.vu().set_accumulator_high(i, (acc >> 32) & 0xFFFF);
        rsp.vu().set_accumulator_mid(i, (acc >> 16) & 0xFFFF);
        rsp.vu().set_accumulator_low(i, acc & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_unsigned(static_cast<s32>(acc >> 16)));
    }
    return 1;
}

u8 VRNDP(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        u64 acc_u = (static_cast<u64>(rsp.vu().get_accumulator_high(i) & 0xFFFF) << 32) |
                    (static_cast<u64>(rsp.vu().get_accumulator_mid(i) & 0xFFFF) << 16) |
                    (static_cast<u64>(rsp.vu().get_accumulator_low(i) & 0xFFFF));
        s64 acc = static_cast<s64>(acc_u << 16) >> 16;
        
        s32 product;
        if (vs & 1) {
            product = static_cast<s32>(static_cast<u16>(element_vt)) << 16;
        } else {
            product = static_cast<s32>(element_vt);
        }
        
        if (acc >= 0) {
            acc += static_cast<s64>(product);
        }

        rsp.vu().set_accumulator_high(i, (acc >> 32) & 0xFFFF);
        rsp.vu().set_accumulator_mid(i, (acc >> 16) & 0xFFFF);
        rsp.vu().set_accumulator_low(i, acc & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed((acc >> 16) & 0xFFFF));
    }
    return 1;
}

u8 VMULQ(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);
        s32 result = 0;
        
        if (product < 0) {
            result = (product & 0xFFFF) + 0x0020;
        } else {
            result = product & 0xFFFF;
        }

        rsp.vu().set_accumulator_high(i, result >> 16);
        rsp.vu().set_accumulator_mid(i, result & 0xFFFF);
        rsp.vu().set_accumulator_low(i, 0);
        
        rsp.vu().write_element(vd, i, clamp_signed((result >> 1) & 0xFFFF) & 0xFFF0);
    }
    return 1;
}

u8 VMUDL(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        
        u32 product = static_cast<u32>(element_vs) * static_cast<u32>(element_vt);
        u16 sign_ext = (product & 0x80000000) ? 0xFFFF : 0x0000;
        s16 upper = product >> 16;

        rsp.vu().set_accumulator_high(i, 0);
        rsp.vu().set_accumulator_mid(i, sign_ext);
        rsp.vu().set_accumulator_low(i, upper);
        
        rsp.vu().write_element(vd, i, clamp_signed(upper));
    }
    return 1;
}

u8 VMUDM(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);

        rsp.vu().set_accumulator_high(i, 0);
        rsp.vu().set_accumulator_mid(i, product >> 16);
        rsp.vu().set_accumulator_low(i, product & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed(product >> 16));
    }
    return 1;
}

u8 VMUDN(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);

        rsp.vu().set_accumulator_high(i, 0);
        rsp.vu().set_accumulator_mid(i, product >> 16);
        rsp.vu().set_accumulator_low(i, product & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed(product & 0xFFFF));
    }
    return 1;
}

u8 VMUDH(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);

        rsp.vu().set_accumulator_high(i, 0);
        rsp.vu().set_accumulator_mid(i, product >> 16);
        rsp.vu().set_accumulator_low(i, 0);
        
        rsp.vu().write_element(vd, i, clamp_signed(product >> 16));
    }
    return 1;
}

u8 VMACF(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);
        s32 acc_47_16 = (product << 1) + (rsp.vu().get_accumulator_high(i) << 16 | rsp.vu().get_accumulator_mid(i));

        rsp.vu().set_accumulator_high(i, acc_47_16 >> 16);
        rsp.vu().set_accumulator_mid(i, acc_47_16 & 0xFFFF);
        rsp.vu().set_accumulator_low(i, 0);
        
        rsp.vu().write_element(vd, i, clamp_signed(acc_47_16 & 0xFFFF));
    }
    return 1;
}

u8 VMACU(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);
        s32 acc_47_16 = (product << 1) + (rsp.vu().get_accumulator_high(i) << 16 | rsp.vu().get_accumulator_mid(i));

        rsp.vu().set_accumulator_high(i, acc_47_16 >> 16);
        rsp.vu().set_accumulator_mid(i, acc_47_16 & 0xFFFF);
        rsp.vu().set_accumulator_low(i, 0);
        
        rsp.vu().write_element(vd, i, clamp_unsigned(acc_47_16 >> 16));
    }
    return 1;
}

u8 VRNDN(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        u64 acc_u = (static_cast<u64>(rsp.vu().get_accumulator_high(i) & 0xFFFF) << 32) |
                    (static_cast<u64>(rsp.vu().get_accumulator_mid(i) & 0xFFFF) << 16) |
                    (static_cast<u64>(rsp.vu().get_accumulator_low(i) & 0xFFFF));
        s64 acc = static_cast<s64>(acc_u << 16) >> 16;
        
        s32 product;
        if (vs & 1) {
            product = static_cast<s32>(static_cast<u16>(element_vt)) << 16;
        } else {
            product = static_cast<s32>(element_vt);
        }
        
        if (acc < 0) {
            acc += static_cast<s64>(product);
        }

        rsp.vu().set_accumulator_high(i, (acc >> 32) & 0xFFFF);
        rsp.vu().set_accumulator_mid(i, (acc >> 16) & 0xFFFF);
        rsp.vu().set_accumulator_low(i, acc & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed((acc >> 16) & 0xFFFF));
    }
    return 1;
}

u8 VMACQ(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        
        u64 acc_u = (static_cast<u64>(rsp.vu().get_accumulator_high(i) & 0xFFFF) << 32) |
                    (static_cast<u64>(rsp.vu().get_accumulator_mid(i) & 0xFFFF) << 16) |
                    (static_cast<u64>(rsp.vu().get_accumulator_low(i) & 0xFFFF));
        s64 acc = static_cast<s64>(acc_u << 16) >> 16;
        bool acc_21 = (acc >> 21) & 1;
        
        if (acc < 0 && !acc_21) {
            acc += 0x200000;
        } else if (acc > 0 && !acc_21) {
            acc -= 0x200000;
        }

        rsp.vu().set_accumulator_high(i, (acc >> 32) & 0xFFFF);
        rsp.vu().set_accumulator_mid(i, (acc >> 16) & 0xFFFF);
        rsp.vu().set_accumulator_low(i, acc & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed((acc >> 17) & 0xFFFF));
    }
    return 1;
}

u8 VMADL(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        
        u32 product = static_cast<u32>(element_vs) * static_cast<u32>(element_vt);
        s32 acc_31_0 = product + rsp.vu().get_accumulator_mid(i);

        rsp.vu().set_accumulator_high(i, 0);
        rsp.vu().set_accumulator_mid(i, acc_31_0 >> 16);
        rsp.vu().set_accumulator_low(i, acc_31_0 & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed(acc_31_0 & 0xFFFF));
    }
    return 1;
}

u8 VMADM(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        u16 element_vt = rsp.vu().get_vt_element(vt, i, e);
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);
        s32 acc_31_0 = product + rsp.vu().get_accumulator_low(i) + (rsp.vu().get_accumulator_mid(i) << 16);

        rsp.vu().set_accumulator_high(i, 0);
        rsp.vu().set_accumulator_mid(i, acc_31_0 >> 16);
        rsp.vu().set_accumulator_low(i, acc_31_0 & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed(acc_31_0 >> 16));
    }
    return 1;
}

u8 VMADN(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        u16 element_vs = rsp.vu().read_element(vs, i);
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);
        s32 acc_31_0 = product + rsp.vu().get_accumulator_low(i) + (rsp.vu().get_accumulator_mid(i) << 16);

        rsp.vu().set_accumulator_high(i, 0);
        rsp.vu().set_accumulator_mid(i, acc_31_0 >> 16);
        rsp.vu().set_accumulator_low(i, acc_31_0 & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed(acc_31_0 & 0xFFFF));
    }
    return 1;
}

u8 VMADH(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    
    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, e));
        
        s32 product = static_cast<s32>(element_vs) * static_cast<s32>(element_vt);
        s32 acc_31_0 = product + (rsp.vu().get_accumulator_mid(i) << 16);

        rsp.vu().set_accumulator_high(i, 0);
        rsp.vu().set_accumulator_mid(i, acc_31_0 >> 16);
        rsp.vu().set_accumulator_low(i, acc_31_0 & 0xFFFF);
        
        rsp.vu().write_element(vd, i, clamp_signed(acc_31_0 >> 16));
    }
    return 1;
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
        rsp.vu().write_element(vd, i, clamp_signed(result));
        rsp.vu().set_accumulator_low(i, result);
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
        rsp.vu().write_element(vd, i, clamp_signed(result));
        rsp.vu().set_accumulator_low(i, result);
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
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 elem = instr.v_type.e;
    u32 new_vcc = 0;
    u16 old_vco = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCO);
    u8 old_vce = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCE);
    s16 result = 0;

    for (u32 i = 0; i < 8; i++) {
        bool vco_i = (old_vco >> i) & 1;
        bool vce_i = (old_vce >> i) & 1;

        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, elem));

        if (element_vs < element_vt) {
            new_vcc |= 1 << i;
            result = element_vs;
        } else if ((element_vs == element_vt) && vco_i && !vce_i) {
            new_vcc |= 1 << i;
            result = element_vs;
        } else {
            result = element_vt;
        }

        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCC, new_vcc);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCE, 0);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, 0);
    return 1;
}

u8 VEQ(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 elem = instr.v_type.e;
    u32 new_vcc = 0;
    s16 result = 0;

    u8 old_vce = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCE);

    for (u32 i = 0; i < 8; i++) {
        bool vce_i = (old_vce >> i) & 1;

        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, elem));

        if ((element_vs == element_vt) && vce_i) {
            new_vcc |= 1 << i;
            result = element_vs;
        } else {
            result = element_vt;
        }

        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCC, new_vcc);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCE, 0);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, 0);
    return 1;
}

u8 VNE(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 elem = instr.v_type.e;
    u16 new_vcc = 0;
    u8 old_vce = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCE);
    s16 result = 0;

    for (u32 i = 0; i < 8; i++) {
        bool vce_i = (old_vce >> i) & 1;

        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, elem));

        if (element_vs < element_vt) {
            new_vcc |= 1 << i;
            result = element_vs;
        } else if (element_vs > element_vt) {
            new_vcc |= 1 << i;
            result = element_vs;
        } else if ((element_vs == element_vt) && !vce_i) {
            new_vcc |= 1 << i;
            result = element_vs;
        } else {
            result = element_vt;
        }

        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCC, new_vcc);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCE, 0);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, 0);
    return 1;
}

u8 VGE(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 elem = instr.v_type.e;
    u16 new_vcc = 0;
    u16 old_vco = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCO);
    u8 old_vce = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCE);

    for (u32 i = 0; i < 8; i++) {
        bool vco_i = (old_vco >> i) & 1;
        bool vce_i = (old_vce >> i) & 1;

        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, elem));
        s16 result;

        if (element_vs > element_vt) {
            new_vcc |= 1 << i;
            result = element_vs;
        } else if ((element_vs == element_vt) && (!vco_i || vce_i)) {
            new_vcc |= 1 << i;
            result = element_vs;
        } else {
            result = element_vt;
        }

        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCC, new_vcc);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCE, 0);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, 0);
    return 1;
}

u8 VCL(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 elem = instr.v_type.e;
    u16 old_vcc = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCC);
    u16 new_vcc = old_vcc;
    u16 old_vco = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCO);
    u8 old_vce = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCE);

    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, elem));
        bool sign = (old_vco >> i) & 1;
        bool ge = (old_vcc >> (i + 8)) & 1;
        bool le = (old_vcc >> i) & 1;
        bool vce = (old_vce >> i) & 1;
        bool eq = !((old_vco >> (i + 8)) & 1);
        s16 di;

        if (sign) {
            u32 sum = static_cast<u16>(element_vs) + static_cast<u16>(element_vt);
            bool carry = sum > 0xFFFF;
            di = static_cast<s16>(sum & 0xFFFF);
            
            if (eq) {
                le = (!vce && (di == 0) && !carry) || (vce && ((di == 0) || !carry));
            }
            di = le ? -element_vt : element_vs;
        } else {
            di = element_vs - element_vt;
            if (eq) {
                ge = di >= 0;
            }
            di = ge ? element_vt : element_vs;
        }

        rsp.vu().set_accumulator_low(i, di);
        rsp.vu().write_element(vd, i, di);
        new_vcc = (new_vcc & ~(0x0101 << i)) | (ge << (i + 8)) | (le << i);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCC, new_vcc);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCE, 0);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, 0);
    return 1;
}

u8 VCH(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 elem = instr.v_type.e;
    u16 new_vcc = 0;
    u16 new_vco = 0;
    u8 new_vce = 0;

    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, elem));
        bool sign = (element_vs ^ element_vt) < 0;
        bool ge;
        bool le;
        bool vce;
        bool eq;
        s16 di;

        if (sign) {
            ge = element_vt < 0;
            le = (element_vs + element_vt) <= 0;
            vce = (element_vs + element_vt) == -1;
            eq = (element_vs + element_vt) == 0;
            di = le ? -element_vt : element_vs;
        } else {
            le = element_vt < 0;
            ge = (element_vs - element_vt) >= 0;
            vce = false;
            eq = (element_vs - element_vt) == 0;
            di = ge ? element_vt : element_vs;
        }

        rsp.vu().set_accumulator_low(i, di);
        rsp.vu().write_element(vd, i, di);
        bool neq = !eq;
        new_vcc |= (ge << (i + 8)) | (le << i);
        new_vco |= (neq << (i + 8)) | (sign << i);
        new_vce |= (vce << i);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCC, new_vcc);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCE, new_vce);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, new_vco);
    return 1;
}

u8 VCR(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 elem = instr.v_type.e;
    u16 new_vcc = 0;

    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, elem));
        bool sign = (element_vs ^ element_vt) < 0;
        bool ge;
        bool le;
        s16 di;

        if (sign) {
            ge = element_vt < 0;
            le = (element_vs + element_vt + 1) <= 0;
            di = le ? ~element_vt : element_vs;
        } else {
            le = element_vt < 0;
            ge = (element_vs - element_vt) >= 0;
            di = ge ? element_vt : element_vs;
        }

        rsp.vu().set_accumulator_low(i, di);
        rsp.vu().write_element(vd, i, di);
        new_vcc |= (ge << (i + 8)) | (le << i);
    }
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCC, new_vcc);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCE, 0);
    rsp.vu().write_control_register(VU_CONTROL_REGISTER_VCO, 0);
    return 1;
}

u8 VMRG(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 elem = instr.v_type.e;
    u16 vcc = rsp.vu().read_control_register(VU_CONTROL_REGISTER_VCC);

    for (u32 i = 0; i < 8; i++) {
        s16 element_vs = static_cast<s16>(rsp.vu().read_element(vs, i));
        s16 element_vt = static_cast<s16>(rsp.vu().get_vt_element(vt, i, elem));
        
        s16 result = ((vcc >> i) & 1) ? element_vs : element_vt;

        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
    }
    return 1;
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
        u16 result = element_vs & element_vt;
        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
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
        u16 result = ~(element_vs & element_vt);
        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
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
        u16 result = element_vs | element_vt;
        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
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
        u16 result = ~(element_vs | element_vt);
        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
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
        u16 result = element_vs ^ element_vt;
        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
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
        u16 result = ~(element_vs ^ element_vt);
        rsp.vu().set_accumulator_low(i, result);
        rsp.vu().write_element(vd, i, result);
    }
    return 1;
}

// COP2 Accumulator instructions
u8 VSAR(RSP& rsp, const RSPInstruction& instr)
{
    u32 vs = instr.v_type.vs;
    u32 vd = instr.v_type.vd;
    u32 elem = instr.v_type.e;

    switch(elem) {
        case 0:  // ACC high - swap
            for (u32 i = 0; i < 8; i++) {
                u16 acc_val = rsp.vu().get_accumulator_high(i);
                u16 vs_val = rsp.vu().read_element(vs, i);
                rsp.vu().write_element(vd, i, acc_val);
                rsp.vu().set_accumulator_high(i, vs_val);
            }
            break;
        case 1:  // ACC mid - swap
            for (u32 i = 0; i < 8; i++) {
                u16 acc_val = rsp.vu().get_accumulator_mid(i);
                u16 vs_val = rsp.vu().read_element(vs, i);
                rsp.vu().write_element(vd, i, acc_val);
                rsp.vu().set_accumulator_mid(i, vs_val);
            }
            break;
        case 2:  // ACC low - swap
            for (u32 i = 0; i < 8; i++) {
                u16 acc_val = rsp.vu().get_accumulator_low(i);
                u16 vs_val = rsp.vu().read_element(vs, i);
                rsp.vu().write_element(vd, i, acc_val);
                rsp.vu().set_accumulator_low(i, vs_val);
            }
            break;
        case 8:  // ACC high - read only
            for (u32 i = 0; i < 8; i++) {
                rsp.vu().write_element(vd, i, rsp.vu().get_accumulator_high(i));
            }
            break;
        case 9:  // ACC mid - read only
            for (u32 i = 0; i < 8; i++) {
                rsp.vu().write_element(vd, i, rsp.vu().get_accumulator_mid(i));
            }
            break;
        case 10: // ACC low - read only
            for (u32 i = 0; i < 8; i++) {
                rsp.vu().write_element(vd, i, rsp.vu().get_accumulator_low(i));
            }
            break;
        default:
            for (u32 i = 0; i < 8; i++) {
                rsp.vu().write_element(vd, i, 0);
            }
            break;
    }
    return 1;
}

// COP2 Divide/Reciprocal instructions
u8 VRCP(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e;
    u32 de = instr.v_type.vs & 7;
    
    s32 input = static_cast<s16>(rsp.vu().read_element(vt, e & 7));
    s32 data = (input < 0) ? -input : input;
    
    s32 result;
    if (data != 0) {
        u32 shift = 0;
        while (!(data & (1 << 15)) && shift < 16) {
            data <<= 1;
            shift++;
        }
        
        u32 index = (data >> 6) & 0x1FF;
        
        u32 reciprocal = rsp.vu().get_reciprocal(index);
        result = (0x10000 | reciprocal) << 14;
        result >>= (31 - shift);
    } else {
        result = 0x7FFFFFFF;
    }
    
    if (input < 0) {
        result = ~result;
    }
    
    rsp.vu().set_div_out(result);
    
    u16 vt_element = rsp.vu().read_element(vt, e & 7);
    for (u32 i = 0; i < 8; i++) {
        rsp.vu().set_accumulator_low(i, vt_element);
    }
    
    rsp.vu().write_element(vd, de, result & 0xFFFF);
    return 1;
}

u8 VRCPL(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e & 7;
    u32 de = instr.v_type.vs & 7;
    
    s32 input;
    if (rsp.vu().get_div_dp()) {
        // Double precision: high from VRCPH, low from current element
        input = rsp.vu().get_div_in() | rsp.vu().read_element(vt, e);
    } else {
        // Single precision: sign extend 16-bit to 32-bit
        input = static_cast<s16>(rsp.vu().read_element(vt, e));
    }
    
    s32 data = (input < 0) ? -input : input;
    
    s32 result;
    if (data != 0) {
        // Count leading zeros for 32-bit value
        u32 shift = 0;
        while (!(data & (1 << 31)) && shift < 32) {
            data <<= 1;
            shift++;
        }
        
        // Extract 9-bit index (bits 30:22 after normalization)
        u32 index = (data >> 22) & 0x1FF;
        
        // Get reciprocal and reconstruct
        u32 reciprocal = rsp.vu().get_reciprocal(index);
        result = (0x10000 | reciprocal) << 14;
        result >>= (31 - shift);
    } else {
        result = 0x7FFFFFFF;
    }
    
    if (input < 0) {
        result = ~result;
    }
    
    rsp.vu().set_div_out(result);
    rsp.vu().set_div_dp(false);
    
    // Broadcast VT[e] to all ACC lows
    u16 vt_element = rsp.vu().read_element(vt, e);
    for (u32 i = 0; i < 8; i++) {
        rsp.vu().set_accumulator_low(i, vt_element);
    }
    
    rsp.vu().write_element(vd, de, result & 0xFFFF);
    return 1;
}

u8 VRCPH(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 de = instr.v_type.vs & 7;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e & 7;

    s32 element_vt = static_cast<s32>(rsp.vu().read_element(vt, e));
    rsp.vu().set_div_in(element_vt << 16);
    rsp.vu().set_div_dp(true);

    for (u32 i = 0; i < 8; i++) {
        rsp.vu().set_accumulator_low(i, element_vt);
    }
    rsp.vu().write_element(vd, de, rsp.vu().get_div_out() >> 16);
    return 1;
}

u8 VMOV(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 de = instr.v_type.vs & 7;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e & 7;

    u16 vt_element = rsp.vu().read_element(vt, e);
    rsp.vu().write_element(vd, de, vt_element);
    
    for (u32 i = 0; i < 8; i++) {
        rsp.vu().set_accumulator_low(i, vt_element);
    }
    return 1;
}

u8 VRSQ(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e & 7;
    u32 de = instr.v_type.vs & 7;
    
    s32 input = static_cast<s16>(rsp.vu().read_element(vt, e));
    s32 data = (input < 0) ? -input : input;
    
    s32 result;
    if (data != 0) {
        // Count leading zeros for 16-bit value
        u32 shift = 0;
        while (!(data & (1 << 15)) && shift < 16) {
            data <<= 1;
            shift++;
        }
        
        // Index includes shift parity (even/odd affects sqrt)
        u32 index = ((data >> 6) & 0x1FE) | ((shift & 1) ^ 1);
        
        u32 sqrt_recip = rsp.vu().get_square_root(index);
        result = (0x10000 | sqrt_recip) << 14;
        result >>= ((31 - shift) >> 1);  // divide shift by 2 for sqrt
    } else {
        result = 0x7FFFFFFF;
    }
    
    if (input < 0) {
        result = ~result;
    }
    
    rsp.vu().set_div_out(result);
    
    u16 vt_element = rsp.vu().read_element(vt, e);
    for (u32 i = 0; i < 8; i++) {
        rsp.vu().set_accumulator_low(i, vt_element);
    }
    
    rsp.vu().write_element(vd, de, result & 0xFFFF);
    return 1;
}

u8 VRSQL(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e & 7;
    u32 de = instr.v_type.vs & 7;
    
    s32 input;
    if (rsp.vu().get_div_dp()) {
        input = rsp.vu().get_div_in() | rsp.vu().read_element(vt, e);
    } else {
        input = static_cast<s16>(rsp.vu().read_element(vt, e));
    }
    
    s32 data = (input < 0) ? -input : input;
    
    s32 result;
    if (data != 0) {
        // Count leading zeros for 32-bit value
        u32 shift = 0;
        while (!(data & (1 << 31)) && shift < 32) {
            data <<= 1;
            shift++;
        }
        
        // Index includes shift parity
        u32 index = ((data >> 22) & 0x1FE) | ((shift & 1) ^ 1);
        
        u32 sqrt_recip = rsp.vu().get_square_root(index);
        result = (0x10000 | sqrt_recip) << 14;
        result >>= ((31 - shift) >> 1);
    } else {
        result = 0x7FFFFFFF;
    }
    
    if (input < 0) {
        result = ~result;
    }
    
    rsp.vu().set_div_out(result);
    rsp.vu().set_div_dp(false);
    
    u16 vt_element = rsp.vu().read_element(vt, e);
    for (u32 i = 0; i < 8; i++) {
        rsp.vu().set_accumulator_low(i, vt_element);
    }
    
    rsp.vu().write_element(vd, de, result & 0xFFFF);
    return 1;
}

u8 VRSQH(RSP& rsp, const RSPInstruction& instr)
{
    u32 vt = instr.v_type.vt;
    u32 de = instr.v_type.vs & 7;
    u32 vd = instr.v_type.vd;
    u32 e = instr.v_type.e & 7;

    rsp.vu().set_div_in(rsp.vu().read_element(vt, e) << 16);
    rsp.vu().set_div_dp(true);
    
    u16 vt_element = rsp.vu().read_element(vt, e);
    for (u32 i = 0; i < 8; i++) {
        rsp.vu().set_accumulator_low(i, vt_element);
    }
    
    rsp.vu().write_element(vd, de, rsp.vu().get_div_out() >> 16);
    return 1;
}

u8 VNOP(RSP& rsp, const RSPInstruction& instr)
{
    // No operation
    return 1;
}

// LWC2 - Vector loads
u8 LBV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset;
    s32 address = base + offset;
    u8 value = rsp.read<u8>((address & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
    rsp.vu().write_element_byte(instr.l_type.vt, instr.l_type.element, value);
    return 1;
}

u8 LSV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 1;
    s32 address = base + offset;
    u32 element = instr.l_type.element;
    for (u32 i = 0; i < 2; i++) {
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
        rsp.vu().write_element_byte(instr.l_type.vt, (element + i) & 0x0F, value);
    }
    return 1;
}

u8 LLV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 2;
    u32 address = base + offset;
    u32 element = instr.l_type.element;
    for (u32 i = 0; i < 4; i++) {
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
        rsp.vu().write_element_byte(instr.l_type.vt, (element + i) & 0x0F, value);
    }
    return 1;
}

u8 LDV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 3;
    u32 address = base + offset;
    u32 element = instr.l_type.element;
    for (u32 i = 0; i < 8; i++) {
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
        rsp.vu().write_element_byte(instr.l_type.vt, (element + i) & 0x0F, value);
    }
    return 1;
}

u8 LQV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;
    u32 start_element = address & 0xF;
    for (u32 i = 0; i < 16 - start_element; i++) {
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
        rsp.vu().write_element_byte(instr.l_type.vt, (start_element + i) & 0x0F, value);
    }
    return 1;
}

u8 LRV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;
    u32 index = address & 0xF;

    u32 start_element = 16 - index;
    u32 aligned_addr = address & ~0xF;

    for (u32 i = 0; i < index; i++) {
        u8 value = rsp.read<u8>(((aligned_addr + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
        rsp.vu().write_element_byte(instr.l_type.vt, (start_element + i) & 0x0F, value);
    }
    return 1;
}

u8 LPV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 3;
    u32 address = base + offset;

    for (u32 i = 0; i < 8; i++) {
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
        u16 element_value = static_cast<u16>(value) << 8;
        rsp.vu().write_element(instr.l_type.vt, i, element_value);
    }
    return 1;
}

u8 LUV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 3;
    u32 address = base + offset;

    for (u32 i = 0; i < 8; i++) {
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
        u16 element_value = static_cast<u16>(value) << 7;
        rsp.vu().write_element(instr.l_type.vt, i, element_value);
    }
    return 1;
}

u8 LHV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;

    for (u32 i = 0; i < 8; i++) {
        u8 value = rsp.read<u8>(((address + i * 2) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
        u16 element_value = static_cast<u16>(value) << 7;
        rsp.vu().write_element(instr.l_type.vt, i, element_value);
    }
    return 1;
}

u8 LFV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;
    u32 start_element = instr.l_type.element;

    for (u32 i = 0; i < 4; i++) {
        u8 value = rsp.read<u8>(((address + i * 4) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS);
        u16 element_value = static_cast<u16>(value) << 7;
        rsp.vu().write_element(instr.l_type.vt, (start_element + i) & 0xF, element_value);
    }
    return 1;
}

u8 LTV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;
    u32 vt = instr.l_type.vt & 0x18;
    u32 start_element = instr.l_type.element >> 1;

    for (u32 i = 0; i < 8; i++) {
        u32 mem_addr = ((address + i * 2) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS;
        u8 hi = rsp.read<u8>(mem_addr);
        u8 lo = rsp.read<u8>(mem_addr + 1);
        u16 value = (hi << 8) | lo;
        u32 reg = vt | ((i + start_element) & 0x07);
        rsp.vu().write_element(reg, i, value);
    }

    return 1;
}

// SWC2 - Vector stores
u8 SBV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset;
    s32 address = base + offset;
    u8 value = rsp.vu().read_element_byte(instr.l_type.vt, instr.l_type.element);
    rsp.write<u8>((address & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    return 1;
}

u8 SSV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 1;
    s32 address = base + offset;
    u32 element = instr.l_type.element;
    for (u32 i = 0; i < 2; i++) {
        u8 value = rsp.vu().read_element_byte(instr.l_type.vt, (element + i) & 0x0F);
        rsp.write<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    }
    return 1;
}

u8 SLV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 2;
    s32 address = base + offset;
    u32 element = instr.l_type.element;
    for (u32 i = 0; i < 4; i++) {
        u8 value = rsp.vu().read_element_byte(instr.l_type.vt, (element + i) & 0x0F);
        rsp.write<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    }
    return 1;
}

u8 SDV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 3;
    s32 address = base + offset;
    u32 element = instr.l_type.element;
    for (u32 i = 0; i < 8; i++) {
        u8 value = rsp.vu().read_element_byte(instr.l_type.vt, (element + i) & 0x0F);
        rsp.write<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    }
    return 1;
}

u8 SQV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    s32 address = base + offset;
    u32 start_element = address & 0xF;
    for (u32 i = 0; i < 16 - start_element; i++) {
        u8 value = rsp.vu().read_element_byte(instr.l_type.vt, i);
        rsp.write<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    }
    return 1;
}

u8 SRV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;
    u32 index = address & 0xF;

    u32 start_element = 16 - index;
    u32 aligned_addr = address & ~0xF;

    for (u32 i = 0; i < index; i++) {
        u8 value = rsp.vu().read_element_byte(instr.l_type.vt, (start_element + i) & 0x0F);
        rsp.write<u8>(((aligned_addr + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    }

    return 1;
}

u8 SPV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 3;
    u32 address = base + offset;

    for (u32 i = 0; i < 8; i++) {
        u8 value = rsp.vu().read_element(instr.l_type.vt, i) >> 8;
        rsp.write<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value);
    }
    return 1;
}

u8 SUV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 3;
    u32 address = base + offset;

    for (u32 i = 0; i < 8; i++) {
        u16 value = rsp.vu().read_element(instr.l_type.vt, i);
        u8 element_value = (value >> 7) & 0x00FF;
        rsp.write<u8>(((address + i) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, element_value);
    }
    return 1;
}

u8 SHV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;

    for (u32 i = 0; i < 8; i++) {
        u16 value = rsp.vu().read_element(instr.l_type.vt, i);
        u8 element_value = (value >> 7) & 0x00FF;
        rsp.write<u8>(((address + i * 2) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, element_value);
    }
    return 1;
}

u8 SFV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;
    u32 start_element = instr.l_type.element >> 1;

    for (u32 i = 0; i < 4; i++) {
        u16 value = rsp.vu().read_element(instr.l_type.vt, (start_element + i) & 0x0F);
        u8 element_value = (value >> 7) & 0x00FF;
        rsp.write<u8>(((address + i * 4) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, element_value);
    }
    return 1;
}

u8 SWV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;
    u32 vt = instr.l_type.vt & 0x18;
    u32 start_element = instr.l_type.element >> 1;

    for (u32 i = 0; i < 8; i++) {
        u32 reg = vt | ((i + start_element) & 0x07);
        u32 elem = (i + start_element) & 0x07;
        u16 value = rsp.vu().read_element(reg, elem);
        rsp.write<u8>(((address + i * 2) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value >> 8);
        rsp.write<u8>(((address + i * 2 + 1) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value & 0x00FF);
    }

    return 1;
}

u8 STV(RSP& rsp, const RSPInstruction& instr)
{
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset << 4;
    u32 address = base + offset;
    u32 vt = instr.l_type.vt & 0x18;
    u32 start_element = instr.l_type.element >> 1;

    for (u32 i = 0; i < 8; i++) {
        u32 reg = vt | ((i + start_element) & 0x07);
        u16 value = rsp.vu().read_element(reg, i);
        rsp.write<u8>(((address + i * 2) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value >> 8);
        rsp.write<u8>(((address + i * 2 + 1) & 0x00000FFF) + n64::memory::RSP_DATA_MEMORY_START_ADDRESS, value & 0x00FF);
    }

    return 1;
}

} // namespace n64::rcp
