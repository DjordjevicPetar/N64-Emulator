#include "vi.hpp"
#include <iostream>

namespace n64::interfaces {

VI::VI(MI& mi, memory::RDRAM& rdram)
    : mi_(mi)
    , renderer_(this, rdram)
    , cycles_counter_(0)
    , ctrl_{}
    , origin_{}
    , width_{}
    , v_intr_{}
    , v_current_{}
    , burst_{}
    , v_total_{}
    , h_total_{}
    , h_total_leap_{}
    , h_video_{}
    , v_video_{}
    , v_burst_{}
    , x_scale_{}
    , y_scale_{}
    , test_addr_{}
    , staged_data_{}
{
}

VI::~VI() {}

template<typename T>
T VI::read(u32 address) const {
    return read_register(address);
}

template<typename T>
void VI::write(u32 address, T value) {
    write_register(address, value);
}

u32 VI::read_register(u32 address) const {
    // NOTE: On real hardware, reading VI_V_CURRENT returns the current half-line being rendered
    switch (address) {
        case VI_CTRL:
            return ctrl_.raw;
        case VI_ORIGIN:
            return origin_.raw;
        case VI_WIDTH:
            return width_.raw;
        case VI_V_INTR:
            return v_intr_.raw;
        case VI_V_CURRENT:
            return v_current_.raw;
        case VI_BURST:
            return burst_.raw;
        case VI_V_TOTAL:
            return v_total_.raw;
        case VI_H_TOTAL:
            return h_total_.raw;
        case VI_H_TOTAL_LEAP:
            return h_total_leap_.raw;
        case VI_H_VIDEO:
            return h_video_.raw;
        case VI_V_VIDEO:
            return v_video_.raw;
        case VI_V_BURST:
            return v_burst_.raw;
        case VI_X_SCALE:
            return x_scale_.raw;
        case VI_Y_SCALE:
            return y_scale_.raw;
        case VI_TEST_ADDR:
            return test_addr_.raw;
        case VI_STAGED_DATA:
            return staged_data_.raw;
        default:
            return 0;
    }
}

void VI::write_register(u32 address, u32 value) {
    switch (address) {
        case VI_CTRL:
            fprintf(stderr, "[VI] CTRL = 0x%08X (type=%u)\n", value, value & 3);
            ctrl_.raw = value & 0x0001FBFF;
            break;
        case VI_ORIGIN:
            fprintf(stderr, "[VI] ORIGIN = 0x%08X\n", value);
            origin_.raw = value & 0x00FFFFFF;
            break;
        case VI_WIDTH:
            fprintf(stderr, "[VI] WIDTH = %u\n", value);
            width_.raw = value & 0x00000FFF;
            break;
        case VI_V_INTR:
            fprintf(stderr, "[VI] V_INTR = %u\n", value);
            v_intr_.raw = value & 0x000003FF;
            break;
        case VI_V_CURRENT:
            mi_.clear_interrupt(MI_INTERRUPT_VI);
            v_current_.raw = value & 0x000003FF;
            break;
        case VI_BURST:
            burst_.raw = value & 0x3FFFFFFF;
            break;
        case VI_V_TOTAL:
            fprintf(stderr, "[VI] V_TOTAL = %u\n", value & 0x3FF);
            v_total_.raw = value & 0x000003FF;
            break;
        case VI_H_TOTAL:
            fprintf(stderr, "[VI] H_TOTAL = %u\n", value & 0xFFF);
            h_total_.raw = value & 0x001F0FFF;
            break;
        case VI_H_TOTAL_LEAP:
            h_total_leap_.raw = value & 0x0FFF0FFF;
            break;
        case VI_H_VIDEO:
            h_video_.raw = value & 0x03FF03FF;
            break;
        case VI_V_VIDEO:
            v_video_.raw = value & 0x03FF03FF;
            break;
        case VI_V_BURST:
            v_burst_.raw = value & 0x03FF03FF;
            break;
        case VI_X_SCALE:
            x_scale_.raw = value & 0x0FFF0FFF;
            break;
        case VI_Y_SCALE:
            y_scale_.raw = value & 0x03FF0FFF;
            break;
        case VI_TEST_ADDR:
            test_addr_.raw = value & 0x0000007F;
            break;
        case VI_STAGED_DATA:
            staged_data_.raw = value;
            break;
        default:
            break;
    }
}

void VI::process_passed_cycles(u32 cycles) {
    // Don't process if VI isn't configured yet
    if (h_total_.h_total == 0 || v_total_.v_total == 0) {
        return;
    }
    
    cycles_counter_ += cycles;

    // h_total is in VI clock units (VI DAC runs at ~48.68 MHz for NTSC)
    // CPU runs at 93.75 MHz. h_total covers one full scan line.
    // v_current counts half-lines, so divide by 2.
    static constexpr double CPU_FREQ = 93750000.0;
    static constexpr double VI_FREQ  = 48681812.0;
    u64 cycles_to_render_half_line = static_cast<u64>((h_total_.h_total + 1) * CPU_FREQ / VI_FREQ / 2.0);

    
    if (cycles_to_render_half_line == 0) {
        cycles_to_render_half_line = 1;
    }

    u32 v_current_max = v_total_.v_total;
    
    while (cycles_counter_ >= cycles_to_render_half_line) {
        cycles_counter_ -= cycles_to_render_half_line;
        
        u32 old_v_current = v_current_.v_current;
        v_current_.v_current = (v_current_.v_current + 1) % (v_current_max + 1);
        
        // Field wrap: toggle field bit for interlaced modes
        if (v_current_.v_current < old_v_current) {
            if (ctrl_.serrate) {
                v_current_.v_current ^= 1;
            }
            if (v_video_.v_end == 0) {
                renderer_.render_frame();
            }
        }
        
        // Render at end of active display period (v_video.v_end).
        // This captures the framebuffer after the RDP finishes the current frame
        // but before the CPU starts the next frame's rendering at WaitScanline.
        if (v_video_.v_end > 0 && old_v_current < v_video_.v_end && v_current_.v_current >= v_video_.v_end) {
            renderer_.render_frame();
        }
        
        // VI interrupt fires when v_current matches v_intr
        if (v_current_.v_current == v_intr_.v_intr) {
            mi_.set_interrupt(MI_INTERRUPT_VI);
        }
    }
}

template u8 VI::read<u8>(u32) const;
template u16 VI::read<u16>(u32) const;
template u32 VI::read<u32>(u32) const;
template u64 VI::read<u64>(u32) const;
template void VI::write<u8>(u32, u8);
template void VI::write<u16>(u32, u16);
template void VI::write<u32>(u32, u32);
template void VI::write<u64>(u32, u64);

} // namespace n64::interfaces
