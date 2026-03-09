#include "rdp.hpp"
#include "../../memory/rdram.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace n64::rdp {

float RDP::bytes_per_pixel(Size size) const {
    switch (size) {
        case Size::SIZE_4B: return 0.5f;
        case Size::SIZE_8B: return 1.0f;
        case Size::SIZE_16B: return 2.0f;
        case Size::SIZE_32B: return 4.0f;
        default: throw std::runtime_error("Invalid size: " + std::to_string(static_cast<u8>(size)));
    }
}

template<typename T>
T RDP::read_tmem(u32 addr) const {
    T result = 0;
    for (u8 i = 0; i < sizeof(T); i++) {
        result = (result << 8) | static_cast<T>(tmem_[(addr + i) % 4096]);
    }
    return result;
}

Color RDP::fetch_pixel_tmem(u32 addr, Size size, Format format, bool odd_texel, u8 palette) const {
    Color color;
    u32 pixel;

    switch (size) {
        case Size::SIZE_4B: {
            pixel = read_tmem<u8>(addr);

            if (odd_texel) {
                pixel &= 0x0F;
            } else {
                pixel >>= 4;
            }

            if (format == Format::FORMAT_CI) {
                u32 tmem_addr = TLUT_BASE_ADDRESS + (palette * 16 + pixel) * 8;
                return fetch_pixel_tmem(tmem_addr, Size::SIZE_16B, tlut_type_, false);
            }

            color.set_color_4b(pixel, format);
            break;
        }
        case Size::SIZE_8B: {
            pixel = read_tmem<u8>(addr);
            if (format == Format::FORMAT_CI) {
                u32 tmem_addr = TLUT_BASE_ADDRESS + pixel * 8;
                return fetch_pixel_tmem(tmem_addr, Size::SIZE_16B, tlut_type_, false);
            }
            color.set_color_8b(pixel, format);
            break;
        }
        case Size::SIZE_16B: {
            if (format == Format::FORMAT_YUV) {
                u32 pair_base = addr & ~3;
                s32 Y = read_tmem<u8>(addr + 1);
                s32 U = read_tmem<u8>(pair_base);
                s32 V = read_tmem<u8>(pair_base + 2);
                return color_combiner_.convert_yuv(Y, U, V);
            }
            pixel = read_tmem<u16>(addr);
            color.set_color_16b(pixel, format);
            break;
        }
        case Size::SIZE_32B: {
            pixel = read_tmem<u32>(addr);
            if (format == Format::FORMAT_CI) {
                format = tlut_type_;
            }
            color.set_color_32b(pixel, format);
            break;
        }
        default:
            throw std::runtime_error("Invalid texture size: " + std::to_string(static_cast<u8>(texture_image_.size)));
    }
    return color;
}

void RDP::write_pixel_framebuffer(u32 addr, const Color& color) {
    switch (color_image_.size) {
        case Size::SIZE_4B: {
            // TODO: Implement 4b pixel writing
            break;
        }
        case Size::SIZE_8B: {
            // TODO: Implement 8b pixel writing
            break;
        }
        case Size::SIZE_16B: {
            rdram_.write_memory<u16>(addr, color.encode_16b() | 1);
            break;
        }
        case Size::SIZE_32B: {
            rdram_.write_memory<u32>(addr, color.encode_32b() | 0xFF);
            break;
        }
    default:
        throw std::runtime_error("Invalid color image size: " + std::to_string(static_cast<u8>(color_image_.size)));
    }
}

Color RDP::read_pixel_framebuffer(u32 addr) const {
    Color color;
    switch (color_image_.size) {
        case Size::SIZE_16B: {
            u16 raw = rdram_.read_memory<u16>(addr);
            color.set_color_16b(raw, Format::FORMAT_RGB);
            break;
        }
        case Size::SIZE_32B: {
            u32 raw = rdram_.read_memory<u32>(addr);
            color.set_color_32b(raw, Format::FORMAT_RGB);
            break;
        }
        default:
            break;
    }
    return color;
}

bool RDP::is_pixel_transparent(const Color& color) const {
    // In 1-cycle mode, always skip transparent texels (blender approximation)
    if (cycle_type_ == 0) {
        return color.alpha == 0;
    }
    // In copy mode, only skip if alpha compare is enabled
    if (!alpha_compare_enable_) return false;
    return color.alpha == 0;
}

void RDP::process_tmem_coordinates(s32& coord, u8 shift, u8 mask, bool mirror, bool clamp, u16 lower_limit, u16 upper_limit) const {
    // 1. Shift
    if (shift <= 10)
        coord >>= shift;
    else
        coord <<= (16 - shift);

    if (clamp) {
        s32 clamped_coord = std::clamp(coord, static_cast<s32>(lower_limit), static_cast<s32>(upper_limit));
        if (coord != clamped_coord) {
            coord = clamped_coord;
            return;
        }
    }

    // 2. Mask + Mirror
    if (mask == 0) return;

    if (mirror && (coord & (1 << mask))) {
        coord = ((1 << mask) - 1) - (coord & ((1 << mask) - 1));
    } else {
        coord &= (1 << mask) - 1;
    }
}

u32 RDP::load_tlut(u64 command) {
    u16 sl = get_bits(command, 55, 44) >> 2;
    u16 sh = get_bits(command, 23, 12) >> 2;

    for (u16 index = sl; index <= sh; index++) {
        u32 rdram_addr = texture_image_.addr + index * 2;
        u32 tmem_addr = TLUT_BASE_ADDRESS + index * 8;
        u16 color = rdram_.read_memory<u16>(rdram_addr);
        tmem_[(tmem_addr + 0) % 4096] = (color >> 8) & 0xFF;
        tmem_[(tmem_addr + 1) % 4096] = color & 0xFF;
    }
    return std::max<u32>(sh - sl + 1, 8);
}

// TODO: Implement DXT row-swapping (odd rows byte-swap pairs for dual-bank TMEM interleaving)
u32 RDP::load_block(u64 command) {
    u8 tile_index = get_bits(command, 26, 25);
    u16 number_of_texels_to_load = get_bits(command, 23, 12) + 1;

    u32 bpp = bytes_per_pixel(texture_image_.size);
    u32 total_bytes = number_of_texels_to_load * bpp;

    total_bytes = std::min(total_bytes, static_cast<u32>(4096 - tiles_[tile_index].address));

    u32 tmem_addr = tiles_[tile_index].address;
    for (u16 i = 0; i < total_bytes; i++) {
        tmem_[tmem_addr + i] = rdram_.read_memory<u8>(texture_image_.addr + i);
    }
    return std::max(total_bytes, 8u);
}

u32 RDP::load_tile(u64 command) {
    u16 upper_left_s = get_bits(command, 55, 44) >> 2;
    u16 upper_left_t = get_bits(command, 43, 32) >> 2;
    u8 tile_index = get_bits(command, 26, 24);
    u16 lower_right_s = get_bits(command, 23, 12) >> 2;
    u16 lower_right_t = get_bits(command, 11, 0) >> 2;

    u32 bpp = bytes_per_pixel(texture_image_.size);
    u32 tmem_line_stride = tiles_[tile_index].line_bytes;

    u32 total_bytes = 0;
    for (u16 t = upper_left_t; t <= lower_right_t; t++) {
        u32 row_tmem_base = tiles_[tile_index].address + (t - upper_left_t) * tmem_line_stride;
        for (u16 s = upper_left_s; s <= lower_right_s; s++) {
            u32 rdram_addr = texture_image_.addr + (t * texture_image_.width + s) * bpp;
            u32 tmem_addr = row_tmem_base + (s - upper_left_s) * bpp;
            for (u16 i = 0; i < bpp; i++) {
                tmem_[(tmem_addr + i) % 4096] = rdram_.read_memory<u8>(rdram_addr + i);
            }
            total_bytes += bpp;
        }
    }
    return std::max(total_bytes, 8u);
}

template u8 RDP::read_tmem<u8>(u32) const;
template u16 RDP::read_tmem<u16>(u32) const;
template u32 RDP::read_tmem<u32>(u32) const;

} // namespace n64::rdp
