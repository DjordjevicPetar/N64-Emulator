#pragma once

#include "../utils/types.hpp"
#include "mi.hpp"

namespace n64::interfaces {

enum VI_REGISTERS_ADDRESS : u32 {
    VI_CTRL = 0x04400000,
    VI_ORIGIN = 0x04400004,
    VI_WIDTH = 0x04400008,
    VI_V_INTR = 0x0440000C,
    VI_V_CURRENT = 0x04400010,
    VI_BURST = 0x04400014,
    VI_V_TOTAL = 0x04400018,
    VI_H_TOTAL = 0x0440001C,
    VI_H_TOTAL_LEAP = 0x04400020,
    VI_H_VIDEO = 0x04400024,
    VI_V_VIDEO = 0x04400028,
    VI_V_BURST = 0x0440002C,
    VI_X_SCALE = 0x04400030,
    VI_Y_SCALE = 0x04400034,
    VI_TEST_ADDR = 0x04400038,
    VI_STAGED_DATA = 0x0440003C
};

class VI {
public:
    VI(MI& mi);
    ~VI();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

private:
    MI& mi_;
    
    u32 ctrl_;
    u32 origin_;
    u32 width_;
    u32 v_intr_;
    u32 v_current_;
    u32 burst_;
    u32 v_total_;
    u32 h_total_;
    u32 h_total_leap_;
    u32 h_video_;
    u32 v_video_;
    u32 v_burst_;
    u32 x_scale_;
    u32 y_scale_;
    u32 test_addr_;
    u32 staged_data_;
};
}