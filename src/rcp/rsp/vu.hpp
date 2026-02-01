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
    u8 read_element_byte(u32 index, u32 element) const;
    void write_element_byte(u32 index, u32 element, u8 value);

    u16 read_control_register(u32 index) const;
    void write_control_register(u32 index, u16 value);

    s16 get_accumulator_low(u32 index) { return accumulator_.low(index); };
    s16 get_accumulator_mid(u32 index) { return accumulator_.mid(index); };
    s16 get_accumulator_high(u32 index) { return accumulator_.high(index); };
    void set_accumulator_low(u32 index, s16 value) { accumulator_.set_low(index, value); };
    void set_accumulator_mid(u32 index, s16 value) { accumulator_.set_mid(index, value); };
    void set_accumulator_high(u32 index, s16 value) { accumulator_.set_high(index, value); };

    u16 get_vt_element(u32 vt, u32 lane, u32 e) const;
    u16 get_reciprocal(u32 index) const { return reciprocal_table_[index]; };
    u16 get_square_root(u32 index) const { return square_root_table_[index]; };

    s32 get_div_in() const { return div_in_; }
    void set_div_in(s32 value) { div_in_ = value; }
    s32 get_div_out() const { return div_out_; }
    void set_div_out(s32 value) { div_out_ = value; }
    bool get_div_dp() const { return div_dp_; }
    void set_div_dp(bool value) { div_dp_ = value; }

private:

    void init_reciprocal_table_();
    void init_square_root_table_();

    std::array<VUElement, 32> gpr_;
    VUAccumulator accumulator_;
    u16 vcc_;
    u16 vco_;
    u8 vce_;

    std::array<u16, 512> reciprocal_table_;
    std::array<u16, 512> square_root_table_;

    s32 div_in_ = 0;   // Input for VRCPL/VRSQL (high part from VRCPH/VRSQH)
    s32 div_out_ = 0;  // Output result
    bool div_dp_ = false;  // Double-precision flag
};

}