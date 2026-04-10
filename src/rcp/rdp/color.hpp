#pragma once

#include "../../utils/types.hpp"
#include <algorithm>
#include <cstdlib>

namespace n64::rdp {

enum class Size : u8 {
    SIZE_4B = 0,
    SIZE_8B = 1,
    SIZE_16B = 2,
    SIZE_32B = 3,
};

enum class Format : u8 {
    FORMAT_RGB = 0,
    FORMAT_YUV = 1,
    FORMAT_CI = 2,
    FORMAT_IA = 3,
    FORMAT_I = 4,
};

class Color {
public:
    u8 red = 0;
    u8 green = 0;
    u8 blue = 0;
    u8 alpha = 0;

    Color() = default;
    Color(u8 r, u8 g, u8 b, u8 a) : red(r), green(g), blue(b), alpha(a) {}

    void set_color_32b(u32 color, Format format) {
        switch (format) {
            case Format::FORMAT_RGB:
                red = get_bits(color, 31, 24);
                green = get_bits(color, 23, 16);
                blue = get_bits(color, 15, 8);
                alpha = get_bits(color, 7, 0);
                break;
            default:
                return;
        }
    }

    void set_color_16b(u16 color, Format format) {
        switch (format) {
            case Format::FORMAT_RGB:
                red = get_bits(color, 15, 11) << 3;
                green = get_bits(color, 10, 6) << 3;
                blue = get_bits(color, 5, 1) << 3;
                alpha = (color & 1) ? 255 : 0;
                break;
            case Format::FORMAT_IA:
                red = blue = green = get_bits(color, 15, 8);
                alpha = get_bits(color, 7, 0);
                break;
            default:
                return;
        }
    }

    void set_color_8b(u8 color, Format format) {
        switch (format) {
            case Format::FORMAT_IA: {
                u8 intensity = (color >> 4) & 0x0F;
                red = green = blue = intensity | (intensity << 4);
                alpha = color & 0x0F;
                alpha = alpha | (alpha << 4);
                break;
            }
            case Format::FORMAT_I:
            default:
                red = green = blue = alpha = color;
                break;
        }
    }

    void set_color_4b(u8 color, Format format) {
        switch (format) {
            case Format::FORMAT_IA: {
                u8 i3 = (color >> 1) & 0x07;
                red = green = blue = (i3 << 5) | (i3 << 2) | (i3 >> 1);
                alpha = (color & 1) ? 255 : 0;
                break;
            }
            case Format::FORMAT_I:
            default:
                red = green = blue = alpha = color | (color << 4);
                break;
        }
    }

    static Color noise() {
        s32 val = ((std::rand() & 0x07) << 6) | 0x20;
        u8 clamped = static_cast<u8>(std::min(val, 255));
        return Color(clamped, clamped, clamped, clamped);
    }

    [[nodiscard]] u8 encode_4b() const {
        return 0;
    }

    [[nodiscard]] u8 encode_8b() const {
        return 0;
    }

    [[nodiscard]] u16 encode_16b() const {
        return ((red >> 3) << 11)   |
               ((green >> 3) << 6)  |
               ((blue >> 3) << 1)   |
               (alpha > 0 ? 1 : 0);
    }

    [[nodiscard]] u32 encode_32b() const {
        return (red << 24)   |
               (green << 16)  |
               (blue << 8)    |
               alpha;
    }
};

} // namespace n64::rdp
