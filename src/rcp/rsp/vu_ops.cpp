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
    u32 base = rsp.su().read_gpr(instr.l_type.base);
    s32 offset = instr.l_type.offset;
    s32 address = base + offset;
    u8 value = rsp.read<u8>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u8 value = rsp.read<u8>(((aligned_addr + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u8 value = rsp.read<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u8 value = rsp.read<u8>(((address + i * 2) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u8 value = rsp.read<u8>(((address + i * 4) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS);
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
        u32 mem_addr = ((address + i * 2) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS;
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
    rsp.write<u8>((address & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
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
        rsp.write<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
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
        rsp.write<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
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
        rsp.write<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
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
        rsp.write<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
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
        rsp.write<u8>(((aligned_addr + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
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
        rsp.write<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value);
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
        rsp.write<u8>(((address + i) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, element_value);
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
        rsp.write<u8>(((address + i * 2) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, element_value);
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
        rsp.write<u8>(((address + i * 4) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, element_value);
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
        u32elem = (i + start_element) & 0x07;
        u16 value = rsp.vu().read_element(reg, elem);
        rsp.write<u8>(((address + i * 2) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value >> 8);
        rsp.write<u8>(((address + i * 2 + 1) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value & 0x00FF);
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
        rsp.write<u8>(((address + i * 2) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value >> 8);
        rsp.write<u8>(((address + i * 2 + 1) & 0x00000FFF) + memory::RSP_DATA_MEMORY_START_ADDRESS, value & 0x00FF);
    }

    return 1;
}

} // namespace n64::rcp
