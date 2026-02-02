#pragma once

#include "../../utils/types.hpp"

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

// VI_CTRL (0x04400000)
union VICtrl {
    u32 raw;
    struct {
        u32 type : 2;                    // 0-1: Pixel format (0=blank, 2=16bit, 3=32bit)
        u32 gamma_dither_enable : 1;     // 2
        u32 gamma_enable : 1;            // 3
        u32 divot_enable : 1;            // 4
        u32 vbus_clock_enable : 1;       // 5
        u32 serrate : 1;                 // 6: Interlace
        u32 test_mode : 1;               // 7
        u32 aa_mode : 2;                 // 8-9: Anti-aliasing mode
        u32 : 1;                         // 10: reserved
        u32 kill_we : 1;                 // 11
        u32 pixel_advance : 4;           // 12-15
        u32 dedither_filter_enable : 1;  // 16
        u32 : 15;                        // 17-31: unused
    };
};

// VI_ORIGIN (0x04400004)
union VIOrigin {
    u32 raw;
    struct {
        u32 origin : 24;  // 0-23: RDRAM base address of frame buffer
        u32 : 8;          // 24-31: unused
    };
};

// VI_WIDTH (0x04400008)
union VIWidth {
    u32 raw;
    struct {
        u32 width : 12;   // 0-11: Frame buffer line width in pixels
        u32 : 20;         // 12-31: unused
    };
};

// VI_V_INTR (0x0440000C)
union VIVIntr {
    u32 raw;
    struct {
        u32 v_intr : 10;  // 0-9: Half-line to trigger VI interrupt
        u32 : 22;         // 10-31: unused
    };
};

// VI_V_CURRENT (0x04400010)
union VIVCurrent {
    u32 raw;
    struct {
        u32 field : 1;      // 0: Field number (0 or 1 in interlaced modes)
        u32 v_current : 9;  // 1-9: Current half-line
        u32 : 22;           // 10-31: unused
    };
};

// VI_BURST (0x04400014)
union VIBurst {
    u32 raw;
    struct {
        u32 hsync_width : 8;   // 0-7: Horizontal sync width in pixels
        u32 burst_width : 8;   // 8-15: Color burst width in pixels
        u32 vsync_height : 4;  // 16-19: One less than vertical sync duration in half-lines
        u32 burst_start : 10;  // 20-29: Start of color burst in pixels from H-sync
        u32 : 2;               // 30-31: unused
    };
};

// VI_V_TOTAL (0x04400018)
union VIVTotal {
    u32 raw;
    struct {
        u32 v_total : 10;  // 0-9: One less than total number of half-lines
        u32 : 22;          // 10-31: unused
    };
};

// VI_H_TOTAL (0x0440001C)
union VIHTotal {
    u32 raw;
    struct {
        u32 h_total : 12;  // 0-11: One less than total scanline length in 1/4 pixels
        u32 : 4;           // 12-15: unused
        u32 leap : 5;      // 16-20: 5-bit leap pattern (PAL only)
        u32 : 11;          // 21-31: unused
    };
};

// VI_H_TOTAL_LEAP (0x04400020)
union VIHTotalLeap {
    u32 raw;
    struct {
        u32 leap_b : 12;  // 0-11: Special scanline length when LEAP=1
        u32 : 4;          // 12-15: unused
        u32 leap_a : 12;  // 16-27: Special scanline length when LEAP=0
        u32 : 4;          // 28-31: unused
    };
};

// VI_H_VIDEO (0x04400024)
union VIHVideo {
    u32 raw;
    struct {
        u32 h_end : 10;    // 0-9: End of active video in screen pixels
        u32 : 6;           // 10-15: unused
        u32 h_start : 10;  // 16-25: Start of active video in screen pixels
        u32 : 6;           // 26-31: unused
    };
};

// VI_V_VIDEO (0x04400028)
union VIVVideo {
    u32 raw;
    struct {
        u32 v_end : 10;    // 0-9: End of active video in half-lines
        u32 : 6;           // 10-15: unused
        u32 v_start : 10;  // 16-25: Start of active video in half-lines
        u32 : 6;           // 26-31: unused
    };
};

// VI_V_BURST (0x0440002C)
union VIVBurst {
    u32 raw;
    struct {
        u32 v_burst_end : 10;    // 0-9: End of color burst enable in half-lines
        u32 : 6;                 // 10-15: unused
        u32 v_burst_start : 10;  // 16-25: Start of color burst enable in half-lines
        u32 : 6;                 // 26-31: unused
    };
};

// VI_X_SCALE (0x04400030)
union VIXScale {
    u32 raw;
    struct {
        u32 x_scale : 12;   // 0-11: 1/horizontal scale factor (2.10 fixed point)
        u32 : 4;            // 12-15: unused
        u32 x_offset : 12;  // 16-27: Horizontal subpixel offset (2.10 fixed point)
        u32 : 4;            // 28-31: unused
    };
};

// VI_Y_SCALE (0x04400034)
union VIYScale {
    u32 raw;
    struct {
        u32 y_scale : 12;   // 0-11: 1/vertical scale factor (2.10 fixed point)
        u32 : 4;            // 12-15: unused
        u32 y_offset : 10;  // 16-25: Vertical subpixel offset (0.10 fixed point)
        u32 : 2;            // 26-27: unused (holds state but ignored)
        u32 : 4;            // 28-31: unused
    };
};

// VI_TEST_ADDR (0x04400038)
union VITestAddr {
    u32 raw;
    struct {
        u32 test_addr : 7;  // 0-6: Line buffer word address
        u32 : 25;           // 7-31: unused
    };
};

// VI_STAGED_DATA (0x0440003C)
union VIStagedData {
    u32 raw;
    struct {
        u32 staged_data : 32;  // 0-31: Line buffer data
    };
};

} // namespace n64::interfaces
