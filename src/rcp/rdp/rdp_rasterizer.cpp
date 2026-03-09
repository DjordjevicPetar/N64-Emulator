#include "rdp.hpp"
#include "../../memory/rdram.hpp"

#include <stdexcept>
#include <string>
#include <algorithm>

namespace n64::rdp {

u32 RDP::triangle(u64 command) {
    bool has_shade = get_bit(command, 58);
    bool has_texture = get_bit(command, 57);
    bool has_zbuffer = get_bit(command, 56);
    bool l_major = get_bit(command, 55);
    u8 tile_index = get_bits(command, 50, 48);
    s32 y_low = sign_extend_n(get_bits(command, 45, 32), 14) >> 2;
    s32 y_mid = sign_extend_n(get_bits(command, 29, 16), 14) >> 2;
    s32 y_high = sign_extend_n(get_bits(command, 13, 0), 14) >> 2;

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;

    s32 x_low = sign_extend_n(get_bits(command, 59, 32), 28);
    s32 dx_low_dy = sign_extend_n(get_bits(command, 29, 0), 30);

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;

    s32 x_high = sign_extend_n(get_bits(command, 59, 32), 28);
    s32 dx_high_dy = sign_extend_n(get_bits(command, 29, 0), 30);

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;

    s32 x_mid = sign_extend_n(get_bits(command, 59, 32), 28);
    s32 dx_mid_dy = sign_extend_n(get_bits(command, 29, 0), 30);

    u32 fb_bpp = bytes_per_pixel(color_image_.size);
    u32 pixel_count = 0;

    if (has_shade) {
        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
    }

    s32 s = 0;
    s32 t = 0;
    s32 w = 0;

    s32 DsDx = 0;
    s32 DtDx = 0;
    s32 DwDx = 0;

    s32 DsDe = 0;
    s32 DtDe = 0;
    s32 DwDe = 0;

    s32 DsDy = 0;
    s32 DtDy = 0;
    s32 DwDy = 0;

    if (has_texture) {
        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
        s = get_bits(command, 63, 48) << 16;
        t = get_bits(command, 47, 32) << 16;
        w = get_bits(command, 31, 16) << 16;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
        DsDx = get_bits(command, 63, 48) << 16;
        DtDx = get_bits(command, 47, 32) << 16;
        DwDx = get_bits(command, 31, 16) << 16;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
        s += get_bits(command, 63, 48);
        t += get_bits(command, 47, 32);
        w += get_bits(command, 31, 16);

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
        DsDx += get_bits(command, 63, 48);
        DtDx += get_bits(command, 47, 32);
        DwDx += get_bits(command, 31, 16);

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
        DsDe = get_bits(command, 63, 48) << 16;
        DtDe = get_bits(command, 47, 32) << 16;
        DwDe = get_bits(command, 31, 16) << 16;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
        DsDy = get_bits(command, 63, 48) << 16;
        DtDy = get_bits(command, 47, 32) << 16;
        DwDy = get_bits(command, 31, 16) << 16;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
        DsDe += get_bits(command, 63, 48);
        DtDe += get_bits(command, 47, 32);
        DwDe += get_bits(command, 31, 16);

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
        DsDy += get_bits(command, 63, 48);
        DtDy += get_bits(command, 47, 32);
        DwDy += get_bits(command, 31, 16);
    }

    if (has_zbuffer) {
        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;

        command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
    }

    for (s32 y = y_high; y <= y_low; y++) {
        s32 x_left;
        s32 x_right;

        if (l_major) {
            x_left = x_high >> 16;
            x_right = (y < y_mid) ? x_mid >> 16 : x_low >> 16;
        } else {
            x_left = (y < y_mid) ? x_mid >> 16 : x_low >> 16;
            x_right = x_high >> 16;
        }

        for (s32 x = x_left; x < x_right; x++) {
            u32 fb_addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;
            s32 tex_s = (s + (x - x_left) * DsDx) >> 16;
            s32 tex_t = (t + (x - x_left) * DtDx) >> 16;
            process_pixel(fb_addr, x, tiles_[tile_index], tex_s, tex_t, Color(), has_texture, has_shade);
            pixel_count++;
        }

        x_high += dx_high_dy;
        if (y < y_mid) {
            x_mid += dx_mid_dy;
        } else {
            x_low += dx_low_dy;
        }

        s += DsDe;
        t += DtDe;
    }

    return std::max(pixel_count, 8u);
}

void RDP::process_pixel(u32 fb_addr, s32 x, const Tile& tile,
                        s32 tex_s, s32 tex_t,
                        const Color& shade,
                        bool has_texture, bool has_shade) {
    switch (cycle_type_) {
        case 0: { // 1-cycle
            Color texel;
            if (has_texture) {
                float tex_bpp = bytes_per_pixel(tile.size);
                u32 tmem_addr = tile.address + tex_t * tile.line_bytes + static_cast<u32>(tex_s * tex_bpp);
                texel = fetch_pixel_tmem(tmem_addr, tile.size, tile.format, tex_s & 1, tile.palette);
                if (is_pixel_transparent(texel)) return;
            }
            Color result = color_combiner_.combine(texel, shade);
            Color fb_before = read_pixel_framebuffer(fb_addr);
            result = blender_.blend(result, fb_before);
            write_pixel_framebuffer(fb_addr, result);
            break;
        }
        case 2: { // copy
            if (!has_texture) return;
            float tex_bpp = bytes_per_pixel(tile.size);
            u32 tmem_addr = tile.address + tex_t * tile.line_bytes + static_cast<u32>(tex_s * tex_bpp);
            Color texel = fetch_pixel_tmem(tmem_addr, tile.size, tile.format, tex_s & 1, tile.palette);
            if (is_pixel_transparent(texel)) return;
            write_pixel_framebuffer(fb_addr, texel);
            break;
        }
        case 3: { // fill
            switch (color_image_.size) {
                case Size::SIZE_16B: {
                    u16 color = (x & 1) ? (fill_color_ & 0xFFFF) : (fill_color_ >> 16);
                    rdram_.write_memory<u16>(fb_addr, color);
                    break;
                }
                case Size::SIZE_32B:
                    rdram_.write_memory<u32>(fb_addr, fill_color_);
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

u32 RDP::texture_rectangle(u64 command) {
    u16 right  = get_bits(command, 55, 44) >> 2;
    u16 bottom = get_bits(command, 43, 32) >> 2;
    u8 tile_index = get_bits(command, 26, 24);
    u16 left   = get_bits(command, 23, 12) >> 2;
    u16 top    = get_bits(command, 11, 0) >> 2;

    scissor_.clip(left, top, right, bottom);
    if (cycle_type_ > 1) {
        bottom++;
        right++;
    }

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;
    s32 s = sign_extend16(get_bits(command, 63, 48));
    s32 t = sign_extend16(get_bits(command, 47, 32));
    s32 dsdx = sign_extend16(get_bits(command, 31, 16));
    s32 dtdy = sign_extend16(get_bits(command, 15, 0));

    const auto& tile = tiles_[tile_index];
    u32 fb_bpp = bytes_per_pixel(color_image_.size);
    s32 s_inc = (cycle_type_ == 2) ? (dsdx >> 2) : dsdx;
    s32 t_acc = t << 5;
    u32 pixel_count = 0;

    for (u16 y = top; y < bottom; y++) {
        s32 s_acc = s << 5;
        s32 tex_t = t_acc >> 10;
        process_tmem_coordinates(tex_t, tile.shift_t, tile.mask_t, tile.mirror_t, tile.clamp_t, tile.upper_left_t, tile.lower_right_t);
        for (u16 x = left; x < right; x++) {
            s32 tex_s = s_acc >> 10;
            s_acc += s_inc;
            process_tmem_coordinates(tex_s, tile.shift_s, tile.mask_s, tile.mirror_s, tile.clamp_s, tile.upper_left_s, tile.lower_right_s);

            u32 fb_addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;
            process_pixel(fb_addr, x, tile, tex_s, tex_t, Color(), true, false);
            pixel_count++;
        }
        t_acc += dtdy;
    }
    return std::max(pixel_count, 8u);
}

u32 RDP::texture_rectangle_flip(u64 command) {
    u16 right  = get_bits(command, 55, 44) >> 2;
    u16 bottom = get_bits(command, 43, 32) >> 2;
    u8 tile_index = get_bits(command, 26, 24);
    u16 left   = get_bits(command, 23, 12) >> 2;
    u16 top    = get_bits(command, 11, 0) >> 2;

    scissor_.clip(left, top, right, bottom);

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;
    s32 s = sign_extend16(get_bits(command, 63, 48));
    s32 t = sign_extend16(get_bits(command, 47, 32));
    s32 dsdx = sign_extend16(get_bits(command, 31, 16));
    s32 dtdy = sign_extend16(get_bits(command, 15, 0));

    const auto& tile = tiles_[tile_index];
    u32 fb_bpp = bytes_per_pixel(color_image_.size);
    s32 s_inc = (cycle_type_ == 2) ? (dsdx >> 2) : dsdx;
    s32 s_acc = s << 5;
    u32 pixel_count = 0;

    for (u16 y = top; y < bottom; y++) {
        s32 t_acc = t << 5;
        s32 tex_s = s_acc >> 10;
        process_tmem_coordinates(tex_s, tile.shift_s, tile.mask_s, tile.mirror_s, tile.clamp_s, tile.upper_left_s, tile.lower_right_s);
        for (u16 x = left; x < right; x++) {
            s32 tex_t = t_acc >> 10;
            process_tmem_coordinates(tex_t, tile.shift_t, tile.mask_t, tile.mirror_t, tile.clamp_t, tile.upper_left_t, tile.lower_right_t);
            t_acc += dtdy;

            u32 fb_addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;
            process_pixel(fb_addr, x, tile, tex_s, tex_t, Color(), true, false);
            pixel_count++;
        }
        s_acc += s_inc;
    }
    return std::max(pixel_count, 8u);
}

u32 RDP::fill_rectangle(u64 command) {
    u16 right  = get_bits(command, 55, 44) >> 2;
    u16 bottom = get_bits(command, 43, 32) >> 2;
    u16 left   = get_bits(command, 23, 12) >> 2;
    u16 top    = get_bits(command, 11, 0) >> 2;

    scissor_.clip(left, top, right, bottom);

    switch (cycle_type_) {
        case 0:
            return 8;
        case 1:
            return 8;
        case 2:
            return copy_rectangle(right, bottom, left, top);
        case 3:
            return fill_rectangle(right, bottom, left, top);
        default:
            throw std::runtime_error("Invalid cycle type: " + std::to_string(cycle_type_));
    }
}

u32 RDP::copy_rectangle(u16 right, u16 bottom, u16 left, u16 top) {
    u32 fb_bpp = bytes_per_pixel(color_image_.size);
    u32 pixel_count = 0;

    for (u16 y = top; y <= bottom; y++) {
        for (u16 x = left; x <= right; x++) {
            s32 tex_s = x - left;
            s32 tex_t = y - top;
            u32 fb_addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;
            process_pixel(fb_addr, x, tiles_[tile_index_], tex_s, tex_t, Color(), true, false);
            pixel_count++;
        }
    }
    return std::max(pixel_count, 8u);
}

u32 RDP::fill_rectangle(u16 right, u16 bottom, u16 left, u16 top) {
    u32 fb_bpp = bytes_per_pixel(color_image_.size);
    u32 pixel_count = 0;

    for (u16 y = top; y <= bottom; y++) {
        for (u16 x = left; x <= right; x++) {
            u32 addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;
            process_pixel(addr, x, tiles_[0], 0, 0, Color(), false, false);
            pixel_count++;
        }
    }
    return std::max(pixel_count, 8u);
}

} // namespace n64::rdp
