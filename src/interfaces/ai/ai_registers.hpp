#pragma once

#include "../../utils/types.hpp"

namespace n64::interfaces {

// AI Register Addresses
enum AI_REGISTERS_ADDRESS : u32 {
    AI_DRAM_ADDR = 0x04500000,
    AI_LENGTH    = 0x04500004,
    AI_CONTROL   = 0x04500008,
    AI_STATUS    = 0x0450000C,
    AI_DACRATE   = 0x04500010,
    AI_BITRATE   = 0x04500014,
};

// AI_DRAM_ADDR (0x04500000) - RDRAM address for DMA (write-only, reads mirror AI_LENGTH)
union AIDramAddr {
    u32 raw;
    struct {
        u32 address : 24;     // Bits 0-23: RDRAM address
        u32 : 8;              // Bits 24-31: unused
    };
};

// AI_LENGTH (0x04500004) - DMA transfer length in bytes
// Write: sets length for next DMA transfer
// Read: returns bytes remaining in current transfer
union AILength {
    u32 raw;
    struct {
        u32 length : 18;      // Bits 0-17: transfer length
        u32 : 14;             // Bits 18-31: unused
    };
};

// AI_CONTROL (0x04500008) - DMA enable (write-only, reads mirror AI_LENGTH)
union AIControl {
    u32 raw;
    struct {
        u32 dma_enable : 1;   // Bit 0: enable AI DMA
        u32 : 31;             // Bits 1-31: unused
    };
};

// AI_STATUS (0x0450000C) - Status register
// Read: returns status flags
// Write: clears AI interrupt
union AIStatus {
    u32 raw;
    struct {
        u32 full0 : 1;        // Bit 0: FIFO full (duplicate of bit 31)
        u32 count : 14;       // Bits 1-14: internal DAC counter
        u32 : 1;              // Bit 15: unused
        u32 bit_clock : 1;    // Bit 16: bit clock readback (BCLK)
        u32 : 2;              // Bits 17-18: unused
        u32 word_clock : 1;   // Bit 19: word clock readback (LRCK)
        u32 : 5;              // Bits 20-24: unused
        u32 enabled : 1;      // Bit 25: reflects AI_CONTROL DMA_ENABLE
        u32 : 4;              // Bits 26-29: unused
        u32 busy : 1;         // Bit 30: DMA transfer in progress
        u32 full : 1;         // Bit 31: DMA FIFO is full (second transfer pending)
    };
};

// AI_DACRATE (0x04500010) - DAC sample rate (write-only, reads mirror AI_LENGTH)
// Sample rate = VI_clock / (dac_rate + 1)
// NTSC: VI_clock = 48,681,812 Hz. dac_rate=1103 -> ~44,100 Hz
union AIDacRate {
    u32 raw;
    struct {
        u32 dac_rate : 14;    // Bits 0-13: sample period divisor
        u32 : 18;             // Bits 14-31: unused
    };
};

// AI_BITRATE (0x04500014) - Bit clock rate (write-only, reads mirror AI_LENGTH)
// Bit clock = VI_clock / (bitrate + 1). Must be >= 66x faster than DAC rate.
// Value of 0 stops the clock.
union AIBitRate {
    u32 raw;
    struct {
        u32 bitrate : 4;      // Bits 0-3: bit clock period
        u32 : 28;             // Bits 4-31: unused
    };
};

} // namespace n64::interfaces
