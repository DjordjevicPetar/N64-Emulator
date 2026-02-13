#include "vi.hpp"
#include <stdexcept>
#include <string>
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
    // TODO: Verify if reading VI_V_CURRENT should latch/update the value
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
            throw std::runtime_error("Invalid VI register address: " + std::to_string(address));
    }
}

void VI::write_register(u32 address, u32 value) {
    switch (address) {
        case VI_CTRL:
            ctrl_.raw = value & 0x0001FBFF;
            break;
        case VI_ORIGIN:
            origin_.raw = value & 0x00FFFFFF;
            break;
        case VI_WIDTH:
            width_.raw = value & 0x00000FFF;
            break;
        case VI_V_INTR:
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
            v_total_.raw = value & 0x000003FF;
            break;
        case VI_H_TOTAL:
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
            throw std::runtime_error("Invalid VI register address: " + std::to_string(address));
    }
}

void VI::process_passed_cycles(u32 cycles) {
    // Don't process if VI isn't configured yet
    if (h_total_.h_total == 0 || v_total_.v_total == 0) {
        return;
    }
    
    cycles_counter_ += cycles;
    // h_total is in 1/4 pixel units, dividing by 4 gives pixels per line
    u64 cycles_to_render_half_line = (h_total_.h_total + 1) / 4;
    
    // Safety check to prevent infinite loop
    if (cycles_to_render_half_line == 0) {
        cycles_to_render_half_line = 1;
    }

    // v_total is already in half-lines (525 for NTSC, 625 for PAL)
    // v_current counts half-lines from 0 to v_total
    u32 v_current_max = v_total_.v_total;
    
    while (cycles_counter_ >= cycles_to_render_half_line) {
        cycles_counter_ -= cycles_to_render_half_line;
        
        u32 old_v_current = v_current_.v_current;
        v_current_.v_current = (v_current_.v_current + 1) % (v_current_max + 1);
        
        // Detect frame boundary (wrap around)
        if (v_current_.v_current < old_v_current) {
            if (ctrl_.serrate) {
                v_current_.v_current ^= 1;  // Toggle field bit (bit 0)
            }
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
