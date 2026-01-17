#include "vi.hpp"
#include <stdexcept>
#include <string>

namespace n64::interfaces {

VI::VI(MI& mi)
    : mi_(mi)
    , ctrl_(0)
    , origin_(0)
    , width_(0)
    , v_intr_(0)
    , v_current_(0)
    , burst_(0)
    , v_total_(0)
    , h_total_(0)
    , h_total_leap_(0)
    , h_video_(0)
    , v_video_(0)
    , v_burst_(0)
    , x_scale_(0)
    , y_scale_(0)
    , test_addr_(0)
    , staged_data_(0)
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
    switch (address) {
        case VI_REGISTERS_ADDRESS::VI_CTRL:
            return ctrl_;
        case VI_REGISTERS_ADDRESS::VI_ORIGIN:
            return origin_;
        case VI_REGISTERS_ADDRESS::VI_WIDTH:
            return width_;
        case VI_REGISTERS_ADDRESS::VI_V_INTR:
            return v_intr_;
        case VI_REGISTERS_ADDRESS::VI_V_CURRENT:
            return v_current_;
        case VI_REGISTERS_ADDRESS::VI_BURST:
            return burst_;
        case VI_REGISTERS_ADDRESS::VI_V_TOTAL:
            return v_total_;
        case VI_REGISTERS_ADDRESS::VI_H_TOTAL:
            return h_total_;
        case VI_REGISTERS_ADDRESS::VI_H_TOTAL_LEAP:
            return h_total_leap_;
        case VI_REGISTERS_ADDRESS::VI_H_VIDEO:
            return h_video_;
        case VI_REGISTERS_ADDRESS::VI_V_VIDEO:
            return v_video_;
        case VI_REGISTERS_ADDRESS::VI_V_BURST:
            return v_burst_;
        case VI_REGISTERS_ADDRESS::VI_X_SCALE:
            return x_scale_;
        case VI_REGISTERS_ADDRESS::VI_Y_SCALE:
            return y_scale_;
        case VI_REGISTERS_ADDRESS::VI_TEST_ADDR:
            return test_addr_;
        case VI_REGISTERS_ADDRESS::VI_STAGED_DATA:
            return staged_data_;
        default:
            throw std::runtime_error("Invalid VI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

void VI::write_register(u32 address, u32 value) {
    switch (address) {
        case VI_REGISTERS_ADDRESS::VI_CTRL:
            ctrl_ = value & 0x0001FBFF;
            break;
        case VI_REGISTERS_ADDRESS::VI_ORIGIN:
            origin_ = value & 0x00FFFFFF;
            break;
        case VI_REGISTERS_ADDRESS::VI_WIDTH:
            width_ = value & 0x00000FFF;
            break;
        case VI_REGISTERS_ADDRESS::VI_V_INTR:
            v_intr_ = value & 0x000003FF;
            break;
        case VI_REGISTERS_ADDRESS::VI_V_CURRENT:
            v_current_ = value & 0x000003FF;
            break;
        case VI_REGISTERS_ADDRESS::VI_BURST:
            burst_ = value & 0x3FFFFFFF;
            break;
        case VI_REGISTERS_ADDRESS::VI_V_TOTAL:
            v_total_ = value & 0x000003FF;
            break;
        case VI_REGISTERS_ADDRESS::VI_H_TOTAL:
            h_total_ = value & 0x001F0FFF;
            break;
        case VI_REGISTERS_ADDRESS::VI_H_TOTAL_LEAP:
            h_total_leap_ = value & 0x0FFF0FFF;
            break;
        case VI_REGISTERS_ADDRESS::VI_H_VIDEO:
            h_video_ = value & 0x03FF03FF;
            break;
        case VI_REGISTERS_ADDRESS::VI_V_VIDEO:
            v_video_ = value & 0x03FF03FF;
            break;
        case VI_REGISTERS_ADDRESS::VI_V_BURST:
            v_burst_ = value & 0x03FF03FF;
            break;
        case VI_REGISTERS_ADDRESS::VI_X_SCALE:
            x_scale_ = value & 0x0FFF0FFF;
            break;
        case VI_REGISTERS_ADDRESS::VI_Y_SCALE:
            y_scale_ = value & 0x03FF0FFF;
            break;
        case VI_REGISTERS_ADDRESS::VI_TEST_ADDR:
            test_addr_ = value & 0x0000007F;
            break;
        case VI_REGISTERS_ADDRESS::VI_STAGED_DATA:
            staged_data_ = value;
            break;
        default:
            throw std::runtime_error("Invalid VI register address: " + std::to_string(static_cast<u32>(address)));
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
