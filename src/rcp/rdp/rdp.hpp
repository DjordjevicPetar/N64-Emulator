#pragma once

#include "../../utils/types.hpp"
#include "rdp_registers.hpp"
#include <array>
#include <vector>
#include "color_combiner.hpp"
#include "blender.hpp"
#include "fixed_point_float.hpp"

namespace n64::memory {
class RDRAM;
}

namespace n64::interfaces {
class MI;
}

namespace n64::rdp {

constexpr u32 TLUT_BASE_ADDRESS = 0x800;

struct Rectangle {
    FixedPointFloat left;
    FixedPointFloat top;
    FixedPointFloat right;
    FixedPointFloat bottom;

    Rectangle(u64 command) {
        right  = FixedPointFloat(get_bits(command, 55, 46), get_bits(command, 45, 44), 10, 2, false);
        bottom = FixedPointFloat(get_bits(command, 43, 34), get_bits(command, 33, 32), 10, 2, false);
        left   = FixedPointFloat(get_bits(command, 23, 14), get_bits(command, 13, 12), 10, 2, false);
        top    = FixedPointFloat(get_bits(command, 11, 2), get_bits(command, 1, 0), 10, 2, false);
    }

    Rectangle() : left(0), top(0), right(0), bottom(0) {}
};

struct Scissor {
    Rectangle scissor_rect;
    bool interlace_enable = false;
    bool odd_even = false;

    Scissor() = default;

    Scissor(u64 command) {
        scissor_rect.left   = FixedPointFloat(get_bits(command, 55, 46), get_bits(command, 45, 44), 10, 2, false);
        scissor_rect.top    = FixedPointFloat(get_bits(command, 43, 34), get_bits(command, 33, 32), 10, 2, false);
        interlace_enable = get_bit(command, 25);
        odd_even = get_bit(command, 24);
        scissor_rect.right  = FixedPointFloat(get_bits(command, 23, 14), get_bits(command, 13, 12), 10, 2, false);
        scissor_rect.bottom = FixedPointFloat(get_bits(command, 11, 2), get_bits(command, 1, 0), 10, 2, false);
    }

    void clip(Rectangle& rect) {
        rect.left   = std::max(rect.left,   scissor_rect.left);
        rect.top    = std::max(rect.top,    scissor_rect.top);
        rect.right  = std::min(rect.right,  scissor_rect.right);
        rect.bottom = std::min(rect.bottom, scissor_rect.bottom);
    }
};

struct Image {
    u32 addr;
    u16 width;
    Format format;
    Size size;
};

struct Tile {
    Format format;
    Size size;
    u16 line;
    u16 line_bytes;
    u16 address;
    u8 palette;
    bool clamp_t;
    bool mirror_t;
    u8 mask_t;
    u8 shift_t;
    bool clamp_s;
    bool mirror_s;
    u8 mask_s;
    u8 shift_s;
    u16 upper_left_s;
    u16 upper_left_t;
    u16 lower_right_s;
    u16 lower_right_t;
};

class RDP {
public:
    RDP(memory::RDRAM& rdram, n64::interfaces::MI& mi);
    ~RDP();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

    void process_passed_cycles(u32 cycles);

    // Accessors
    [[nodiscard]] const DPCStatus& status() const { return status_; }

private:
    using CommandHandler = u32 (RDP::*)(u64);


    // Command handlers (return cycle cost)
    u32 nop(u64 command);
    u32 triangle(u64 command);
    u32 texture_rectangle(u64 command);
    u32 texture_rectangle_flip(u64 command);
    u32 sync_load(u64 command);
    u32 sync_pipe(u64 command);
    u32 sync_tile(u64 command);
    u32 sync_full(u64 command);
    u32 set_key_gb(u64 command);
    u32 set_key_r(u64 command);
    u32 set_convert(u64 command);
    u32 set_scissor(u64 command);
    u32 set_primitive_depth(u64 command);
    u32 set_other_modes(u64 command);
    u32 load_tlut(u64 command);
    u32 set_tile_size(u64 command);
    u32 load_block(u64 command);
    u32 load_tile(u64 command);
    u32 set_tile(u64 command);
    u32 fill_rectangle(u64 command);
    u32 copy_rectangle(const Rectangle& copy_rect);
    u32 fill_rectangle(const Rectangle& fill_rect);
    u32 set_fill_color(u64 command);
    u32 set_fog_color(u64 command);
    u32 set_blend_color(u64 command);
    u32 set_primitive_color(u64 command);
    u32 set_environment_color(u64 command);
    u32 set_combine_mode(u64 command);
    u32 set_texture_image(u64 command);
    u32 set_depth_image(u64 command);
    u32 set_color_image(u64 command);

    void process_command_list();

    // Helper functions
    [[nodiscard]] float bytes_per_pixel(Size size) const;

    template<typename T>
    [[nodiscard]] T read_tmem(u32 addr) const;

    [[nodiscard]] Color fetch_pixel_tmem(u32 addr, Size size, Format format, bool odd_texel, u8 palette = 0) const;
    void write_pixel_framebuffer(u32 addr, const Color& color);
    [[nodiscard]] Color read_pixel_framebuffer(u32 addr) const;
    bool is_pixel_transparent(const Color& color) const;
    void process_tmem_coordinates(s32& coord, u8 shift, u8 mask, bool mirror, bool clamp, u16 lower_limit, u16 upper_limit) const;
    void process_pixel(s32 x, s32 y, u8 tile_index,
                       s32 tex_s, s32 tex_t,
                       const Color& shade, s32 z_depth,
                       u8 pixel_cvg,
                       bool has_texture, bool has_shade,
                       int& pix_log_cnt);

    void apply_alpha_dither(s32 x, s32 y, Color& color);
    void apply_rgb_dither(s32 x, s32 y, Color& color);

    memory::RDRAM& rdram_;
    n64::interfaces::MI& mi_;

    // DPC (Command) registers
    DPCStart start_;
    DPCEnd end_;
    DPCCurrent current_;
    DPCStatus status_;
    DPCClock clock_;
    DPCBufBusy buf_busy_;
    DPCPipeBusy pipe_busy_;
    DPCTmemBusy tmem_busy_;

    // DPS (Span) registers
    DPSTbist tbist_;
    DPSTestMode test_mode_;
    DPSBuftestAddr buftest_addr_;
    DPSBuftestData buftest_data_;

    // Command dispatch table (indexed by command ID, bits 56-61)
    std::array<CommandHandler, 64> command_table_;

    // Framebuffer state (from Set_Color_Image)
    Image color_image_;

    // Scissor state (from Set_Scissor)
    Scissor scissor_;
    bool scissor_enable_ = false;

    // Fill color (from Set_Fill_Color)
    u32 fill_color_ = 0;

    // Tile state (from Set_Tile)
    std::array<Tile, 8> tiles_;
    u8 tile_index_ = 0;

    // Other modes state (from Set_Other_Modes)
    bool atomic_prim_ = false;
    u8 cycle_type_ = 0;
    bool perspective_texture_enable_ = false;
    bool detail_textures_enable_ = false;
    bool sharpen_textures_enable_ = false;
    bool texture_lod_enable_ = false;
    bool tlut_enable_ = false;
    Format tlut_type_ = Format::FORMAT_RGB;
    u8 sample_type_ = 0;
    bool mid_texel_ = false;
    bool bi_lerp_0_ = false;
    bool bi_lerp_1_ = false;
    bool convert_one_ = false;
    bool key_enable_ = false;
    u8 z_mode_ = 0;
    bool image_read_enable_ = false;
    bool antialiasing_enable_ = false;
    bool alpha_compare_enable_ = false;

    // Alpha coverage state
    bool apply_alpha_coverage(u8 pixel_cvg, u32 cvg_index, Color& color);
    u8 compute_pixel_cvg(s32 x, FixedPointFloat x_left, FixedPointFloat x_right) const;
    u8 alpha_cvg_select_ = 0;
    bool cvg_x_alpha_ = false;
    bool color_on_cvg_ = false;
    u8 cvg_dest_ = 0;
    std::vector<u8> cvg_buffer_ = std::vector<u8>(640 * 480, 7);

    // Z-buffer state
    bool z_update_enable_ = false;
    bool z_compare_enable_ = false;
    bool z_source_select_ = false;
    u32 z_buffer_addr_ = 0;
    u16 z_prim_depth_ = 0;
    u16 dz_prim_depth_ = 0;

    // Dither state and utilitires
    u8 rgb_dither_sel_ = 0;
    u8 alpha_dither_sel_ = 0;
    bool dither_alpha_enable_ = false;
    std::array<std::array<u8, 4>, 4> bayer_matrix_ = {{
        {0, 4, 1, 5},
        {4, 0, 5, 1},
        {3, 7, 2, 6},
        {7, 3, 6, 2}
    }};
    std::array<std::array<u8, 4>, 4> magic_square_matrix_ = {{
        {0, 6, 1, 7},
        {4, 2, 5, 3},
        {3, 5, 2, 4},
        {7, 1, 6, 0}
    }};

    // Texture state (from Set_Texture_Image)
    Image texture_image_;

    // Color combiner state (from Set_Combine_Mode)
    ColorCombiner color_combiner_;

    // Blender state (from Set_Blender_Mode)
    Blender blender_;

    // Texture memory
    std::array<u8, 4096> tmem_;
};

} // namespace n64::rdp
