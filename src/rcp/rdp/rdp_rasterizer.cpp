#include "rdp.hpp"
#include "rdp_log.hpp"
#include "../../memory/rdram.hpp"

#include <algorithm>

namespace n64::rdp {

u32 RDP::triangle(u64 command) {
    bool has_shade = get_bit(command, 58);
    bool has_texture = get_bit(command, 57);
    bool has_zbuffer = get_bit(command, 56);
    bool l_major = get_bit(command, 55);
    u8 tile_index = get_bits(command, 50, 48);
    FixedPointFloat y_low(get_bits(command, 45, 34), get_bits(command, 33, 32), 12, 2, true);
    FixedPointFloat y_mid(get_bits(command, 29, 18), get_bits(command, 17, 16), 12, 2, true);
    FixedPointFloat y_high(get_bits(command, 13, 2), get_bits(command, 1, 0), 12, 2, true);

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;

    FixedPointFloat x_low(get_bits(command, 59, 48), get_bits(command, 47, 32), 12, 16, true);
    FixedPointFloat dx_low_dy(get_bits(command, 29, 16), get_bits(command, 15, 0), 14, 16, true);

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;

    FixedPointFloat x_high(get_bits(command, 59, 48), get_bits(command, 47, 32), 12, 16, true);
    FixedPointFloat dx_high_dy(get_bits(command, 29, 16), get_bits(command, 15, 0), 14, 16, true);

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;

    FixedPointFloat x_mid(get_bits(command, 59, 48), get_bits(command, 47, 32), 12, 16, true);
    FixedPointFloat dx_mid_dy(get_bits(command, 29, 16), get_bits(command, 15, 0), 14, 16, true);

    RDP_LOG_PRIM("triangle: yH=%d yM=%d yL=%d tile=%u shade=%d tex=%d zbuf=%d lmaj=%d",
        y_high.integer(), y_mid.integer(), y_low.integer(), tile_index,
        has_shade, has_texture, has_zbuffer, l_major);

    u32 pixel_count = 0;
    [[maybe_unused]] int pix_log_cnt = 0;

    FixedPointFloat r, g, b, a;
    FixedPointFloat DrDx, DgDx, DbDx, DaDx;
    FixedPointFloat DrDe, DgDe, DbDe, DaDe;
    FixedPointFloat DrDy, DgDy, DbDy, DaDy;

    if (has_shade) {
        u64 word_0 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_1 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_2 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_3 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_4 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_5 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_6 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_7 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;

        r = FixedPointFloat(get_bits(word_0, 63, 48), get_bits(word_2, 63, 48), 16, 16, true);
        g = FixedPointFloat(get_bits(word_0, 47, 32), get_bits(word_2, 47, 32), 16, 16, true);
        b = FixedPointFloat(get_bits(word_0, 31, 16), get_bits(word_2, 31, 16), 16, 16, true);
        a = FixedPointFloat(get_bits(word_0, 15,  0), get_bits(word_2, 15,  0), 16, 16, true);

        DrDx = FixedPointFloat(get_bits(word_1, 63, 48), get_bits(word_3, 63, 48), 16, 16, true);
        DgDx = FixedPointFloat(get_bits(word_1, 47, 32), get_bits(word_3, 47, 32), 16, 16, true);
        DbDx = FixedPointFloat(get_bits(word_1, 31, 16), get_bits(word_3, 31, 16), 16, 16, true);
        DaDx = FixedPointFloat(get_bits(word_1, 15,  0), get_bits(word_3, 15,  0), 16, 16, true);

        DrDe = FixedPointFloat(get_bits(word_4, 63, 48), get_bits(word_6, 63, 48), 16, 16, true);
        DgDe = FixedPointFloat(get_bits(word_4, 47, 32), get_bits(word_6, 47, 32), 16, 16, true);
        DbDe = FixedPointFloat(get_bits(word_4, 31, 16), get_bits(word_6, 31, 16), 16, 16, true);
        DaDe = FixedPointFloat(get_bits(word_4, 15,  0), get_bits(word_6, 15,  0), 16, 16, true);

        DrDy = FixedPointFloat(get_bits(word_5, 63, 48), get_bits(word_7, 63, 48), 16, 16, true);
        DgDy = FixedPointFloat(get_bits(word_5, 47, 32), get_bits(word_7, 47, 32), 16, 16, true);
        DbDy = FixedPointFloat(get_bits(word_5, 31, 16), get_bits(word_7, 31, 16), 16, 16, true);
        DaDy = FixedPointFloat(get_bits(word_5, 15,  0), get_bits(word_7, 15,  0), 16, 16, true);
    }

    FixedPointFloat s, t, w;
    FixedPointFloat DsDx, DtDx, DwDx;
    FixedPointFloat DsDe, DtDe, DwDe;
    FixedPointFloat DsDy, DtDy, DwDy;

    if (has_texture) {
        u64 word_0 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_1 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_2 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_3 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_4 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_5 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_6 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_7 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;

        s    = FixedPointFloat(get_bits(word_0, 63, 48), get_bits(word_2, 63, 48), 16, 16, true);
        t    = FixedPointFloat(get_bits(word_0, 47, 32), get_bits(word_2, 47, 32), 16, 16, true);
        w    = FixedPointFloat(get_bits(word_0, 31, 16), get_bits(word_2, 31, 16), 16, 16, true);

        DsDx = FixedPointFloat(get_bits(word_1, 63, 48), get_bits(word_3, 63, 48), 16, 16, true);
        DtDx = FixedPointFloat(get_bits(word_1, 47, 32), get_bits(word_3, 47, 32), 16, 16, true);
        DwDx = FixedPointFloat(get_bits(word_1, 31, 16), get_bits(word_3, 31, 16), 16, 16, true);

        DsDe = FixedPointFloat(get_bits(word_4, 63, 48), get_bits(word_6, 63, 48), 16, 16, true);
        DtDe = FixedPointFloat(get_bits(word_4, 47, 32), get_bits(word_6, 47, 32), 16, 16, true);
        DwDe = FixedPointFloat(get_bits(word_4, 31, 16), get_bits(word_6, 31, 16), 16, 16, true);

        DsDy = FixedPointFloat(get_bits(word_5, 63, 48), get_bits(word_7, 63, 48), 16, 16, true);
        DtDy = FixedPointFloat(get_bits(word_5, 47, 32), get_bits(word_7, 47, 32), 16, 16, true);
        DwDy = FixedPointFloat(get_bits(word_5, 31, 16), get_bits(word_7, 31, 16), 16, 16, true);
    }

    FixedPointFloat z, DzDx, DzDe, DzDy;

    if (has_zbuffer) {
        u64 word_0 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;
        u64 word_1 = rdram_.read_memory<u64>(current_.raw); current_.raw += 8;

        z    = FixedPointFloat(get_bits(word_0, 63, 48), get_bits(word_0, 47, 32), 16, 16, true);
        DzDx = FixedPointFloat(get_bits(word_0, 31, 16), get_bits(word_0, 15,  0), 16, 16, true);
        DzDe = FixedPointFloat(get_bits(word_1, 63, 48), get_bits(word_1, 47, 32), 16, 16, true);
        DzDy = FixedPointFloat(get_bits(word_1, 31, 16), get_bits(word_1, 15,  0), 16, 16, true);
    }

    s32 y_start = std::max(y_high, scissor_.scissor_rect.top).integer();
    s32 y_end = std::min(y_low, scissor_.scissor_rect.bottom).integer();

    for (s32 y = y_start; y < y_end; y++) {
        FixedPointFloat x_left;
        FixedPointFloat x_right;

        if (l_major) {
            x_left = x_high;
            x_right = (y < y_mid.integer()) ? x_mid : x_low;
        } else {
            x_left = (y < y_mid.integer()) ? x_mid : x_low;
            x_right = x_high;
        }

        s32 x_start = std::max(x_left.integer(), scissor_.scissor_rect.left.integer());
        s32 x_end = std::min(x_right.integer(), scissor_.scissor_rect.right.integer());

        if (cycle_type_ > 1) x_end++;

        s32 major_x = x_high.integer();

        FixedPointFloat eff_DsDx = (cycle_type_ == 2) ? (DsDx >> 2) : DsDx;
        FixedPointFloat eff_DtDx = (cycle_type_ == 2) ? (DtDx >> 2) : DtDx;

        bool do_offset = (dx_high_dy.raw() != 0) && (cycle_type_ <= 1);
        FixedPointFloat dsdiff = (do_offset && DsDx.raw() == 0) ? DsDe : FixedPointFloat();
        FixedPointFloat dtdiff = (do_offset && DtDx.raw() == 0) ? DtDe : FixedPointFloat();

        for (s32 x = x_start; x < x_end; x++) {
            s64 dx = x - major_x;
            s32 tex_s = (s + dsdiff + eff_DsDx * dx).integer() >> 5;
            s32 tex_t = (t + dtdiff + eff_DtDx * dx).integer() >> 5;
            s32 r_shade = std::clamp((r + DrDx * dx).integer(), 0, 255);
            s32 g_shade = std::clamp((g + DgDx * dx).integer(), 0, 255);
            s32 b_shade = std::clamp((b + DbDx * dx).integer(), 0, 255);
            s32 a_shade = std::clamp((a + DaDx * dx).integer(), 0, 255);
            s32 z_pixel = std::clamp((z + DzDx * dx).integer(), 0, 0x7FFF);

            Color shade(r_shade, g_shade, b_shade, a_shade);
            u8 pixel_cvg = compute_pixel_cvg(x, x_left, x_right);
            process_pixel(x, y, tile_index, tex_s, tex_t, shade, z_pixel, pixel_cvg, has_texture, has_shade, pix_log_cnt);
            pixel_count++;
        }

        x_high += dx_high_dy;
        if (y < y_mid.integer()) {
            x_mid += dx_mid_dy;
        } else {
            x_low += dx_low_dy;
        }

        s += DsDe;
        t += DtDe;

        r += DrDe;
        g += DgDe;
        b += DbDe;
        a += DaDe;

        z += DzDe;
    }

    return std::max(pixel_count, 8u);
}

void RDP::process_pixel(s32 x, s32 y, u8 tile_index,
                        s32 tex_s, s32 tex_t,
                        const Color& shade, s32 z_depth,
                        u8 pixel_cvg,
                        bool has_texture, bool has_shade,
                        int& pix_log_cnt) {
    u32 fb_bpp = bytes_per_pixel(color_image_.size);
    u32 fb_addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;

    u32 z_addr = z_buffer_addr_ + (y * color_image_.width + x) * 2;
    const auto& tile = tiles_[tile_index];

    if (has_texture) {
        process_tmem_coordinates(tex_s, tile.shift_s, tile.mask_s, tile.mirror_s, tile.clamp_s, tile.upper_left_s, tile.lower_right_s);
        process_tmem_coordinates(tex_t, tile.shift_t, tile.mask_t, tile.mirror_t, tile.clamp_t, tile.upper_left_t, tile.lower_right_t);
    }

    if (z_source_select_) {
        z_depth = z_prim_depth_;
    }

    switch (cycle_type_) {
        case 0: { // 1-cycle
            Color texel0, texel1;

            if (z_compare_enable_) {
                u16 old_z = rdram_.read_memory<u16>(z_addr);
                if (z_depth >= old_z) return;
            }
            if (z_update_enable_) {
                rdram_.write_memory<u16>(z_addr, z_depth);
            }

            if (has_texture) {
                s32 s_offset = (tile.size == Size::SIZE_4B) ? (tex_s >> 1) : static_cast<s32>(tex_s * bytes_per_pixel(tile.size));
                u32 tmem_addr = (tile.address + tex_t * (s32)tile.line_bytes + s_offset) & 0xFFF;
                texel0 = fetch_pixel_tmem(tmem_addr, tile.size, tile.format, tex_s & 1, tile.palette);

                const auto& next_tile = tiles_[(tile_index + 1) & 7];
                s32 s_offset1 = (next_tile.size == Size::SIZE_4B) ? (tex_s >> 1) : static_cast<s32>(tex_s * bytes_per_pixel(next_tile.size));
                u32 tmem_addr1 = (next_tile.address + tex_t * (s32)next_tile.line_bytes + s_offset1) & 0xFFF;
                texel1 = fetch_pixel_tmem(tmem_addr1, next_tile.size, next_tile.format, tex_s & 1, next_tile.palette);

                if (is_pixel_transparent(texel0)) return;
            }
            Color result = color_combiner_.combine(texel0, texel1, shade, Color(), 1);

            if (apply_alpha_coverage(pixel_cvg, y * color_image_.width + x, result)) return;

            apply_alpha_dither(x, y, result);

            if (alpha_compare_enable_) {
                u8 threshold = blender_.get_alpha_threshold(dither_alpha_enable_);
                if (result.alpha < threshold) return;
            }

            Color fb_before = read_pixel_framebuffer(fb_addr);
            u8 blend_cvg = cvg_buffer_[y * color_image_.width + x];
            u8 cvg_5bit = (blend_cvg << 2) | (blend_cvg >> 1);
            result = blender_.blend(result, fb_before, shade, cvg_5bit, 0);
            apply_rgb_dither(x, y, result);

            RDP_LOG_PIXEL(pix_log_cnt, "(%d,%d) tex=(%d,%d) texel=(%u,%u,%u,%u) combined=(%u,%u,%u,%u) final=(%u,%u,%u,%u)",
                x, y, tex_s, tex_t,
                texel0.red, texel0.green, texel0.blue, texel0.alpha,
                result.red, result.green, result.blue, result.alpha,
                result.red, result.green, result.blue, result.alpha);

            write_pixel_framebuffer(fb_addr, result);
            break;
        }
        case 1: { // 2-cycle
            Color texel0, texel1;

            if (z_compare_enable_) {
                u16 old_z = rdram_.read_memory<u16>(z_addr);
                if (z_depth >= old_z) return;
            }
            if (z_update_enable_) {
                rdram_.write_memory<u16>(z_addr, z_depth);
            }

            if (has_texture) {
                s32 s_offset = (tile.size == Size::SIZE_4B) ? (tex_s >> 1) : static_cast<s32>(tex_s * bytes_per_pixel(tile.size));
                u32 tmem_addr = (tile.address + tex_t * (s32)tile.line_bytes + s_offset) & 0xFFF;
                texel0 = fetch_pixel_tmem(tmem_addr, tile.size, tile.format, tex_s & 1, tile.palette);

                const auto& next_tile = tiles_[(tile_index + 1) & 7];
                s32 s_offset1 = (next_tile.size == Size::SIZE_4B) ? (tex_s >> 1) : static_cast<s32>(tex_s * bytes_per_pixel(next_tile.size));
                u32 tmem_addr1 = (next_tile.address + tex_t * (s32)next_tile.line_bytes + s_offset1) & 0xFFF;
                texel1 = fetch_pixel_tmem(tmem_addr1, next_tile.size, next_tile.format, tex_s & 1, next_tile.palette);

                if (is_pixel_transparent(texel0)) return;
            }

            Color combined0 = color_combiner_.combine(texel0, texel1, shade, Color(), 0);
            Color result = color_combiner_.combine(texel0, texel1, shade, combined0, 1);

            if (apply_alpha_coverage(pixel_cvg, y * color_image_.width + x, result)) return;

            apply_alpha_dither(x, y, result);

            if (alpha_compare_enable_) {
                u8 threshold = blender_.get_alpha_threshold(dither_alpha_enable_);
                if (result.alpha < threshold) return;
            }

            Color fb_before = read_pixel_framebuffer(fb_addr);
            u8 blend_cvg = cvg_buffer_[y * color_image_.width + x];
            u8 cvg_5bit = (blend_cvg << 2) | (blend_cvg >> 1);
            Color blended0 = blender_.blend(result, fb_before, shade, cvg_5bit, 0);
            result = blender_.blend(blended0, fb_before, shade, cvg_5bit, 1);

            apply_rgb_dither(x, y, result);

            RDP_LOG_PIXEL(pix_log_cnt, "(%d,%d) tex=(%d,%d) texel=(%u,%u,%u,%u) combined=(%u,%u,%u,%u) final=(%u,%u,%u,%u)",
                x, y, tex_s, tex_t,
                texel0.red, texel0.green, texel0.blue, texel0.alpha,
                combined0.red, combined0.green, combined0.blue, combined0.alpha,
                result.red, result.green, result.blue, result.alpha);

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
    Rectangle texture_rect(command);
    u8 tile_index = get_bits(command, 26, 24);

    scissor_.clip(texture_rect);
    if (cycle_type_ > 1) {
        texture_rect.bottom++;
        texture_rect.right++;
    }

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;
    FixedPointFloat s(get_bits(command, 63, 53), get_bits(command, 52, 48), 11, 5, true);
    FixedPointFloat t(get_bits(command, 47, 37), get_bits(command, 36, 32), 11, 5, true);
    FixedPointFloat dsdx(get_bits(command, 31, 26), get_bits(command, 25, 16), 6, 10, true);
    FixedPointFloat dtdy(get_bits(command, 15, 10), get_bits(command, 9, 0), 6, 10, true);

    const auto& tile = tiles_[tile_index];
    FixedPointFloat s_inc = (cycle_type_ == 2) ? (dsdx >> 2) : dsdx;
    FixedPointFloat t_acc = t;
    u32 pixel_count = 0;
    [[maybe_unused]] int pix_log_cnt = 0;

    RDP_LOG_PRIM("texrect: rect=(%d,%d)-(%d,%d) tile=%u S=%d T=%d DsDx_raw=%d DtDy_raw=%d mask_s=%u mask_t=%u",
        texture_rect.left.integer(), texture_rect.top.integer(),
        texture_rect.right.integer(), texture_rect.bottom.integer(),
        tile_index, s.integer(), t.integer(), dsdx.raw(), dtdy.raw(),
        tile.mask_s, tile.mask_t);

    for (u16 y = texture_rect.top.integer(); y < texture_rect.bottom.integer(); y++) {
        FixedPointFloat s_acc = s;
        s32 tex_t = t_acc.integer();
        for (u16 x = texture_rect.left.integer(); x < texture_rect.right.integer(); x++) {
            s32 tex_s = s_acc.integer();
            s_acc += s_inc;

            process_pixel(x, y, tile_index, tex_s, tex_t, Color(), 0, 0x07, true, false, pix_log_cnt);
            pixel_count++;
        }
        t_acc += dtdy;
    }
    return std::max(pixel_count, 8u);
}

u32 RDP::texture_rectangle_flip(u64 command) {
    
    Rectangle flip_rect(command);
    u8 tile_index = get_bits(command, 26, 24);

    scissor_.clip(flip_rect);

    command = rdram_.read_memory<u64>(current_.raw);
    current_.raw += 8;
    FixedPointFloat s(get_bits(command, 63, 53), get_bits(command, 52, 48), 11, 5, true);
    FixedPointFloat t(get_bits(command, 47, 37), get_bits(command, 36, 32), 11, 5, true);
    FixedPointFloat dsdx(get_bits(command, 31, 26), get_bits(command, 25, 16), 6, 10, true);
    FixedPointFloat dtdy(get_bits(command, 15, 10), get_bits(command, 9, 0), 6, 10, true);

    FixedPointFloat s_inc = (cycle_type_ == 2) ? (dsdx >> 2) : dsdx;
    FixedPointFloat s_acc = s;
    u32 pixel_count = 0;
    [[maybe_unused]] int pix_log_cnt = 0;

    RDP_LOG_PRIM("texrect_flip: rect=(%d,%d)-(%d,%d) tile=%u S=%d T=%d DsDx_raw=%d DtDy_raw=%d",
        flip_rect.left.integer(), flip_rect.top.integer(),
        flip_rect.right.integer(), flip_rect.bottom.integer(),
        tile_index, s.integer(), t.integer(), dsdx.raw(), dtdy.raw());

    for (u16 y = flip_rect.top.integer(); y < flip_rect.bottom.integer(); y++) {
        FixedPointFloat t_acc = t;
        s32 tex_s = s_acc.integer();
        for (u16 x = flip_rect.left.integer(); x < flip_rect.right.integer(); x++) {
            s32 tex_t = t_acc.integer();
            t_acc += dtdy;

            process_pixel(x, y, tile_index, tex_s, tex_t, Color(), 0, 0x07, true, false, pix_log_cnt);
            pixel_count++;
        }
        s_acc += s_inc;
    }
    return std::max(pixel_count, 8u);
}

u32 RDP::fill_rectangle(u64 command) {
    Rectangle fill_rect(command);

    scissor_.clip(fill_rect);

#ifdef RDP_LOG
    {
        static constexpr const char* cycle_names[] = {"1cycle", "2cycle", "copy", "fill"};
        RDP_LOG_PRIM("fill_rect: rect=(%d,%d)-(%d,%d) cycle=%s",
            fill_rect.left.integer(), fill_rect.top.integer(),
            fill_rect.right.integer(), fill_rect.bottom.integer(),
            cycle_names[cycle_type_]);
    }
#endif

    switch (cycle_type_) {
        case 0:
        case 1: {
            u32 pixel_count = 0;
            [[maybe_unused]] int pix_log_cnt = 0;
            for (u16 y = fill_rect.top.integer(); y <= fill_rect.bottom.integer(); y++) {
                for (u16 x = fill_rect.left.integer(); x <= fill_rect.right.integer(); x++) {
                    process_pixel(x, y, 0, 0, 0, Color(), 0, 0x07, false, false, pix_log_cnt);
                    pixel_count++;
                }
            }
            return std::max(pixel_count, 8u);
        }
        case 2:
            return copy_rectangle(fill_rect);
        case 3:
            return fill_rectangle(fill_rect);
        default:
            return 0;
    }
}

u32 RDP::copy_rectangle(const Rectangle& copy_rect) {
    u32 pixel_count = 0;
    [[maybe_unused]] int pix_log_cnt = 0;

    for (u16 y = copy_rect.top.integer(); y <= copy_rect.bottom.integer(); y++) {
        for (u16 x = copy_rect.left.integer(); x <= copy_rect.right.integer(); x++) {
            s32 tex_s = x - copy_rect.left.integer();
            s32 tex_t = y - copy_rect.top.integer();
            process_pixel(x, y, tile_index_, tex_s, tex_t, Color(), 0, 0x07, true, false, pix_log_cnt);
            pixel_count++;
        }
    }
    return std::max(pixel_count, 8u);
}

u32 RDP::fill_rectangle(const Rectangle& fill_rect) {
    u32 pixel_count = 0;
    [[maybe_unused]] int pix_log_cnt = 0;

    for (u16 y = fill_rect.top.integer(); y <= fill_rect.bottom.integer(); y++) {
        for (u16 x = fill_rect.left.integer(); x <= fill_rect.right.integer(); x++) {
            process_pixel(x, y, 0, 0, 0, Color(), 0, 0x07, false, false, pix_log_cnt);
            pixel_count++;
        }
    }
    return std::max(pixel_count, 8u);
}

void RDP::apply_alpha_dither(s32 x, s32 y, Color& color) {
    u8 x_index = x % 4;
    u8 y_index = (scissor_enable_) ? (y >> 1) % 4 : y % 4;
    auto& selected_matrix = rgb_dither_sel_ % 2 == 0 ? magic_square_matrix_ : bayer_matrix_;

    switch (alpha_dither_sel_) {
        case 0:
            color.alpha = std::clamp(color.alpha + selected_matrix[y_index][x_index], 0, 255);
            break;
        case 1:
            color.alpha = std::clamp(color.alpha + ((~selected_matrix[y_index][x_index]) & 0x07), 0, 255);
            break;
        case 2:
            color.alpha = std::clamp(color.alpha + (rand() & 0x07), 0, 255);
            break;
        case 3:
            break;
    }
}

void RDP::apply_rgb_dither(s32 x, s32 y, Color& color) {
    u8 x_index = x % 4;
    u8 y_index = (scissor_enable_) ? (y >> 1) % 4 : y % 4;

    switch (rgb_dither_sel_) {
        case 0: {
            int dither = magic_square_matrix_[y_index][x_index];
            color.red = std::clamp(color.red + dither, 0, 255);
            color.green = std::clamp(color.green + dither, 0, 255);
            color.blue = std::clamp(color.blue + dither, 0, 255);
            break;
        }
        case 1: {
            int dither = bayer_matrix_[y_index][x_index];
            color.red = std::clamp(color.red + dither, 0, 255);
            color.green = std::clamp(color.green + dither, 0, 255);
            color.blue = std::clamp(color.blue + dither, 0, 255);
            break;
        }
        case 2: {
            color.red = std::clamp(color.red + (rand() & 0x07), 0, 255);
            color.green = std::clamp(color.green + (rand() & 0x07), 0, 255);
            color.blue = std::clamp(color.blue + (rand() & 0x07), 0, 255);
            break;
        }
        case 3:
            break;
    }
}

u8 RDP::compute_pixel_cvg(s32 x, FixedPointFloat x_left, FixedPointFloat x_right) const {
    // TODO: Implement sub-pixel coverage calculation
    if (x == x_left.integer()) {
        u8 frac = x_left.frac() >> 13;
        return 7 - frac;
    } else if (x == x_right.integer() - 1) {
        u8 frac = x_right.frac() >> 13;
        return frac ? frac : 7;
    }
    return 7;
}

bool RDP::apply_alpha_coverage(u8 pixel_cvg, u32 cvg_index, Color& color) {
    u8 new_cvg = pixel_cvg;
    u8 old_cvg = image_read_enable_ ? cvg_buffer_[cvg_index] : 0;

    if (cvg_x_alpha_) {
        new_cvg = (pixel_cvg * color.alpha + 128) / 255;
    }
    if (alpha_cvg_select_) {
        color.alpha = (new_cvg << 5) | (new_cvg << 2) | (new_cvg >> 1);
    }

    bool overflow = false;
    u8 final_cvg;
    switch (cvg_dest_) {
        case 0: overflow = (old_cvg + new_cvg) > 7; final_cvg = std::min<u8>(old_cvg + new_cvg, 7); break;
        case 1: overflow = (old_cvg + new_cvg) > 7; final_cvg = (old_cvg + new_cvg) & 7; break;
        case 2: final_cvg = 7; break;
        case 3: final_cvg = old_cvg; break;
        default: final_cvg = 7; break;
    }
    cvg_buffer_[cvg_index] = final_cvg;

    return color_on_cvg_ && !overflow;
}

} // namespace n64::rdp
