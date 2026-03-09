#pragma once

#include "color.hpp"
#include "../../utils/types.hpp"

namespace n64::rdp {

struct BlenderInput {
    u8 bl_m1_0;
    u8 bl_m1_1;
};

class Blender {
public:

    // Inputs are color that has been processed by the color combiner and the already standing color in the framebuffer
    [[nodiscard]] Color blend(const Color& combined_color, const Color& framebuffer_color) const;

    void set_blender_input(u16 command_input);
    void set_fog_color(u64 command);
    void set_blend_color(u64 command);
    void set_force_blend(bool force_blend_mode) { force_blend_ = force_blend_mode; };

private:

    [[nodiscard]] Color select_p(const Color& combined_color, const Color& framebuffer_color) const;
    [[nodiscard]] f32 select_a(const Color& combined_color, const Color& framebuffer_color) const;
    [[nodiscard]] Color select_m(const Color& combined_color, const Color& framebuffer_color) const;
    [[nodiscard]] f32 select_b(const Color& combined_color, const Color& framebuffer_color, const f32& a) const;

    BlenderInput input_p_;
    BlenderInput input_a_;
    BlenderInput input_m_;
    BlenderInput input_b_;
    bool force_blend_ = false;

    Color fog_color_;
    Color blend_color_;
};

}