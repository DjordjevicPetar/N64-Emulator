#pragma once

#include "../../utils/types.hpp"
#include <array>

namespace n64::rcp {

enum VUControlRegister {
    VU_CONTROL_REGISTER_VCO = 0,
    VU_CONTROL_REGISTER_VCC = 1,
    VU_CONTROL_REGISTER_VCE = 2,
};

struct VUElement {
    u16 elements[8];
};

struct VUAccumulator {
    s64 lanes[8];

    s16 high(u32 index) const { return (lanes[index] >> 32) & 0xFFFF; }
    s16 mid(u32 index) const { return (lanes[index] >> 16) & 0xFFFF; }
    s16 low(u32 index) const { return lanes[index] & 0xFFFF; }
    void set_high(u32 index, s16 value) { 
        lanes[index] = (lanes[index] & 0x0000FFFFFFFFLL) | ((s64)(value & 0xFFFF) << 32); 
    }
    void set_mid(u32 index, s16 value) { 
        lanes[index] = (lanes[index] & 0xFFFF0000FFFFLL) | ((s64)(value & 0xFFFF) << 16); 
    }
    void set_low(u32 index, s16 value) { 
        lanes[index] = (lanes[index] & 0xFFFFFFFF0000LL) | (value & 0xFFFF); 
    }
};

class VU {
public:
    VU();
    ~VU();

    u16 read_element(u32 index, u32 element) const;
    void write_element(u32 index, u32 element, u16 value);

    u16 read_control_register(u32 index) const;
    void write_control_register(u32 index, u16 value);

    s16 get_accumulator_low(u32 index) { return accumulator_.low(index); };
    s16 get_accumulator_mid(u32 index) { return accumulator_.mid(index); };
    s16 get_accumulator_high(u32 index) { return accumulator_.high(index); };
    void set_accumulator_low(u32 index, s16 value) { accumulator_.set_low(index, value); };
    void set_accumulator_mid(u32 index, s16 value) { accumulator_.set_mid(index, value); };
    void set_accumulator_high(u32 index, s16 value) { accumulator_.set_high(index, value); };


    u16 get_vt_element(u32 vt, u32 lane, u32 e) const;

private:
    std::array<VUElement, 32> gpr_;
    VUAccumulator accumulator_;
    u16 vcc_;
    u16 vco_;
    u8 vce_;
};

}