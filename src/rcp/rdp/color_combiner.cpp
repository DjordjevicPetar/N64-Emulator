#include "color_combiner.hpp"

namespace n64::rdp {

void ColorCombiner::set_combine_mode(u64 command) {
    a_input_.rgb0 = get_bits(command, 55, 52);
    c_input_.rgb0 = get_bits(command, 51, 47);
    a_input_.alpha0 = get_bits(command, 46, 44);
    c_input_.alpha0 = get_bits(command, 43, 41);
    a_input_.rgb1 = get_bits(command, 40, 37);
    c_input_.rgb1 = get_bits(command, 36, 32);
    b_input_.rgb0 = get_bits(command, 31, 28);
    b_input_.rgb1 = get_bits(command, 27, 24);
    a_input_.alpha1 = get_bits(command, 23, 21);
    c_input_.alpha1 = get_bits(command, 20, 18);
    d_input_.rgb0 = get_bits(command, 17, 15);
    b_input_.alpha0 = get_bits(command, 14, 12);
    d_input_.alpha0 = get_bits(command, 11, 9);
    d_input_.rgb1 = get_bits(command, 8, 6);
    b_input_.alpha1 = get_bits(command, 5, 3);
    d_input_.alpha1 = get_bits(command, 2, 0);
}

void ColorCombiner::set_primitive_color(u64 command) {
    min_level_ = get_bits(command, 47, 40);
    prim_lod_fraction_ = get_bits(command, 39, 32);
    primitive_color_.set_color_32b(get_bits(command, 31, 0), Format::FORMAT_RGB);
}

void ColorCombiner::set_environment_color(u64 command) {
    environment_color_.set_color_32b(get_bits(command, 31, 0), Format::FORMAT_RGB);
}

void ColorCombiner::set_yuv_constants(u64 command) {
    k0_ = sign_extend9(get_bits(command, 53, 45));
    k1_ = sign_extend9(get_bits(command, 44, 36));
    k2_ = sign_extend9(get_bits(command, 35, 27));
    k3_ = sign_extend9(get_bits(command, 26, 18));
    k4_ = sign_extend9(get_bits(command, 17, 9));
    k5_ = sign_extend9(get_bits(command, 8, 0));
}

void ColorCombiner::set_key_gb(u64 command) {
    key_green_.width = FixedPointFloat(get_bits(command, 55, 52), get_bits(command, 51, 44), 4, 8, false);
    key_blue_.width = FixedPointFloat(get_bits(command, 43, 40), get_bits(command, 39, 32), 4, 8, false);
    key_green_.center = get_bits(command, 31, 24);
    key_blue_.center = get_bits(command, 23, 16);
    key_green_.scale = get_bits(command, 15, 8);
    key_blue_.scale = get_bits(command, 7, 0);
}

void ColorCombiner::set_key_r(u64 command) {
    key_red_.width = FixedPointFloat(get_bits(command, 27, 24), get_bits(command, 23, 16), 4, 8, false);
    key_red_.center = get_bits(command, 15, 8);
    key_red_.scale = get_bits(command, 7, 0);
}

Color ColorCombiner::convert_yuv(s32 Y, s32 U, s32 V) const {
    s32 u = U - 128;
    s32 v = V - 128;
    Color color;
    color.red   = static_cast<u8>(std::clamp(Y + ((k0_ * v) >> 7), 0, 255));
    color.green = static_cast<u8>(std::clamp(Y + ((k1_ * u + k2_ * v) >> 7), 0, 255));
    color.blue  = static_cast<u8>(std::clamp(Y + ((k3_ * u) >> 7), 0, 255));
    color.alpha = Y;
    return color;
}

// (A - B) * C + D per channel
// C is 0.0-1.0 (255 = 1.0), so divide by 256 after multiply
Color ColorCombiner::combine(const Color& texel0, const Color& texel1, const Color& shade,
                                const Color& combined_prev, u8 cycle) const {
    Color a = select_a(texel0, texel1, shade, combined_prev, cycle);
    Color b = select_b(texel0, texel1, shade, combined_prev, cycle);
    Color c = select_c(texel0, texel1, shade, combined_prev, cycle);
    Color d = select_d(texel0, texel1, shade, combined_prev, cycle);

    Color result;
    result.red   = std::clamp(((int)a.red   - (int)b.red)   * (int)c.red   / 256 + (int)d.red,   0, 255);
    result.green = std::clamp(((int)a.green - (int)b.green) * (int)c.green / 256 + (int)d.green, 0, 255);
    result.blue  = std::clamp(((int)a.blue  - (int)b.blue)  * (int)c.blue  / 256 + (int)d.blue,  0, 255);
    result.alpha = std::clamp(((int)a.alpha - (int)b.alpha) * (int)c.alpha / 256 + (int)d.alpha, 0, 255);

    if (key_enable_ && cycle == 1) {
        // TODO: Should get double checked
        s32 kr = (key_red_.width.raw() >> 12) - result.red;
        s32 kg = (key_green_.width.raw() >> 12) - result.green;
        s32 kb = (key_blue_.width.raw() >> 12) - result.blue;

        result = a;
        result.alpha = std::clamp(std::min({kr, kg, kb}), 0, 255);
    }

    return result;
}

// SubA RGB (4 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, 1.0, NOISE, 8+=0
// SubA Alpha (3 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, 1.0, 0
Color ColorCombiner::select_a(const Color& texel0, const Color& texel1, const Color& shade,
                                const Color& combined_prev, u8 cycle) const {
    Color result;
    u8 sel = (cycle == 0) ? a_input_.rgb0 : a_input_.rgb1;

    switch (sel) {
        case 0:  result.red = combined_prev.red; result.green = combined_prev.green; result.blue = combined_prev.blue; break;
        case 1:  result.red = texel0.red; result.green = texel0.green; result.blue = texel0.blue; break;
        case 2:  result.red = texel1.red; result.green = texel1.green; result.blue = texel1.blue; break;
        case 3:  result.red = primitive_color_.red; result.green = primitive_color_.green; result.blue = primitive_color_.blue; break;
        case 4:  result.red = shade.red; result.green = shade.green; result.blue = shade.blue; break;
        case 5:  result.red = environment_color_.red; result.green = environment_color_.green; result.blue = environment_color_.blue; break;
        case 6:  result.red = 255; result.green = 255; result.blue = 255; break;
        case 7:  { Color n = Color::noise(); result.red = n.red; result.green = n.green; result.blue = n.blue; break; }
        default: result.red = 0; result.green = 0; result.blue = 0; break;
    }

    sel = (cycle == 0) ? a_input_.alpha0 : a_input_.alpha1;
    switch (sel) {
        case 0: result.alpha = combined_prev.alpha; break;
        case 1: result.alpha = texel0.alpha; break;
        case 2: result.alpha = texel1.alpha; break;
        case 3: result.alpha = primitive_color_.alpha; break;
        case 4: result.alpha = shade.alpha; break;
        case 5: result.alpha = environment_color_.alpha; break;
        case 6: result.alpha = 255; break;
        case 7: result.alpha = 0; break;
    }

    return result;
}

// SubB RGB (4 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, KEY_CENTER, K4, 8+=0
// SubB Alpha (3 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, 1.0, 0
Color ColorCombiner::select_b(const Color& texel0, const Color& texel1, const Color& shade, const Color& combined_prev, u8 cycle) const {
    Color result;
    u8 sel = (cycle == 0) ? b_input_.rgb0 : b_input_.rgb1;

    switch (sel) {
        case 0:  result.red = combined_prev.red; result.green = combined_prev.green; result.blue = combined_prev.blue; break;
        case 1:  result.red = texel0.red; result.green = texel0.green; result.blue = texel0.blue; break;
        case 2:  result.red = texel1.red; result.green = texel1.green; result.blue = texel1.blue; break;
        case 3:  result.red = primitive_color_.red; result.green = primitive_color_.green; result.blue = primitive_color_.blue; break;
        case 4:  result.red = shade.red; result.green = shade.green; result.blue = shade.blue; break;
        case 5:  result.red = environment_color_.red; result.green = environment_color_.green; result.blue = environment_color_.blue; break;
        case 6:  result.red = key_red_.center; result.green = key_green_.center; result.blue = key_blue_.center; break;
        case 7:  result.red = k4_; result.green = k4_; result.blue = k4_; break;
        default: result.red = 0; result.green = 0; result.blue = 0; break;
    }

    sel = (cycle == 0) ? b_input_.alpha0 : b_input_.alpha1;
    switch (sel) {
        case 0: result.alpha = combined_prev.alpha; break;
        case 1: result.alpha = texel0.alpha; break;
        case 2: result.alpha = texel1.alpha; break;
        case 3: result.alpha = primitive_color_.alpha; break;
        case 4: result.alpha = shade.alpha; break;
        case 5: result.alpha = environment_color_.alpha; break;
        case 6: result.alpha = 255; break;
        case 7: result.alpha = 0; break;
    }

    return result;
}

// Mul RGB (5 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, KEY_SCALE,
//   COMBINED_A, TEXEL0_A, TEXEL1_A, PRIMITIVE_A, SHADE_A, ENV_A, LOD_FRAC, PRIM_LOD, K5, 16+=0
// Mul Alpha (3 bits): LOD_FRAC, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, PRIM_LOD, 0
Color ColorCombiner::select_c(const Color& texel0, const Color& texel1, const Color& shade, const Color& combined_prev, u8 cycle) const {
    Color result;
    u8 sel = (cycle == 0) ? c_input_.rgb0 : c_input_.rgb1;

    switch (sel) {
        case 0:  result.red = combined_prev.red; result.green = combined_prev.green; result.blue = combined_prev.blue; break;
        case 1:  result.red = texel0.red; result.green = texel0.green; result.blue = texel0.blue; break;
        case 2:  result.red = texel1.red; result.green = texel1.green; result.blue = texel1.blue; break;
        case 3:  result.red = primitive_color_.red; result.green = primitive_color_.green; result.blue = primitive_color_.blue; break;
        case 4:  result.red = shade.red; result.green = shade.green; result.blue = shade.blue; break;
        case 5:  result.red = environment_color_.red; result.green = environment_color_.green; result.blue = environment_color_.blue; break;
        case 6:  result.red = key_red_.scale; result.green = key_green_.scale; result.blue = key_blue_.scale; break;
        case 7:  result.red = combined_prev.alpha; result.green = combined_prev.alpha; result.blue = combined_prev.alpha; break;
        case 8:  result.red = texel0.alpha; result.green = texel0.alpha; result.blue = texel0.alpha; break;
        case 9:  result.red = texel1.alpha; result.green = texel1.alpha; result.blue = texel1.alpha; break;
        case 10: result.red = primitive_color_.alpha; result.green = primitive_color_.alpha; result.blue = primitive_color_.alpha; break;
        case 11: result.red = shade.alpha; result.green = shade.alpha; result.blue = shade.alpha; break;
        case 12: result.red = environment_color_.alpha; result.green = environment_color_.alpha; result.blue = environment_color_.alpha; break;
        case 13: result.red = lod_fraction_; result.green = lod_fraction_; result.blue = lod_fraction_; break;
        case 14: result.red = prim_lod_fraction_; result.green = prim_lod_fraction_; result.blue = prim_lod_fraction_; break;
        case 15: result.red = k5_; result.green = k5_; result.blue = k5_; break;
        default: result.red = 0; result.green = 0; result.blue = 0; break;
    }

    sel = (cycle == 0) ? c_input_.alpha0 : c_input_.alpha1;
    switch (sel) {
        case 0: result.alpha = lod_fraction_; break;
        case 1: result.alpha = texel0.alpha; break;
        case 2: result.alpha = texel1.alpha; break;
        case 3: result.alpha = primitive_color_.alpha; break;
        case 4: result.alpha = shade.alpha; break;
        case 5: result.alpha = environment_color_.alpha; break;
        case 6: result.alpha = prim_lod_fraction_; break;
        case 7: result.alpha = 0; break;
    }

    return result;
}

// Add RGB (3 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, 1.0, 0
// Add Alpha (3 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, 1.0, 0
Color ColorCombiner::select_d(const Color& texel0, const Color& texel1, const Color& shade, const Color& combined_prev, u8 cycle) const {
    Color result;
    u8 sel = (cycle == 0) ? d_input_.rgb0 : d_input_.rgb1;

    switch (sel) {
        case 0: result.red = combined_prev.red; result.green = combined_prev.green; result.blue = combined_prev.blue; break;
        case 1: result.red = texel0.red; result.green = texel0.green; result.blue = texel0.blue; break;
        case 2: result.red = texel1.red; result.green = texel1.green; result.blue = texel1.blue; break;
        case 3: result.red = primitive_color_.red; result.green = primitive_color_.green; result.blue = primitive_color_.blue; break;
        case 4: result.red = shade.red; result.green = shade.green; result.blue = shade.blue; break;
        case 5: result.red = environment_color_.red; result.green = environment_color_.green; result.blue = environment_color_.blue; break;
        case 6: result.red = 255; result.green = 255; result.blue = 255; break; // 1.0
        case 7: result.red = 0; result.green = 0; result.blue = 0; break;
    }

    sel = (cycle == 0) ? d_input_.alpha0 : d_input_.alpha1;
    switch (sel) {
        case 0: result.alpha = combined_prev.alpha; break;
        case 1: result.alpha = texel0.alpha; break;
        case 2: result.alpha = texel1.alpha; break;
        case 3: result.alpha = primitive_color_.alpha; break;
        case 4: result.alpha = shade.alpha; break;
        case 5: result.alpha = environment_color_.alpha; break;
        case 6: result.alpha = 255; break;                  // 1.0
        case 7: result.alpha = 0; break;
    }

    return result;
}

} // namespace n64::rdp
