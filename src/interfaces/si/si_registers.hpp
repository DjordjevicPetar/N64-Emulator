#pragma once

#include "../../utils/types.hpp"

namespace n64::interfaces {

enum SI_REGISTERS_ADDRESS : u32 {
    SI_DRAM_ADDR = 0x04800000,
    SI_PIF_AD_RD64B = 0x04800004,
    SI_PIF_AD_WR4B = 0x04800008,
    SI_PIF_AD_WR64B = 0x04800010,
    SI_PIF_AD_RD4B = 0x04800014,
    SI_STATUS = 0x04800018,
};

// SI_DRAM_ADDR (0x04800000) - RDRAM address for DMA
union SIDramAddr {
    u32 raw;
    struct {
        u32 address : 24;     // Bits 0-23: RDRAM address
        u32 : 8;              // Bits 24-31: unused
    };
};

// SI_PIF_AD_RD64B (0x04800004) - PIF address for read (64-bit)
union SIPIFAdRD64B {
    u32 raw;
    struct {
        u32 pif_address : 11;  // Bits 0-10: PIF address
        u32 : 21;              // Bits 11-31: unused
    };
};

// SI_PIF_AD_WR4B (0x04800008) - PIF address for write (4-bit)
union SIPIFAdWR4B {
    u32 raw;
};

// SI_PIF_AD_WR64B (0x04800010) - PIF address for write (64-bit)
union SIPIFAdWR64B {
    u32 raw;
};

// SI_PIF_AD_RD4B (0x04800014) - PIF address for read (4-bit)
union SIPIFAdRD4B {
    u32 raw;
};

// SI_STATUS (0x04800018) - Status register
union SIStatus {
    u32 raw;
    struct {
        u32 dma_busy : 1;       // Bit 0: DMA transfer in progress
        u32 io_busy : 1;        // Bit 1: IO access in progress
        u32 read_pending : 1;   // Bit 2: Read pending
        u32 dma_error : 1;      // Bit 3: DMA error occurred
        u32 pch_state : 4;      // Bits 4-7: PIF channel state
        u32 dma_state : 4;      // Bits 8-11: DMA state
        u32 interrupt : 1;      // Bit 12: SI interrupt pending
        u32 : 19;               // Bits 13-31: unused
    };
};
}