#include "blender.hpp"

namespace n64::rdp {

    Color Blender::blend(const Color& combined_color, const Color& framebuffer_color) const {

        Color p = select_p(combined_color, framebuffer_color);
        f32 a = select_a(combined_color, framebuffer_color);
        Color m = select_m(combined_color, framebuffer_color);
        f32 b = select_b(combined_color, framebuffer_color, a);

        if (!force_blend_) {
            return p;
        }

        // TODO: Implement anti-aliasing
        Color result;
        result.red = std::clamp(p.red * a + m.red * b, 0.0f, 255.0f);
        result.green = std::clamp(p.green * a + m.green * b, 0.0f, 255.0f);
        result.blue = std::clamp(p.blue * a + m.blue * b, 0.0f, 255.0f);
        result.alpha = std::clamp(p.alpha * a + m.alpha * b, 0.0f, 255.0f);
        return result;
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

    Color Blender::select_p(const Color& combined_color, const Color& framebuffer_color) const {
        Color result;
        switch (input_p_.bl_m1_0) {
            case 0: result = combined_color; break;
            case 1: result = framebuffer_color; break;
            case 2: result = blend_color_; break;
            case 3: result = fog_color_; break;
        }
        return result;
    }

    f32 Blender::select_a(const Color& combined_color, const Color& framebuffer_color) const {
        f32 result;
        switch (input_a_.bl_m1_0) {
            case 0: result = combined_color.alpha / 255.0f; break;
            case 1: result = fog_color_.alpha / 255.0f; break;
            // TODO: Shade alpha
            case 2: result = 0.0f; break;
            case 3: result = 0.0f; break;
        }
        return result;
    }

    Color Blender::select_m(const Color& combined_color, const Color& framebuffer_color) const {
        Color result;
        switch (input_m_.bl_m1_0) {
            case 0: result = combined_color; break;
            case 1: result = framebuffer_color; break;
            case 2: result = blend_color_; break;
            case 3: result = fog_color_; break;
        }
        return result;
    }

    f32 Blender::select_b(const Color& combined_color, const Color& framebuffer_color, const f32& a) const {
        f32 result;
        switch (input_b_.bl_m1_0) {
            case 0: result = 1.0f - a; break;
            case 1: result = framebuffer_color.alpha / 255.0f; break;
            case 2: result = 1.0f; break;
            case 3: result = 0.0f; break;
        }
        return result;
    }
}
