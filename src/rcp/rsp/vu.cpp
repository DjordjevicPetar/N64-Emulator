#include "vu.hpp"
#include <stdexcept>
#include <string>

namespace n64::rcp {

VU::VU() : gpr_{}, accumulator_{0}, vcc_{0}, vco_{0}, vce_{0} {}

VU::~VU() = default;

u16 VU::read_element(u32 index, u32 element) const
{
    return gpr_[index].elements[element];
}

void VU::write_element(u32 index, u32 element, u16 value)
{
    gpr_[index].elements[element] = value;
}

u8 VU::read_element_byte(u32 index, u32 element) const
{
    if (element & 0x01) {
        return gpr_[index].elements[element >> 1] & 0x00FF;
    } else {
        return (gpr_[index].elements[element >> 1] >> 8) & 0x00FF;
    }
}

void VU::write_element_byte(u32 index, u32 element, u8 value)
{
    if (element & 0x01) {
        gpr_[index].elements[element >> 1] = (gpr_[index].elements[element >> 1] & 0xFF00) | value;
    } else {
        gpr_[index].elements[element >> 1] = (gpr_[index].elements[element >> 1] & 0x00FF) | (value << 8);
    }
}

u16 VU::read_control_register(u32 index) const
{
    switch (index) {
        case 0: return vco_;
        case 1: return vcc_;
        case 2: return vce_;
        default: throw std::runtime_error("Invalid VU control register index: " + std::to_string(index));
    }
}

void VU::write_control_register(u32 index, u16 value)
{
    switch (index) {
        case 0: vco_ = value; break;
        case 1: vcc_ = value; break;
        case 2: vce_ = value & 0x00FF; break;
        default: throw std::runtime_error("Invalid VU control register index: " + std::to_string(index));
    }
}

u16 VU::get_vt_element(u32 vt, u32 lane, u32 e) const
{
    VUElement element = gpr_[vt];
    if (e & 0x08) {
        return element.elements[e & 0x07];
    } else if (e & 0x04) {
        return element.elements[(lane & 0x04) | (e & 0x03)];
    } else if (e & 0x02) {
        return element.elements[(lane & 0x06) | (e & 0x01)];
    } else {
        return element.elements[lane & 0x07];
    }
}

void VU::init_reciprocal_table_()
{
    reciprocal_table_[0] = 0xFFFF;
    for (u32 i = 1; i < 512; i++) {
        u64 a = i + 512;
        u64 b = ((u64)1 << 34) / a;
        reciprocal_table_[i] = (u16)((b + 1) >> 8);
    }
}

void VU::init_square_root_table_()
{
    for (u32 i = 0; i < 512; i++) {
        u64 a = (i + 512) >> (i % 2 == 1 ? 1 : 0);
        u64 b = 1 << 17;
        while (a * (b + 1) * (b + 1) < ((u64)1 << 44)) b++;
        square_root_table_[i] = (u16)(b >> 1);
    }
}

} // namespace n64::rcp
