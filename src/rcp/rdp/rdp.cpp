#include "rdp.hpp"
#include "../../memory/rdram.hpp"
#include "../../interfaces/mi.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace n64::rdp {

// RDP implementation roadmap:
//
// Phase 1 - Command processing: DONE
// Phase 2 - Basic rendering: DONE (Fill_Rectangle, Set_Color_Image, Set_Scissor)
// Phase 3 - Copy mode textures: DONE (Load_Block, Load_Tile, Texture_Rectangle copy)
// Phase 4 - 1-cycle mode: DONE (Color combiner, Texture_Rectangle/Flip, shift/mask/mirror)
// Phase 5 - Color registers: DONE (Primitive, Environment, Fog, Blend colors)
// Phase 6 - Texture formats: DONE (RGBA 16b/32b, IA 16b/8b/4b, I 8b/4b, CI 4b/8b + TLUT palettes)
//
// Phase 7 - Remaining rectangle features (CURRENT):
// TODO: Implement Set_Tile_Size clamping logic in process_tmem_coordinates
// TODO: Implement YUV texture format + Set_Convert (K0-K5 YUV->RGB coefficients)
//
// Phase 8 - Triangles:
// TODO: Implement edge walking rasterizer (Fill Triangle)
// TODO: Implement Gouraud shading (Shade Triangle - per-vertex color interpolation)
// TODO: Implement texture coordinate interpolation (Texture Triangle)
// TODO: Implement Shade+Texture Triangle (combined)
// TODO: Implement Z-buffer triangle variants
//
// Phase 9 - Blender & Z-buffer:
// TODO: Implement full blender (alpha blending, fog, coverage)
// TODO: Implement Z-buffer read/write and comparison
// TODO: Implement Set_Depth_Image and Set_Primitive_Depth
//
// Phase 10 - Accuracy & advanced:
// TODO: Implement 2-cycle mode (two combiner passes per pixel)
// TODO: Implement LOD / detail / sharpen texture filtering
// TODO: Implement bilinear / median filtering
// TODO: Implement coverage calculation and anti-aliasing
// TODO: Implement dithering
// TODO: Implement XBUS mode (commands from RSP DMEM instead of RDRAM)
// TODO: Implement cycle-accurate RDP timing

// Number of extra 64-bit words per triangle command variant
// Bit 0 = zbuffer (+2 words), Bit 1 = texture (+8 words), Bit 2 = shade (+8 words)
static constexpr u32 triangle_extra_words[8] = {
    // 0x08: no shade, no texture, no zbuf
    0,
    // 0x09: zbuf only
    2,
    // 0x0A: texture only
    8,
    // 0x0B: texture + zbuf
    10,
    // 0x0C: shade only
    8,
    // 0x0D: shade + zbuf
    10,
    // 0x0E: shade + texture
    16,
    // 0x0F: shade + texture + zbuf
    18,
};

RDP::RDP(memory::RDRAM& rdram, n64::interfaces::MI& mi)
    : rdram_(rdram)
    , mi_(mi)
    , start_{}
    , end_{}
    , current_{}
    , status_{}
    , clock_{}
    , buf_busy_{}
    , pipe_busy_{}
    , tmem_busy_{}
    , tbist_{}
    , test_mode_{}
    , buftest_addr_{}
    , buftest_data_{}
{
    command_table_.fill(&RDP::nop);

    command_table_[0x08] = &RDP::triangle;
    command_table_[0x09] = &RDP::triangle;
    command_table_[0x0A] = &RDP::triangle;
    command_table_[0x0B] = &RDP::triangle;
    command_table_[0x0C] = &RDP::triangle;
    command_table_[0x0D] = &RDP::triangle;
    command_table_[0x0E] = &RDP::triangle;
    command_table_[0x0F] = &RDP::triangle;

    command_table_[0x24] = &RDP::texture_rectangle;
    command_table_[0x25] = &RDP::texture_rectangle_flip;
    command_table_[0x26] = &RDP::sync_load;
    command_table_[0x27] = &RDP::sync_pipe;
    command_table_[0x28] = &RDP::sync_tile;
    command_table_[0x29] = &RDP::sync_full;
    command_table_[0x2A] = &RDP::set_key_gb;
    command_table_[0x2B] = &RDP::set_key_r;
    command_table_[0x2C] = &RDP::set_convert;
    command_table_[0x2D] = &RDP::set_scissor;
    command_table_[0x2E] = &RDP::set_primitive_depth;
    command_table_[0x2F] = &RDP::set_other_modes;
    command_table_[0x30] = &RDP::load_tlut;

    command_table_[0x32] = &RDP::set_tile_size;
    command_table_[0x33] = &RDP::load_block;
    command_table_[0x34] = &RDP::load_tile;
    command_table_[0x35] = &RDP::set_tile;
    command_table_[0x36] = &RDP::fill_rectangle;
    command_table_[0x37] = &RDP::set_fill_color;
    command_table_[0x38] = &RDP::set_fog_color;
    command_table_[0x39] = &RDP::set_blend_color;
    command_table_[0x3A] = &RDP::set_primitive_color;
    command_table_[0x3B] = &RDP::set_environment_color;
    command_table_[0x3C] = &RDP::set_combine_mode;
    command_table_[0x3D] = &RDP::set_texture_image;
    command_table_[0x3E] = &RDP::set_depth_image;
    command_table_[0x3F] = &RDP::set_color_image;
}

RDP::~RDP() {}

template<typename T>
T RDP::read(u32 address) const {
    return read_register(address);
}

template<typename T>
void RDP::write(u32 address, T value) {
    write_register(address, value);
}

u32 RDP::read_register(u32 address) const {
    switch (address) {
        case DPC_START:     return start_.raw;
        case DPC_END:       return end_.raw;
        case DPC_CURRENT:   return current_.raw;
        case DPC_STATUS:    return status_.raw;
        case DPC_CLOCK:     return clock_.raw;
        case DPC_BUF_BUSY:  return buf_busy_.raw;
        case DPC_PIPE_BUSY: return pipe_busy_.raw;
        case DPC_TMEM_BUSY: return tmem_busy_.raw;
        case DPS_TBIST:        return tbist_.raw;
        case DPS_TEST_MODE:    return test_mode_.raw;
        case DPS_BUFTEST_ADDR: return buftest_addr_.raw;
        case DPS_BUFTEST_DATA: return buftest_data_.raw;
        default:
            throw std::runtime_error("Invalid RDP register address: " + std::to_string(address));
    }
}

void RDP::write_register(u32 address, u32 value) {
    switch (address) {
        case DPC_START:
            start_.raw = value & 0x00FFFFF8;  // 24-bit, 64-bit aligned
            status_.start_pending = 1;
            break;
        case DPC_END:
            end_.raw = value & 0x00FFFFF8;  // 24-bit, 64-bit aligned
            if (!status_.start_pending) {
                // Incremental transfer: continue/restore from CURRENT to new END
                // Works whether previous transfer is running or already finished
                run_commands();
            } else if (!status_.dma_busy) {
                // New transfer: no transfer running, start from START to END
                current_.raw = start_.raw;
                status_.start_pending = 0;
                run_commands();
            } else {
                // New transfer requested but current transfer still running:
                // queue it as pending, will start when current transfer finishes
                // Further writes to DPC_END just update the pending end address
                status_.end_pending = 1;
            }
            break;
        case DPC_STATUS: {
            // Clear counters
            if (get_bit(value, 9)) clock_.raw = 0;
            if (get_bit(value, 8)) buf_busy_.raw = 0;
            if (get_bit(value, 7)) pipe_busy_.raw = 0;
            if (get_bit(value, 6)) tmem_busy_.raw = 0;
            // Clear/set flags
            clear_set_resolver(status_.raw, get_bit(value, 0), get_bit(value, 1), 0);  // xbus_dmem_dma
            clear_set_resolver(status_.raw, get_bit(value, 2), get_bit(value, 3), 1);  // freeze
            clear_set_resolver(status_.raw, get_bit(value, 4), get_bit(value, 5), 2);  // flush
            break;
        }
        case DPS_TBIST:        tbist_.raw = value & 0x00000FF3; break;
        case DPS_TEST_MODE:    test_mode_.raw = value & 0x00000001; break;
        case DPS_BUFTEST_ADDR: buftest_addr_.raw = value & 0x0000007F; break;
        case DPS_BUFTEST_DATA: buftest_data_.raw = value; break;
        default:
            throw std::runtime_error("Invalid RDP register address: " + std::to_string(address));
    }
}

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

bool RDP::is_pixel_transparent(const Color& color) const {
    // In 1-cycle mode, always skip transparent texels (blender approximation)
    if (cycle_type_ == 0) {
        return color.alpha == 0;
    }
    // In copy mode, only skip if alpha compare is enabled
    if (!alpha_compare_enable_) return false;
    return color.alpha == 0;
}

void RDP::process_tmem_coordinates(s32& coord, u8 shift, u8 mask, bool mirror) const {
    // 1. Shift
    if (shift <= 10)
        coord >>= shift;
    else
        coord <<= (16 - shift);

    // 2. Mask + Mirror
    if (mask == 0) return;

    if (mirror && (coord & (1 << mask))) {
        coord = ((1 << mask) - 1) - (coord & ((1 << mask) - 1));
    } else {
        coord &= (1 << mask) - 1;
    }
}

void RDP::run_commands() {
    status_.dma_busy = 1;

    execute_commands();

    status_.dma_busy = 0;

    // If a pending transfer was queued, start it now
    if (status_.end_pending) {
        status_.end_pending = 0;
        current_.raw = start_.raw;
        status_.start_pending = 0;
        run_commands();
    }
}

void RDP::execute_commands() {
    while (current_.raw < end_.raw) {
        u64 command = rdram_.read_memory<u64>(current_.raw);
        current_.raw += 8;
        u8 command_id = (command >> 56) & 0x3F;
        (this->*command_table_[command_id])(command);
    }
}

// ============================================================================
// Command handlers
// ============================================================================

void RDP::nop(u64 command) {
    // Do nothing
}

void RDP::triangle(u64 command) {
    // Triangle commands are multi-word: 4 base words + extra depending on variant
    // command_id & 0x07 indexes into triangle_extra_words table
    u8 variant = (command >> 56) & 0x07;
    u32 extra = triangle_extra_words[variant];
    // Skip the remaining 3 base edge coefficient words + extra words
    current_.raw += (3 + extra) * 8;
}

void RDP::texture_rectangle(u64 command) {
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
    float tex_bpp = bytes_per_pixel(tile.size);
    u32 fb_bpp = bytes_per_pixel(color_image_.size);
    s32 s_inc = (cycle_type_ == 2) ? (dsdx >> 2) : dsdx;
    s32 t_acc = t << 5;

    for (u16 y = top; y < bottom; y++) {
        s32 s_acc = s << 5;
        s32 tex_t = t_acc >> 10;
        process_tmem_coordinates(tex_t, tile.shift_t, tile.mask_t, tile.mirror_t);
        for (u16 x = left; x < right; x++) {
            s32 tex_s = s_acc >> 10;
            s_acc += s_inc;
            process_tmem_coordinates(tex_s, tile.shift_s, tile.mask_s, tile.mirror_s);

            u32 tmem_addr = tile.address + tex_t * tile.line_bytes + (u32)(tex_s * tex_bpp);
            u32 fb_addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;

            Color pixel = fetch_pixel_tmem(tmem_addr, tile.size, tile.format, tex_s & 1, tile.palette);
            if (is_pixel_transparent(pixel)) continue;

            if (cycle_type_ == 0) {
                pixel = color_combiner_.combine(pixel, Color());
            }

            write_pixel_framebuffer(fb_addr, pixel);
        }
        t_acc += dtdy;
    }
}

void RDP::texture_rectangle_flip(u64 command) {
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
    float tex_bpp = bytes_per_pixel(tile.size);
    u32 fb_bpp = bytes_per_pixel(color_image_.size);
    s32 s_inc = (cycle_type_ == 2) ? (dsdx >> 2) : dsdx;
    s32 s_acc = s << 5;

    for (u16 y = top; y < bottom; y++) {
        s32 t_acc = t << 5;
        s32 tex_s = s_acc >> 10;
        process_tmem_coordinates(tex_s, tile.shift_s, tile.mask_s, tile.mirror_s);
        for (u16 x = left; x < right; x++) {
            s32 tex_t = t_acc >> 10;
            process_tmem_coordinates(tex_t, tile.shift_t, tile.mask_t, tile.mirror_t);
            t_acc += dtdy;

            u32 tmem_addr = tile.address + tex_t * tile.line_bytes + (u32)(tex_s * tex_bpp);
            u32 fb_addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;

            Color pixel = fetch_pixel_tmem(tmem_addr, tile.size, tile.format, tex_s & 1, tile.palette);
            if (is_pixel_transparent(pixel)) continue;

            if (cycle_type_ == 0) {
                pixel = color_combiner_.combine(pixel, Color());
            }

            write_pixel_framebuffer(fb_addr, pixel);
        }
        s_acc += s_inc;
    }
}

void RDP::sync_load(u64 command) {}
void RDP::sync_pipe(u64 command) {}
void RDP::sync_tile(u64 command) {}

void RDP::sync_full(u64 command) {
    mi_.set_interrupt(interfaces::MI_INTERRUPT_BITS::MI_INTERRUPT_DP);
}

void RDP::set_key_gb(u64 command) {}
void RDP::set_key_r(u64 command) {}
void RDP::set_convert(u64 command) {}

void RDP::set_scissor(u64 command) {
    // Coordinates are 10.2 fixed point (12 bits each), shift >> 2 for integer pixels
    scissor_.upper_left_x = get_bits(command, 55, 44) >> 2;
    scissor_.upper_left_y = get_bits(command, 43, 32) >> 2;
    scissor_.interlace_enable = get_bit(command, 25);
    scissor_.odd_even = get_bit(command, 24);
    scissor_.lower_right_x = get_bits(command, 23, 12) >> 2;
    scissor_.lower_right_y = get_bits(command, 11, 0) >> 2;
}

void RDP::set_primitive_depth(u64 command) {}

void RDP::set_other_modes(u64 command) {
    atomic_prim_ = get_bit(command, 55);
    cycle_type_ = (command >> 52) & 0x03;
    perspective_texture_enable_ = get_bit(command, 51);
    detail_textures_enable_ = get_bit(command, 50);
    sharpen_textures_enable_ = get_bit(command, 49);
    texture_lod_enable_ = get_bit(command, 48);
    tlut_enable_ = get_bit(command, 47);
    tlut_type_ = get_bit(command, 46) ? Format::FORMAT_IA : Format::FORMAT_RGB;
    sample_type_ = get_bit(command, 45);
    mid_texel_ = get_bit(command, 44);
    bi_lerp_0_ = get_bit(command, 43);
    bi_lerp_1_ = get_bit(command, 42);
    convert_one_ = get_bit(command, 41);
    key_enable_ = get_bit(command, 40);
    rgb_dither_sel_ = (command >> 38) & 0x03;
    alpha_dither_sel_ = (command >> 36) & 0x03;
    bl_m1a_0_ = (command >> 30) & 0x03;
    bl_m1a_1_ = (command >> 28) & 0x03;
    bl_m1b_0_ = (command >> 26) & 0x03;
    bl_m1b_1_ = (command >> 24) & 0x03;
    bl_ma_0_ = (command >> 22) & 0x03;
    bl_ma_1_ = (command >> 20) & 0x03;
    bl_mb_0_ = (command >> 18) & 0x03;
    bl_mb_1_ = (command >> 16) & 0x03;
    force_blend_ = get_bit(command, 14);
    alpha_cvg_select_ = get_bit(command, 13);
    cvg_x_alpha_ = get_bit(command, 12);
    z_mode_ = (command >> 10) & 0x03;
    cvg_dest_ = (command >> 8) & 0x03;
    color_on_cvg_ = get_bit(command, 7);
    image_read_enable_ = get_bit(command, 6);
    z_update_enable_ = get_bit(command, 5);
    z_compare_enable_ = get_bit(command, 4);
    antialiasing_enable_ = get_bit(command, 3);
    z_source_select_ = get_bit(command, 2);
    dither_alpha_enable_ = get_bit(command, 1);
    alpha_compare_enable_ = get_bit(command, 0);

}

void RDP::load_tlut(u64 command) {
    u16 sl = get_bits(command, 55, 44) >> 2;
    u16 sh = get_bits(command, 23, 12) >> 2;

    for (u16 index = sl; index <= sh; index++) {
        u32 rdram_addr = texture_image_.addr + index * 2;
        u32 tmem_addr = TLUT_BASE_ADDRESS + index * 8;
        u16 color = rdram_.read_memory<u16>(rdram_addr);
        tmem_[(tmem_addr + 0) % 4096] = (color >> 8) & 0xFF;
        tmem_[(tmem_addr + 1) % 4096] = color & 0xFF;
    }
}

// TODO: Parse SL/TL/SH/TH and implement clamping in process_tmem_coordinates
void RDP::set_tile_size(u64 command) {}

void RDP::load_block(u64 command) {
    u16 upper_left_s = get_bits(command, 55, 44);
    u16 upper_left_t = get_bits(command, 43, 32);
    u8 tile_index = get_bits(command, 26, 25);
    u16 number_of_texels_to_load = get_bits(command, 23, 12) + 1;
    u16 dxt = get_bits(command, 11, 0);

    u32 bpp = bytes_per_pixel(texture_image_.size);
    u32 total_bytes = number_of_texels_to_load * bpp;

    total_bytes = std::min(total_bytes, static_cast<u32>(4096 - tiles_[tile_index].address));

    u32 tmem_addr = tiles_[tile_index].address;
    for (u16 i = 0; i < total_bytes; i++) {
        tmem_[tmem_addr + i] = rdram_.read_memory<u8>(texture_image_.addr + i);
    }
}

void RDP::load_tile(u64 command) {
    u16 upper_left_s = get_bits(command, 55, 44) >> 2;
    u16 upper_left_t = get_bits(command, 43, 32) >> 2;
    u8 tile_index = get_bits(command, 26, 24);
    u16 lower_right_s = get_bits(command, 23, 12) >> 2;
    u16 lower_right_t = get_bits(command, 11, 0) >> 2;

    u32 bpp = bytes_per_pixel(texture_image_.size);
    u32 tmem_line_stride = tiles_[tile_index].line_bytes;

    for (u16 t = upper_left_t; t <= lower_right_t; t++) {
        u32 row_tmem_base = tiles_[tile_index].address + (t - upper_left_t) * tmem_line_stride;
        for (u16 s = upper_left_s; s <= lower_right_s; s++) {
            u32 rdram_addr = texture_image_.addr + (t * texture_image_.width + s) * bpp;
            u32 tmem_addr = row_tmem_base + (s - upper_left_s) * bpp;
            for (u16 i = 0; i < bpp; i++) {
                tmem_[(tmem_addr + i) % 4096] = rdram_.read_memory<u8>(rdram_addr + i);
            }
        }
    }
    
}

void RDP::set_tile(u64 command) {
    int index = get_bits(command, 26, 24);

    tile_index_ = index;

    tiles_[index].format = static_cast<Format>(get_bits(command, 55, 53));
    tiles_[index].size = static_cast<Size>(get_bits(command, 52, 51));
    tiles_[index].line = get_bits(command, 49, 41);
    tiles_[index].line_bytes = tiles_[index].line * 8 * (tiles_[index].size == Size::SIZE_32B ? 2 : 1);
    tiles_[index].address = get_bits(command, 40, 32) << 3;
    tiles_[index].palette = get_bits(command, 23, 20);
    tiles_[index].clamp_t = get_bit(command, 19);
    tiles_[index].mirror_t = get_bit(command, 18);
    tiles_[index].mask_t = get_bits(command, 17, 14);
    tiles_[index].shift_t = get_bits(command, 13, 10);
    tiles_[index].clamp_s = get_bit(command, 9);
    tiles_[index].mirror_s = get_bit(command, 8);
    tiles_[index].mask_s = get_bits(command, 7, 4);
    tiles_[index].shift_s = get_bits(command, 3, 0);
}

// Fill_Rectangle (0x36) - fill rect directly to framebuffer
void RDP::fill_rectangle(u64 command) {
    // Coordinates are 10.2 fixed point, >> 2 for integer pixels
    u16 right  = get_bits(command, 55, 44) >> 2;
    u16 bottom = get_bits(command, 43, 32) >> 2;
    u16 left   = get_bits(command, 23, 12) >> 2;
    u16 top    = get_bits(command, 11, 0) >> 2;

    scissor_.clip(left, top, right, bottom);

    // Bytes per pixel based on framebuffer size
    switch (cycle_type_) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            copy_rectangle(right, bottom, left, top);
            break;
        case 3:
            fill_rectangle(right, bottom, left, top);
            break;
        default:
            throw std::runtime_error("Invalid cycle type: " + std::to_string(cycle_type_));
    }
}

void RDP::copy_rectangle(u16 right, u16 bottom, u16 left, u16 top) {
    float tex_bpp = bytes_per_pixel(texture_image_.size);
    u32 fb_bpp = bytes_per_pixel(color_image_.size);

    for (u16 y = top; y <= bottom; y++) {
        for (u16 x = left; x <= right; x++) {
            u32 tmem_addr = tiles_[tile_index_].address + ((y - top) * texture_image_.width + (x - left)) * tex_bpp;
            u32 fb_addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;

            Color pixel = fetch_pixel_tmem(tmem_addr, tiles_[tile_index_].size, tiles_[tile_index_].format, ((y - top) * texture_image_.width + (x - left)) & 1, tiles_[tile_index_].palette);
            write_pixel_framebuffer(fb_addr, pixel);
        }
    }
}

void RDP::fill_rectangle(u16 right, u16 bottom, u16 left, u16 top) {
    u32 fb_bpp = bytes_per_pixel(color_image_.size);

    for (u16 y = top; y <= bottom; y++) {
        for (u16 x = left; x <= right; x++) {
            u32 addr = color_image_.addr + (y * color_image_.width + x) * fb_bpp;
            switch (color_image_.size) {
                case Size::SIZE_16B: {
                    u16 color = (x & 1) ? (fill_color_ & 0xFFFF) : (fill_color_ >> 16);
                    rdram_.write_memory<u16>(addr, color);
                    break;
                }
                case Size::SIZE_32B: {
                    rdram_.write_memory<u32>(addr, fill_color_);
                    break;
                }
                default:
                    throw std::runtime_error("Invalid color image size: " + std::to_string(static_cast<u8>(color_image_.size)));
            }
        }
    }
}

// Set_Fill_Color (0x37) - fill color value
void RDP::set_fill_color(u64 command) {
    fill_color_ = get_bits(command, 31, 0);
}

void RDP::set_fog_color(u64 command) {
    color_combiner_.set_fog_color(command);
}

void RDP::set_blend_color(u64 command) {
    color_combiner_.set_blend_color(command);
}

void RDP::set_primitive_color(u64 command) {
    color_combiner_.set_primitive_color(command);
}

void RDP::set_environment_color(u64 command) {
    color_combiner_.set_environment_color(command);
}

// Set_Combine_Mode (0x3C) - color combiner input selectors (both cycles parsed)
// TODO: Implement remaining combiner inputs (COMBINED, TEXEL1, KEY_CENTER, LOD_FRACTION, K4/K5)
void RDP::set_combine_mode(u64 command) {
   color_combiner_.set_combine_mode(command);
}

void RDP::set_texture_image(u64 command) {
    texture_image_.format = static_cast<Format>(get_bits(command, 55, 53));
    texture_image_.size = static_cast<Size>(get_bits(command, 52, 51));
    texture_image_.width = get_bits(command, 41, 32) + 1;
    texture_image_.addr = get_bits(command, 24, 0);
}

// TODO: Store depth buffer RDRAM address for Z-buffer operations
void RDP::set_depth_image(u64 command) {}

// Set_Color_Image (0x3F) - framebuffer address/format/width
void RDP::set_color_image(u64 command) {
    color_image_.format = static_cast<Format>(get_bits(command, 55, 53));
    color_image_.size = static_cast<Size>(get_bits(command, 52, 51));
    color_image_.width = get_bits(command, 41, 32) + 1;
    color_image_.addr = get_bits(command, 24, 0);
}

template u8 RDP::read<u8>(u32) const;
template u16 RDP::read<u16>(u32) const;
template u32 RDP::read<u32>(u32) const;
template u64 RDP::read<u64>(u32) const;
template void RDP::write<u8>(u32, u8);
template void RDP::write<u16>(u32, u16);
template void RDP::write<u32>(u32, u32);
template void RDP::write<u64>(u32, u64);

template u16 RDP::read_tmem<u16>(u32) const;
template u32 RDP::read_tmem<u32>(u32) const;

} // namespace n64::rdp
