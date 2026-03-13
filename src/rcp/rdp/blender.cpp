#include "blender.hpp"

namespace n64::rdp {

    Color Blender::blend(const Color& combined_color, const Color& framebuffer_color, const Color& shade, u8 cvg, u8 cycle) const {

        Color p = select_p(combined_color, framebuffer_color, cycle);

        if (!force_blend_) {
            return p;
        }

        int a = select_a_5bit(combined_color, shade, cycle);
        Color m = select_m(combined_color, framebuffer_color, cycle);
        int b = select_b_5bit(a, cvg, cycle);

        Color result;
        result.red   = std::clamp((p.red   * a + m.red   * b) >> 5, 0, 255);
        result.green = std::clamp((p.green * a + m.green * b) >> 5, 0, 255);
        result.blue  = std::clamp((p.blue  * a + m.blue  * b) >> 5, 0, 255);
        result.alpha = std::clamp((p.alpha * a + m.alpha * b) >> 5, 0, 255);
        return result;
    }

    u8 Blender::get_alpha_threshold(bool dither_alpha_enable) const {
        return dither_alpha_enable ? (rand() & 0xFF) : blend_color_.alpha;
    }

    void Blender::set_blender_input(u16 command_input) {
        input_p_.bl_m1_0 = get_bits(command_input, 15, 14);
        input_p_.bl_m1_1 = get_bits(command_input, 13, 12);
        input_a_.bl_m1_0 = get_bits(command_input, 11, 10);
        input_a_.bl_m1_1 = get_bits(command_input, 9, 8);
        input_m_.bl_m1_0 = get_bits(command_input, 7, 6);
        input_m_.bl_m1_1 = get_bits(command_input, 5, 4);
        input_b_.bl_m1_0 = get_bits(command_input, 3, 2);
        input_b_.bl_m1_1 = get_bits(command_input, 1, 0);
    }

    void Blender::set_fog_color(u64 command) {
        fog_color_.set_color_32b(get_bits(command, 31, 0), Format::FORMAT_RGB);
    }
    
    void Blender::set_blend_color(u64 command) {
        blend_color_.set_color_32b(get_bits(command, 31, 0), Format::FORMAT_RGB);
    }

    Color Blender::select_p(const Color& combined_color, const Color& framebuffer_color, u8 cycle) const {
        Color result;
        u8 sel = (cycle == 0) ? input_p_.bl_m1_0 : input_p_.bl_m1_1;
        switch (sel) {
            case 0: result = combined_color; break;
            case 1: result = framebuffer_color; break;
            case 2: result = blend_color_; break;
            case 3: result = fog_color_; break;
        }
        return result;
    }

    int Blender::select_a_5bit(const Color& combined_color, const Color& shade, u8 cycle) const {
        u8 sel = (cycle == 0) ? input_a_.bl_m1_0 : input_a_.bl_m1_1;
        switch (sel) {
            case 0: return combined_color.alpha >> 3;
            case 1: return fog_color_.alpha >> 3;
            case 2: return shade.alpha >> 3;
            case 3: return 0;
        }
        return 0;
    }

    Color Blender::select_m(const Color& combined_color, const Color& framebuffer_color, u8 cycle) const {
        Color result;
        u8 sel = (cycle == 0) ? input_m_.bl_m1_0 : input_m_.bl_m1_1;
        switch (sel) {
            case 0: result = combined_color; break;
            case 1: result = framebuffer_color; break;
            case 2: result = blend_color_; break;
            case 3: result = fog_color_; break;
        }
        return result;
    }

    int Blender::select_b_5bit(int a, u8 cvg, u8 cycle) const {
        u8 sel = (cycle == 0) ? input_b_.bl_m1_0 : input_b_.bl_m1_1;
        switch (sel) {
            case 0: return (~a) & 0x1F;
            case 1: return cvg;
            case 2: return 0x1F;
            case 3: return 0;
        }
        return 0;
    }
}
