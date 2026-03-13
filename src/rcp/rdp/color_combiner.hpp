#pragma once

#include "color.hpp"
#include "../../utils/types.hpp"
#include "fixed_point_float.hpp"
#include <algorithm>
#include <iostream>

namespace n64::rdp {

struct ColorCombinerInput {
    u8 rgb0;
    u8 alpha0;
    u8 rgb1;
    u8 alpha1;
};

struct KeyParams {
    FixedPointFloat width;
    u8 center;
    u8 scale;
};

class ColorCombiner {
public:
    void set_combine_mode(u64 command);
    void set_primitive_color(u64 command);
    void set_environment_color(u64 command);
    void set_yuv_constants(u64 command);
    void set_key_gb(u64 command);
    void set_key_r(u64 command);
    void set_key_enable(bool enable) { key_enable_ = enable; }

    [[nodiscard]] Color combine(const Color& texel0, const Color& texel1, const Color& shade,
                                const Color& combined_prev, u8 cycle) const;
    
    [[nodiscard]] Color convert_yuv(s32 Y, s32 U, s32 V) const;

private:
    [[nodiscard]] Color select_a(const Color& texel0, const Color& texel1, const Color& shade, const Color& combined_prev, u8 cycle) const;
    [[nodiscard]] Color select_b(const Color& texel0, const Color& texel1, const Color& shade, const Color& combined_prev, u8 cycle) const;
    [[nodiscard]] Color select_c(const Color& texel0, const Color& texel1, const Color& shade, const Color& combined_prev, u8 cycle) const;
    [[nodiscard]] Color select_d(const Color& texel0, const Color& texel1, const Color& shade, const Color& combined_prev, u8 cycle) const;

    ColorCombinerInput a_input_;
    ColorCombinerInput b_input_;
    ColorCombinerInput c_input_;
    ColorCombinerInput d_input_;

    // Chroma key parameters
    KeyParams key_red_;
    KeyParams key_green_;
    KeyParams key_blue_;

    bool key_enable_ = false;

    Color primitive_color_;
    Color environment_color_;
    u8 prim_lod_fraction_ = 0;
    u8 lod_fraction_ = 0xFF;
    u8 min_level_ = 0;

    // YUV convert constants (9-bit signed from Set_Convert)
    s16 k0_ = 0;
    s16 k1_ = 0;
    s16 k2_ = 0;
    s16 k3_ = 0;
    s16 k4_ = 0;
    s16 k5_ = 0;
};

} // namespace n64::rdp
