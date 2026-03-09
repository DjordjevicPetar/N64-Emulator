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
// Phase 7 - YUV + Set_Tile_Size: DONE (UYVY decode, K0-K5 convert, Set_Tile_Size clamping)
//
// Phase 8 - Triangles (CURRENT):
// TODO: Implement edge walking rasterizer (Fill/Shade/Texture/Z triangle variants)
// TODO: Implement per-pixel S/T/W interpolation and perspective correction
// TODO: Implement Gouraud shading (per-vertex RGBA interpolation)
//
// Phase 9 - Blender & Z-buffer:
// TODO: Implement blender (alpha blending, fog, anti-aliasing, coverage)
// TODO: Implement Z-buffer read/write/compare + Set_Depth_Image + Set_Primitive_Depth
//
// Phase 10 - 2-cycle mode & combiner completeness:
// TODO: Implement 2-cycle mode (two combiner+blender passes per pixel)
// TODO: Implement COMBINED input (output of cycle 0 fed into cycle 1)
// TODO: Implement TEXEL1 input (second texture tile for multitexturing)
// TODO: Implement LOD_FRACTION, KEY_CENTER, KEY_SCALE combiner inputs
//
// Phase 11 - Accuracy & advanced:
// TODO: Implement bilinear texture filtering (sample_type)
// TODO: Implement LOD / detail / sharpen texture modes
// TODO: Implement coverage calculation and anti-aliasing
// TODO: Implement dithering (RGB and alpha)
// TODO: Implement XBUS mode (commands from RSP DMEM instead of RDRAM)
// TODO: Implement cycle-accurate RDP timing

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
                status_.dma_busy = 1;
            } else if (!status_.dma_busy) {
                // New transfer: no transfer running, start from START to END
                current_.raw = start_.raw;
                status_.start_pending = 0;
                status_.dma_busy = 1;
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

void RDP::process_passed_cycles(u32 cycles) {
    if (!status_.dma_busy) {
        cycle_accumulator_ = 0;
        return;
    }

    cycle_accumulator_ += cycles * (2.0f / 3.0f);

    while (current_.raw < end_.raw && cycle_accumulator_ > 0) {
        u64 command = rdram_.read_memory<u64>(current_.raw);
        u8 command_id = (command >> 56) & 0x3F;
        current_.raw += 8;
        u32 cost = (this->*command_table_[command_id])(command);
        cycle_accumulator_ -= cost;
    }

    if (current_.raw >= end_.raw) {
        status_.dma_busy = 0;

        if (status_.end_pending) {
            status_.end_pending = 0;
            current_.raw = start_.raw;
            status_.start_pending = 0;
            status_.dma_busy = 1;
        }
    }
}

// ============================================================================
// State-setting command handlers
// ============================================================================

u32 RDP::nop(u64 command) { return 8; }

u32 RDP::sync_load(u64 command) { return 8; }
u32 RDP::sync_pipe(u64 command) { return 8; }
u32 RDP::sync_tile(u64 command) { return 8; }

u32 RDP::sync_full(u64 command) {
    mi_.set_interrupt(interfaces::MI_INTERRUPT_BITS::MI_INTERRUPT_DP);
    return 8;
}

// TODO: Parse chroma key width/center/scale for green and blue channels
u32 RDP::set_key_gb(u64 command) { return 8; }
// TODO: Parse chroma key width/center/scale for red channel
u32 RDP::set_key_r(u64 command) { return 8; }

u32 RDP::set_convert(u64 command) {
    color_combiner_.set_yuv_constants(command);
    return 8;
}

u32 RDP::set_scissor(u64 command) {
    scissor_.upper_left_x = get_bits(command, 55, 44) >> 2;
    scissor_.upper_left_y = get_bits(command, 43, 32) >> 2;
    scissor_.interlace_enable = get_bit(command, 25);
    scissor_.odd_even = get_bit(command, 24);
    scissor_.lower_right_x = get_bits(command, 23, 12) >> 2;
    scissor_.lower_right_y = get_bits(command, 11, 0) >> 2;
    return 8;
}

// TODO: Parse z and deltaz for primitive Z-buffer source
u32 RDP::set_primitive_depth(u64 command) { return 8; }

u32 RDP::set_other_modes(u64 command) {
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
    // 35 - 32: Unused
    blender_.set_blender_input(get_bits(command, 31, 16));
    blender_.set_force_blend(get_bit(command, 14));
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
    return 8;
}

u32 RDP::set_tile_size(u64 command) {
    u16 sl = get_bits(command, 55, 44) >> 2;
    u16 tl = get_bits(command, 43, 32) >> 2;
    u16 sh = get_bits(command, 23, 12) >> 2;
    u16 th = get_bits(command, 11, 0) >> 2;
    u8 tile_index = get_bits(command, 26, 24);

    tiles_[tile_index].upper_left_s = sl;
    tiles_[tile_index].upper_left_t = tl;
    tiles_[tile_index].lower_right_s = sh;
    tiles_[tile_index].lower_right_t = th;
    return 8;
}

u32 RDP::set_tile(u64 command) {
    int index = get_bits(command, 26, 24);

    tile_index_ = index;

    tiles_[index].format = static_cast<Format>(get_bits(command, 55, 53));
    tiles_[index].size = static_cast<Size>(get_bits(command, 52, 51));
    tiles_[index].line = get_bits(command, 49, 41);
    bool dual_bank = tiles_[index].size == Size::SIZE_32B || tiles_[index].format == Format::FORMAT_YUV;
    tiles_[index].line_bytes = tiles_[index].line * 8 * (dual_bank ? 2 : 1);
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
    return 8;
}

u32 RDP::set_fill_color(u64 command) {
    fill_color_ = get_bits(command, 31, 0);
    return 8;
}

u32 RDP::set_fog_color(u64 command) {
    blender_.set_fog_color(command);
    return 8;
}

u32 RDP::set_blend_color(u64 command) {
    blender_.set_blend_color(command);
    return 8;
}

u32 RDP::set_primitive_color(u64 command) {
    color_combiner_.set_primitive_color(command);
    return 8;
}

u32 RDP::set_environment_color(u64 command) {
    color_combiner_.set_environment_color(command);
    return 8;
}

u32 RDP::set_combine_mode(u64 command) {
   color_combiner_.set_combine_mode(command);
   return 8;
}

u32 RDP::set_texture_image(u64 command) {
    texture_image_.format = static_cast<Format>(get_bits(command, 55, 53));
    texture_image_.size = static_cast<Size>(get_bits(command, 52, 51));
    texture_image_.width = get_bits(command, 41, 32) + 1;
    texture_image_.addr = get_bits(command, 24, 0);
    return 8;
}

// TODO: Store depth buffer RDRAM address for Z-buffer operations
u32 RDP::set_depth_image(u64 command) { return 8; }

u32 RDP::set_color_image(u64 command) {
    color_image_.format = static_cast<Format>(get_bits(command, 55, 53));
    color_image_.size = static_cast<Size>(get_bits(command, 52, 51));
    color_image_.width = get_bits(command, 41, 32) + 1;
    color_image_.addr = get_bits(command, 24, 0);
    return 8;
}

template u8 RDP::read<u8>(u32) const;
template u16 RDP::read<u16>(u32) const;
template u32 RDP::read<u32>(u32) const;
template u64 RDP::read<u64>(u32) const;
template void RDP::write<u8>(u32, u8);
template void RDP::write<u16>(u32, u16);
template void RDP::write<u32>(u32, u32);
template void RDP::write<u64>(u32, u64);

} // namespace n64::rdp
