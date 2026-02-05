#pragma once

#include "../../utils/types.hpp"

namespace n64::interfaces {

// PI Register Addresses
enum PI_REGISTERS_ADDRESS : u32 {
    PI_DRAM_ADDR    = 0x04600000,
    PI_CART_ADDR    = 0x04600004,
    PI_RD_LEN       = 0x04600008,
    PI_WR_LEN       = 0x0460000C,
    PI_STATUS       = 0x04600010,
    PI_BSD_DOM1_LAT = 0x04600014,
    PI_BSD_DOM1_PWD = 0x04600018,
    PI_BSD_DOM1_PGS = 0x0460001C,
    PI_BSD_DOM1_RLS = 0x04600020,
    PI_BSD_DOM2_LAT = 0x04600024,
    PI_BSD_DOM2_PWD = 0x04600028,
    PI_BSD_DOM2_PGS = 0x0460002C,
    PI_BSD_DOM2_RLS = 0x04600030,
};

// PI_DRAM_ADDR (0x04600000) - RDRAM address for DMA
union PIDramAddr {
    u32 raw;
    struct {
        u32 : 1;           // Bit 0 unused (2-byte aligned)
        u32 address : 23;  // Bits 1-23: RDRAM address
        u32 : 8;           // Bits 24-31 unused
    };
};

// PI_CART_ADDR (0x04600004) - Cartridge address for DMA
union PICartAddr {
    u32 raw;
    struct {
        u32 : 1;           // Bit 0 unused (2-byte aligned)
        u32 address : 31;  // Bits 1-31: Cartridge bus address
    };
};

// PI_RD_LEN (0x04600008) - Read length (Cart -> RDRAM)
// PI_WR_LEN (0x0460000C) - Write length (RDRAM -> Cart)
// Length is (value + 1) bytes
union PILen {
    u32 raw;
    struct {
        u32 length : 24;   // Bits 0-23: Transfer length - 1
        u32 : 8;           // Bits 24-31 unused
    };
};

// PI_STATUS (0x04600010)
// Read: status flags
// Write: control bits
union PIStatus {
    u32 raw;
    
    // For reading
    struct {
        u32 dma_busy : 1;     // Bit 0: DMA transfer in progress
        u32 io_busy : 1;      // Bit 1: IO access in progress
        u32 dma_error : 1;    // Bit 2: DMA error occurred
        u32 interrupt : 1;    // Bit 3: PI interrupt pending
        u32 : 28;             // Bits 4-31 unused
    };
    
    // For writing (control)
    struct {
        u32 reset_dma : 1;    // Bit 0: Reset DMA controller
        u32 clear_intr : 1;   // Bit 1: Clear PI interrupt
        u32 : 30;             // Bits 2-31 unused
    } write;
};

// PI_BSD_DOMx_LAT - Domain latency
union PIBsdLat {
    u32 raw;
    struct {
        u32 latency : 8;   // Bits 0-7: Latency cycles
        u32 : 24;          // Bits 8-31 unused
    };
};

// PI_BSD_DOMx_PWD - Domain pulse width
union PIBsdPwd {
    u32 raw;
    struct {
        u32 pulse_width : 8;  // Bits 0-7: Pulse width cycles
        u32 : 24;             // Bits 8-31 unused
    };
};

// PI_BSD_DOMx_PGS - Domain page size
// Page size = 2^(pgs + 2) bytes
union PIBsdPgs {
    u32 raw;
    struct {
        u32 page_size : 4;    // Bits 0-3: Page size exponent
        u32 : 28;             // Bits 4-31 unused
    };
    
    // Helper to get actual page size in bytes
    [[nodiscard]] constexpr u32 get_page_bytes() const {
        return 1u << (page_size + 2);
    }
};

// PI_BSD_DOMx_RLS - Domain release
union PIBsdRls {
    u32 raw;
    struct {
        u32 release : 2;      // Bits 0-1: Release cycles
        u32 : 30;             // Bits 2-31 unused
    };
};

} // namespace n64::interfaces
