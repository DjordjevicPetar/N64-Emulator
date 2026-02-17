#pragma once

#include "../../utils/types.hpp"
#include "rdp_registers.hpp"
#include <array>
#include "color_combiner.hpp"

namespace n64::memory {
class RDRAM;
}

namespace n64::interfaces {
class MI;
}

namespace n64::rdp {

constexpr u32 TLUT_BASE_ADDRESS = 0x800;

struct Scissor {
    u16 upper_left_x;
    u16 upper_left_y;
    u16 lower_right_x;
    u16 lower_right_y;
    bool interlace_enable;
    bool odd_even;

    void clip(u16& left, u16& top, u16& right, u16& bottom) {
        left   = std::max(left,   upper_left_x);
        top    = std::max(top,    upper_left_y);
        right  = std::min(right,  lower_right_x);
        bottom = std::min(bottom, lower_right_y);
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

    // Accessors
    [[nodiscard]] const DPCStatus& status() const { return status_; }

private:
    using CommandHandler = void (RDP::*)(u64);

    void run_commands();
    void execute_commands();

    // Command handlers
    void nop(u64 command);
    void triangle(u64 command);
    void texture_rectangle(u64 command);
    void texture_rectangle_flip(u64 command);
    void sync_load(u64 command);
    void sync_pipe(u64 command);
    void sync_tile(u64 command);
    void sync_full(u64 command);
    void set_key_gb(u64 command);
    void set_key_r(u64 command);
    void set_convert(u64 command);
    void set_scissor(u64 command);
    void set_primitive_depth(u64 command);
    void set_other_modes(u64 command);
    void load_tlut(u64 command);
    void set_tile_size(u64 command);
    void load_block(u64 command);
    void load_tile(u64 command);
    void set_tile(u64 command);
    void fill_rectangle(u64 command);
    void copy_rectangle(u16 right, u16 bottom, u16 left, u16 top);
    void fill_rectangle(u16 right, u16 bottom, u16 left, u16 top);
    void set_fill_color(u64 command);
    void set_fog_color(u64 command);
    void set_blend_color(u64 command);
    void set_primitive_color(u64 command);
    void set_environment_color(u64 command);
    void set_combine_mode(u64 command);
    void set_texture_image(u64 command);
    void set_depth_image(u64 command);
    void set_color_image(u64 command);

    // Helper functions
    [[nodiscard]] float bytes_per_pixel(Size size) const;

    template<typename T>
    [[nodiscard]] T read_tmem(u32 addr) const;

    [[nodiscard]] Color fetch_pixel_tmem(u32 addr, Size size, Format format, bool odd_texel, u8 palette = 0) const;
    void write_pixel_framebuffer(u32 addr, const Color& color);
    bool is_pixel_transparent(const Color& color) const;
    void process_tmem_coordinates(s32& coord, u8 shift, u8 mask, bool mirror) const;

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
    u8 rgb_dither_sel_ = 0;
    u8 alpha_dither_sel_ = 0;
    u8 bl_m1a_0_ = 0;
    u8 bl_m1a_1_ = 0;
    u8 bl_m1b_0_ = 0;
    u8 bl_m1b_1_ = 0;
    u8 bl_ma_0_ = 0;
    u8 bl_ma_1_ = 0;
    u8 bl_mb_0_ = 0;
    u8 bl_mb_1_ = 0;
    bool force_blend_ = false;
    u8 alpha_cvg_select_ = 0;
    bool cvg_x_alpha_ = false;
    u8 z_mode_ = 0;
    u8 cvg_dest_ = 0;
    bool color_on_cvg_ = false;
    bool image_read_enable_ = false;
    bool z_update_enable_ = false;
    bool z_compare_enable_ = false;
    bool antialiasing_enable_ = false;
    bool z_source_select_ = false;
    bool dither_alpha_enable_ = false;
    bool alpha_compare_enable_ = false;

    // Texture state (from Set_Texture_Image)
    Image texture_image_;

    // Color combiner state (from Set_Combine_Mode)
    ColorCombiner color_combiner_;

    // Texture memory
    std::array<u8, 4096> tmem_;
};

} // namespace n64::rdp
