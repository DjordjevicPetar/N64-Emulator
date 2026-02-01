#pragma once

#include "../../utils/types.hpp"

namespace n64::rcp {

// RSP Status Register (0x04040010)
union RSPStatus {
    u32 raw;
    struct {
        u32 halt : 1;               // bit 0
        u32 broke : 1;              // bit 1
        u32 dma_busy : 1;           // bit 2
        u32 dma_full : 1;           // bit 3
        u32 io_full : 1;            // bit 4
        u32 single_step : 1;        // bit 5
        u32 interrupt_on_break : 1; // bit 6
        u32 signal0 : 1;            // bit 7
        u32 signal1 : 1;            // bit 8
        u32 signal2 : 1;            // bit 9
        u32 signal3 : 1;            // bit 10
        u32 signal4 : 1;            // bit 11
        u32 signal5 : 1;            // bit 12
        u32 signal6 : 1;            // bit 13
        u32 signal7 : 1;            // bit 14
        u32 : 17;                   // bits 15-31 unused
    };
};

// RSP DMA SP Address Register (0x04040000)
// Note: bits 0-2 are always 0 (8-byte aligned)
union RSPDmaSPAddr {
    u32 raw;
    struct {
        u32 address : 12;    // bits 3-11: offset in DMEM/IMEM (in 8-byte units)
        u32 bank : 1;       // bit 12: 0=DMEM, 1=IMEM
        u32 : 19;           // bits 13-31 unused
    };
};

// RSP DMA RDRAM Address Register (0x04040004)
// Note: bits 0-2 are always 0 (8-byte aligned)
union RSPDmaRamAddr {
    u32 raw;
    struct {
        u32 address : 24;   // bits 3-23: RDRAM address (in 8-byte units)
        u32 : 8;            // bits 24-31 unused
    };
};

// RSP DMA Length Register (0x04040008 / 0x0404000C)
union RSPDmaLen {
    u32 raw;
    struct {
        u32 length : 12;    // bits 0-11: transfer length - 1
        u32 count : 8;      // bits 12-19: number of rows - 1
        u32 skip : 12;      // bits 20-31: skip between rows in RDRAM
    };
};

// Register addresses
enum RSP_REGISTERS_ADDRESS : u32 {
    RSP_DMA_SPADDR  = 0x04040000,
    RSP_DMA_RAMADDR = 0x04040004,
    RSP_DMA_RDLEN   = 0x04040008,
    RSP_DMA_WRLEN   = 0x0404000C,
    RSP_STATUS      = 0x04040010,
    RSP_DMA_FULL    = 0x04040014,
    RSP_DMA_BUSY    = 0x04040018,
    RSP_SEMAPHORE   = 0x0404001C,
    RSP_PC          = 0x04080000
};

} // namespace n64::rcp
