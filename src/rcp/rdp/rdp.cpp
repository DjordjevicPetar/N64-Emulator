#include "rdp.hpp"
#include "rdp_log.hpp"
#include "../../memory/rdram.hpp"
#include "../../interfaces/mi.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace n64::rdp {

// RDP implementation roadmap:
//
// Phase 1  - Command processing: DONE
// Phase 2  - Basic rendering: DONE (Fill_Rectangle, Set_Color_Image, Set_Scissor)
// Phase 3  - Copy mode textures: DONE (Load_Block, Load_Tile, Texture_Rectangle copy)
// Phase 4  - 1-cycle mode: DONE (Color combiner, Texture_Rectangle/Flip, shift/mask/mirror)
// Phase 5  - Color registers: DONE (Primitive, Environment, Fog, Blend colors)
// Phase 6  - Texture formats: DONE (RGBA 16b/32b, IA 16b/8b/4b, I 8b/4b, CI 4b/8b + TLUT palettes)
// Phase 7  - YUV + Set_Tile_Size: DONE (UYVY decode, K0-K5 convert, Set_Tile_Size clamping)
// Phase 8  - Triangles & Lines: DONE (Edge walking, Gouraud shading, S/T/W interpolation, sub-span correction)
// Phase 9  - Z-buffer: DONE (Depth compare/update, Set_Depth_Image, Set_Primitive_Depth, Z source select)
// Phase 10 - Fixed-point refactor: DONE (FixedPointFloat S.15.16 class used throughout)
// Phase 11 - 2-cycle mode & combiner: DONE (COMBINED, TEXEL1, LOD_FRAC, PRIM_LOD_FRAC, KEY_CENTER, KEY_SCALE)
// Phase 12 - Alpha pipeline: DONE (Alpha compare, alpha coverage, cvg_dest modes, chroma key)
// Phase 13 - Dithering: DONE (RGB + alpha dither with Bayer/magic square/noise matrices)
//
// Remaining work:
// TODO: Combiner 9-bit overflow precision (special_9bit_clamptable, inter-cycle 9-bit passing)
// TODO: Bilinear texture filtering (sample_type, bi_lerp_0/1, mid_texel)
// TODO: Texture LOD / detail / sharpen (lod_fraction calculation, tile level selection)
// TODO: Anti-aliasing (proper sub-pixel coverage, blender AA integration)
// TODO: Fog blending (shade alpha as fog factor -- may already work via blender wiring)
// TODO: DXT row-swapping in load_block (odd-row byte-swap for dual-bank TMEM)
// TODO: Framebuffer 4b/8b write/read support
// TODO: Sub-pixel coverage (4x4 sub-pixel grid instead of current heuristic)
// TODO: XBUS mode (commands from RSP DMEM)
// TODO: Cycle-accurate RDP timing

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
            if (status_.start_pending) {
                current_.raw = start_.raw;
                status_.start_pending = 0;
            }
            process_command_list();
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

void RDP::process_command_list() {
#ifdef RDP_LOG
    static constexpr const char* cmd_names[64] = {
        "Nop","Nop","Nop","Nop","Nop","Nop","Nop","Nop",
        "Fill_Tri","Fill_ZB_Tri","Tex_Tri","Tex_ZB_Tri","Shade_Tri","Shade_ZB_Tri","ShTx_Tri","ShTxZB_Tri",
        "Nop","Nop","Nop","Nop","Nop","Nop","Nop","Nop",
        "Nop","Nop","Nop","Nop","Nop","Nop","Nop","Nop",
        "Nop","Nop","Nop","Nop","Tex_Rect","Tex_Rect_Flip","Sync_Load","Sync_Pipe",
        "Sync_Tile","Sync_Full","Set_Key_GB","Set_Key_R","Set_Convert","Set_Scissor","Set_PrimDepth","Set_Other_Modes",
        "Load_TLUT","Nop","Set_Tile_Size","Load_Block","Load_Tile","Set_Tile","Fill_Rect","Set_Fill_Color",
        "Set_Fog_Color","Set_Blend_Color","Set_Prim_Color","Set_Env_Color","Set_Combine","Set_Tex_Image","Set_Z_Image","Set_Color_Image",
    };
#endif
    while (current_.raw < end_.raw) {
        u64 command = rdram_.read_memory<u64>(current_.raw);
        u8 command_id = (command >> 56) & 0x3F;
        RDP_LOG_CMD("0x%02X %s raw=%016llX", command_id, cmd_names[command_id], (unsigned long long)command);
        current_.raw += 8;
        (this->*command_table_[command_id])(command);
    }
    status_.dma_busy = 0;
}

void RDP::process_passed_cycles(u32 cycles) {
    (void)cycles;
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

u32 RDP::set_key_gb(u64 command) {
    color_combiner_.set_key_gb(command);
    return 8;
}

u32 RDP::set_key_r(u64 command) {
    color_combiner_.set_key_r(command);
    return 8;
}

u32 RDP::set_convert(u64 command) {
    color_combiner_.set_yuv_constants(command);
    return 8;
}

u32 RDP::set_scissor(u64 command) {
    scissor_ = Scissor(command);
    scissor_enable_ = true;
    RDP_LOG_STATE("scissor: (%d,%d)-(%d,%d) interlace=%d odd=%d",
        scissor_.scissor_rect.left.integer(), scissor_.scissor_rect.top.integer(),
        scissor_.scissor_rect.right.integer(), scissor_.scissor_rect.bottom.integer(),
        scissor_.interlace_enable, scissor_.odd_even);
    return 8;
}

u32 RDP::set_primitive_depth(u64 command) { 
    z_prim_depth_ = get_bits(command, 31, 16);
    dz_prim_depth_ = get_bits(command, 15, 0);
    RDP_LOG_STATE("prim_depth: z=%u dz=%u", z_prim_depth_, dz_prim_depth_);
    return 8;
}

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
    color_combiner_.set_key_enable(get_bit(command, 40));
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
    static constexpr const char* cycle_names[] = {"1cycle", "2cycle", "copy", "fill"};
    RDP_LOG_STATE("other_modes: cycle=%s persp=%d lod=%d tlut=%d sample=%s bi0=%d bi1=%d conv1=%d key=%d "
        "rgb_dith=%u a_dith=%u force_blend=%d a_cvg_sel=%d cvg_x_a=%d z_mode=%u cvg_dst=%u "
        "z_upd=%d z_cmp=%d aa=%d z_src=%d a_cmp=%d",
        cycle_names[cycle_type_], perspective_texture_enable_, texture_lod_enable_, tlut_enable_,
        sample_type_ ? "2x2" : "1x1", bi_lerp_0_, bi_lerp_1_, convert_one_,
        get_bit(command, 40), rgb_dither_sel_, alpha_dither_sel_,
        get_bit(command, 14), alpha_cvg_select_, cvg_x_alpha_, z_mode_, cvg_dest_,
        z_update_enable_, z_compare_enable_, antialiasing_enable_, z_source_select_, alpha_compare_enable_);
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
    RDP_LOG_STATE("tile_size[%u]: uls=%u ult=%u lrs=%u lrt=%u", tile_index, sl, tl, sh, th);
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
    static constexpr const char* fmt_names[] = {"RGBA","YUV","CI","IA","I","?","?","?"};
    static constexpr const char* size_names[] = {"4b","8b","16b","32b"};
    RDP_LOG_STATE("tile[%d]: fmt=%s size=%s line=%u addr=0x%03X pal=%u "
        "mask_s=%u mask_t=%u mir_s=%d mir_t=%d clamp_s=%d clamp_t=%d shift_s=%u shift_t=%u",
        index, fmt_names[(u8)tiles_[index].format], size_names[(u8)tiles_[index].size],
        tiles_[index].line, tiles_[index].address, tiles_[index].palette,
        tiles_[index].mask_s, tiles_[index].mask_t,
        tiles_[index].mirror_s, tiles_[index].mirror_t,
        tiles_[index].clamp_s, tiles_[index].clamp_t,
        tiles_[index].shift_s, tiles_[index].shift_t);
    return 8;
}

u32 RDP::set_fill_color(u64 command) {
    fill_color_ = get_bits(command, 31, 0);
    RDP_LOG_STATE("fill_color: 0x%08X", fill_color_);
    return 8;
}

u32 RDP::set_fog_color(u64 command) {
    blender_.set_fog_color(command);
    RDP_LOG_STATE("fog_color: R=%u G=%u B=%u A=%u",
        (u32)get_bits(command, 31, 24), (u32)get_bits(command, 23, 16), (u32)get_bits(command, 15, 8), (u32)get_bits(command, 7, 0));
    return 8;
}

u32 RDP::set_blend_color(u64 command) {
    blender_.set_blend_color(command);
    RDP_LOG_STATE("blend_color: R=%u G=%u B=%u A=%u",
        (u32)get_bits(command, 31, 24), (u32)get_bits(command, 23, 16), (u32)get_bits(command, 15, 8), (u32)get_bits(command, 7, 0));
    return 8;
}

u32 RDP::set_primitive_color(u64 command) {
    color_combiner_.set_primitive_color(command);
    RDP_LOG_STATE("prim_color: R=%u G=%u B=%u A=%u min_level=%u lod_frac=%u",
        (u32)get_bits(command, 31, 24), (u32)get_bits(command, 23, 16), (u32)get_bits(command, 15, 8), (u32)get_bits(command, 7, 0),
        (u32)get_bits(command, 44, 40), (u32)get_bits(command, 39, 32));
    return 8;
}

u32 RDP::set_environment_color(u64 command) {
    color_combiner_.set_environment_color(command);
    RDP_LOG_STATE("env_color: R=%u G=%u B=%u A=%u",
        (u32)get_bits(command, 31, 24), (u32)get_bits(command, 23, 16), (u32)get_bits(command, 15, 8), (u32)get_bits(command, 7, 0));
    return 8;
}

u32 RDP::set_combine_mode(u64 command) {
    color_combiner_.set_combine_mode(command);
    RDP_LOG_STATE("combine: A_rgb0=%u B_rgb0=%u C_rgb0=%u D_rgb0=%u A_a0=%u B_a0=%u C_a0=%u D_a0=%u "
        "A_rgb1=%u B_rgb1=%u C_rgb1=%u D_rgb1=%u A_a1=%u B_a1=%u C_a1=%u D_a1=%u",
        (u32)get_bits(command, 55, 52), (u32)get_bits(command, 31, 28),
        (u32)get_bits(command, 51, 47), (u32)get_bits(command, 17, 15),
        (u32)get_bits(command, 46, 44), (u32)get_bits(command, 14, 12),
        (u32)get_bits(command, 43, 41), (u32)get_bits(command, 11, 9),
        (u32)get_bits(command, 40, 37), (u32)get_bits(command, 27, 24),
        (u32)get_bits(command, 36, 32), (u32)get_bits(command, 8, 6),
        (u32)get_bits(command, 23, 21), (u32)get_bits(command, 5, 3),
        (u32)get_bits(command, 20, 18), (u32)get_bits(command, 2, 0));
    return 8;
}

u32 RDP::set_texture_image(u64 command) {
    texture_image_.format = static_cast<Format>(get_bits(command, 55, 53));
    texture_image_.size = static_cast<Size>(get_bits(command, 52, 51));
    texture_image_.width = get_bits(command, 41, 32) + 1;
    texture_image_.addr = get_bits(command, 24, 0);
    {
        static constexpr const char* fn[] = {"RGBA","YUV","CI","IA","I","?","?","?"};
        static constexpr const char* sn[] = {"4b","8b","16b","32b"};
        RDP_LOG_STATE("tex_image: fmt=%s size=%s width=%u addr=0x%06X",
            fn[(u8)texture_image_.format], sn[(u8)texture_image_.size], texture_image_.width, texture_image_.addr);
    }
    return 8;
}

u32 RDP::set_depth_image(u64 command) { 
    z_buffer_addr_ = get_bits(command, 24, 0);
    RDP_LOG_STATE("z_image: addr=0x%06X", z_buffer_addr_);
    return 8;
}

u32 RDP::set_color_image(u64 command) {
    color_image_.format = static_cast<Format>(get_bits(command, 55, 53));
    color_image_.size = static_cast<Size>(get_bits(command, 52, 51));
    color_image_.width = get_bits(command, 41, 32) + 1;
    color_image_.addr = get_bits(command, 24, 0);
    {
        static constexpr const char* fn[] = {"RGBA","YUV","CI","IA","I","?","?","?"};
        static constexpr const char* sn[] = {"4b","8b","16b","32b"};
        RDP_LOG_STATE("color_image: fmt=%s size=%s width=%u addr=0x%06X",
            fn[(u8)color_image_.format], sn[(u8)color_image_.size], color_image_.width, color_image_.addr);
    }

    u32 needed = color_image_.width * 240;
    if (needed > cvg_buffer_.size()) {
        cvg_buffer_.resize(needed, 7);
    }
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
