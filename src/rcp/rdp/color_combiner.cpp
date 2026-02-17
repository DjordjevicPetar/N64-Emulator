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
    prim_lod_fraction_ = get_bits(command, 47, 40);
    min_level_ = get_bits(command, 39, 32);
    primitive_color_.set_color_32b(get_bits(command, 31, 0), Format::FORMAT_RGB);
}

void ColorCombiner::set_environment_color(u64 command) {
    environment_color_.set_color_32b(get_bits(command, 31, 0), Format::FORMAT_RGB);
}

void ColorCombiner::set_fog_color(u64 command) {
    fog_color_.set_color_32b(get_bits(command, 31, 0), Format::FORMAT_RGB);
}

void ColorCombiner::set_blend_color(u64 command) {
    blend_color_.set_color_32b(get_bits(command, 31, 0), Format::FORMAT_RGB);
}

// (A - B) * C + D per channel
// C is 0.0-1.0 (255 = 1.0), so divide by 256 after multiply
Color ColorCombiner::combine(const Color& texel, const Color& shade) const {
    Color a = select_a(texel, shade);
    Color b = select_b(texel, shade);
    Color c = select_c(texel, shade);
    Color d = select_d(texel, shade);

    Color result;
    result.red   = std::clamp(((int)a.red   - (int)b.red)   * (int)c.red   / 256 + (int)d.red,   0, 255);
    result.green = std::clamp(((int)a.green - (int)b.green) * (int)c.green / 256 + (int)d.green, 0, 255);
    result.blue  = std::clamp(((int)a.blue  - (int)b.blue)  * (int)c.blue  / 256 + (int)d.blue,  0, 255);
    result.alpha = std::clamp(((int)a.alpha - (int)b.alpha) * (int)c.alpha / 256 + (int)d.alpha, 0, 255);

    return result;
}

// SubA RGB (4 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, 1.0, NOISE, 8+=0
// SubA Alpha (3 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, 1.0, 0
Color ColorCombiner::select_a(const Color& texel, const Color& shade) const {
    Color result;

    switch (a_input_.rgb1) {
        case 0:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: COMBINED
        case 1:  result.red = texel.red; result.green = texel.green; result.blue = texel.blue; break;
        case 2:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: TEXEL1
        case 3:  result.red = primitive_color_.red; result.green = primitive_color_.green; result.blue = primitive_color_.blue; break;
        case 4:  result.red = shade.red; result.green = shade.green; result.blue = shade.blue; break;
        case 5:  result.red = environment_color_.red; result.green = environment_color_.green; result.blue = environment_color_.blue; break;
        case 6:  result.red = 255; result.green = 255; result.blue = 255; break; // 1.0
        case 7:  { Color n = Color::randomize(); result.red = n.red; result.green = n.green; result.blue = n.blue; break; }
        default: result.red = 0; result.green = 0; result.blue = 0; break;
    }

    switch (a_input_.alpha1) {
        case 0: result.alpha = 0; break;                    // TODO: COMBINED
        case 1: result.alpha = texel.alpha; break;
        case 2: result.alpha = 0; break;                    // TODO: TEXEL1
        case 3: result.alpha = primitive_color_.alpha; break;
        case 4: result.alpha = shade.alpha; break;
        case 5: result.alpha = environment_color_.alpha; break;
        case 6: result.alpha = 255; break;                  // 1.0
        case 7: result.alpha = 0; break;
    }

    return result;
}

// SubB RGB (4 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, KEY_CENTER, K4, 8+=0
// SubB Alpha (3 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, 1.0, 0
Color ColorCombiner::select_b(const Color& texel, const Color& shade) const {
    Color result;

    switch (b_input_.rgb1) {
        case 0:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: COMBINED
        case 1:  result.red = texel.red; result.green = texel.green; result.blue = texel.blue; break;
        case 2:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: TEXEL1
        case 3:  result.red = primitive_color_.red; result.green = primitive_color_.green; result.blue = primitive_color_.blue; break;
        case 4:  result.red = shade.red; result.green = shade.green; result.blue = shade.blue; break;
        case 5:  result.red = environment_color_.red; result.green = environment_color_.green; result.blue = environment_color_.blue; break;
        case 6:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: KEY_CENTER
        case 7:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: K4
        default: result.red = 0; result.green = 0; result.blue = 0; break;
    }

    switch (b_input_.alpha1) {
        case 0: result.alpha = 0; break;                    // TODO: COMBINED
        case 1: result.alpha = texel.alpha; break;
        case 2: result.alpha = 0; break;                    // TODO: TEXEL1
        case 3: result.alpha = primitive_color_.alpha; break;
        case 4: result.alpha = shade.alpha; break;
        case 5: result.alpha = environment_color_.alpha; break;
        case 6: result.alpha = 255; break;                  // 1.0
        case 7: result.alpha = 0; break;
    }

    return result;
}

// Mul RGB (5 bits): COMBINED, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, KEY_SCALE,
//   COMBINED_A, TEXEL0_A, TEXEL1_A, PRIMITIVE_A, SHADE_A, ENV_A, LOD_FRAC, PRIM_LOD, K5, 16+=0
// Mul Alpha (3 bits): LOD_FRAC, TEXEL0, TEXEL1, PRIMITIVE, SHADE, ENVIRONMENT, PRIM_LOD, 0
Color ColorCombiner::select_c(const Color& texel, const Color& shade) const {
    Color result;

    switch (c_input_.rgb1) {
        case 0:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: COMBINED
        case 1:  result.red = texel.red; result.green = texel.green; result.blue = texel.blue; break;
        case 2:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: TEXEL1
        case 3:  result.red = primitive_color_.red; result.green = primitive_color_.green; result.blue = primitive_color_.blue; break;
        case 4:  result.red = shade.red; result.green = shade.green; result.blue = shade.blue; break;
        case 5:  result.red = environment_color_.red; result.green = environment_color_.green; result.blue = environment_color_.blue; break;
        case 6:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: KEY_SCALE
        case 7:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: COMBINED_ALPHA
        case 8:  result.red = texel.alpha; result.green = texel.alpha; result.blue = texel.alpha; break; // TEXEL0_ALPHA
        case 9:  result.red = 0; result.green = 0; result.blue = 0; break; // TODO: TEXEL1_ALPHA
        case 10: result.red = primitive_color_.alpha; result.green = primitive_color_.alpha; result.blue = primitive_color_.alpha; break;
        case 11: result.red = shade.alpha; result.green = shade.alpha; result.blue = shade.alpha; break;
        case 12: result.red = environment_color_.alpha; result.green = environment_color_.alpha; result.blue = environment_color_.alpha; break;
        case 13: result.red = 0; result.green = 0; result.blue = 0; break; // TODO: LOD_FRACTION
        case 14: result.red = prim_lod_fraction_; result.green = prim_lod_fraction_; result.blue = prim_lod_fraction_; break;
        case 15: result.red = 0; result.green = 0; result.blue = 0; break; // TODO: K5
        default: result.red = 0; result.green = 0; result.blue = 0; break;
    }

    switch (c_input_.alpha1) {
        case 0: result.alpha = 0; break;                    // TODO: LOD_FRACTION
        case 1: result.alpha = texel.alpha; break;
        case 2: result.alpha = 0; break;                    // TODO: TEXEL1
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
Color ColorCombiner::select_d(const Color& texel, const Color& shade) const {
    Color result;

    switch (d_input_.rgb1) {
        case 0: result.red = 0; result.green = 0; result.blue = 0; break; // TODO: COMBINED
        case 1: result.red = texel.red; result.green = texel.green; result.blue = texel.blue; break;
        case 2: result.red = 0; result.green = 0; result.blue = 0; break; // TODO: TEXEL1
        case 3: result.red = primitive_color_.red; result.green = primitive_color_.green; result.blue = primitive_color_.blue; break;
        case 4: result.red = shade.red; result.green = shade.green; result.blue = shade.blue; break;
        case 5: result.red = environment_color_.red; result.green = environment_color_.green; result.blue = environment_color_.blue; break;
        case 6: result.red = 255; result.green = 255; result.blue = 255; break; // 1.0
        case 7: result.red = 0; result.green = 0; result.blue = 0; break;
    }

    switch (d_input_.alpha1) {
        case 0: result.alpha = 0; break;                    // TODO: COMBINED
        case 1: result.alpha = texel.alpha; break;
        case 2: result.alpha = 0; break;                    // TODO: TEXEL1
        case 3: result.alpha = primitive_color_.alpha; break;
        case 4: result.alpha = shade.alpha; break;
        case 5: result.alpha = environment_color_.alpha; break;
        case 6: result.alpha = 255; break;                  // 1.0
        case 7: result.alpha = 0; break;
    }

    return result;
}

} // namespace n64::rdp
