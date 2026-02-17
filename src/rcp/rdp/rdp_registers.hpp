#pragma once

#include "../../utils/types.hpp"

namespace n64::rdp {

enum RDP_REGISTERS_ADDRESS : u32 {
    // DPC (Command) registers
    DPC_START      = 0x04100000,
    DPC_END        = 0x04100004,
    DPC_CURRENT    = 0x04100008,
    DPC_STATUS     = 0x0410000C,
    DPC_CLOCK      = 0x04100010,
    DPC_BUF_BUSY   = 0x04100014,
    DPC_PIPE_BUSY  = 0x04100018,
    DPC_TMEM_BUSY  = 0x0410001C,

    // DPS (Span) registers
    DPS_TBIST         = 0x04200000,
    DPS_TEST_MODE     = 0x04200004,
    DPS_BUFTEST_ADDR  = 0x04200008,
    DPS_BUFTEST_DATA  = 0x0420000C
};

// DPC_START (0x04100000) - RDRAM start address of command buffer
union DPCStart {
    u32 raw;
    struct {
        u32 start : 24;  // 0-23: RDRAM address (64-bit aligned, bits 0-2 always 0)
        u32 : 8;         // 24-31: unused
    };
};

// DPC_END (0x04100004) - RDRAM end address of command buffer
union DPCEnd {
    u32 raw;
    struct {
        u32 end : 24;  // 0-23: RDRAM address (64-bit aligned, bits 0-2 always 0)
        u32 : 8;       // 24-31: unused
    };
};

// DPC_CURRENT (0x04100008) - Current RDRAM address being processed (read-only)
union DPCCurrent {
    u32 raw;
    struct {
        u32 current : 24;  // 0-23: Current RDRAM read address
        u32 : 8;           // 24-31: unused
    };
};

// DPC_STATUS (0x0410000C)
// Read format - returns status flags
union DPCStatus {
    u32 raw;
    struct {
        u32 xbus_dmem_dma : 1;  // 0: Source is DMEM (1) or RDRAM (0)
        u32 freeze : 1;         // 1: RDP pipeline frozen
        u32 flush : 1;          // 2: RDP pipeline flushed
        u32 start_gclk : 1;    // 3: Start gated clock (gclk alive)
        u32 tmem_busy : 1;     // 4: TMEM busy
        u32 pipe_busy : 1;     // 5: Pipe busy
        u32 cmd_busy : 1;      // 6: Command unit busy
        u32 cbuf_ready : 1;    // 7: Command buffer ready
        u32 dma_busy : 1;      // 8: DMA busy
        u32 end_pending : 1;     // 9: End register is valid
        u32 start_pending : 1;   // 10: Start register is valid
        u32 : 21;              // 11-31: unused
    };
};

// DPC_CLOCK (0x04100010) - Clock counter (read-only)
union DPCClock {
    u32 raw;
    struct {
        u32 clock : 24;  // 0-23: Clock counter
        u32 : 8;         // 24-31: unused
    };
};

// DPC_BUF_BUSY (0x04100014) - Buffer busy counter (read-only)
union DPCBufBusy {
    u32 raw;
    struct {
        u32 buf_busy : 24;  // 0-23: Buffer busy counter
        u32 : 8;            // 24-31: unused
    };
};

// DPC_PIPE_BUSY (0x04100018) - Pipe busy counter (read-only)
union DPCPipeBusy {
    u32 raw;
    struct {
        u32 pipe_busy : 24;  // 0-23: Pipe busy counter
        u32 : 8;             // 24-31: unused
    };
};

// DPC_TMEM_BUSY (0x0410001C) - TMEM load counter (read-only)
union DPCTmemBusy {
    u32 raw;
    struct {
        u32 tmem_busy : 24;  // 0-23: TMEM load counter
        u32 : 8;             // 24-31: unused
    };
};

// DPS_TBIST (0x04200000) - BIST testing control
union DPSTbist {
    u32 raw;
    struct {
        u32 bist_check : 1;  // 0: BIST check mode
        u32 bist_go : 1;     // 1: Start BIST
        u32 bist_done : 1;   // 2: BIST done (read-only)
        u32 : 1;             // 3: unused
        u32 bist_fail : 8;   // 4-11: BIST fail flags (read-only)
        u32 : 20;            // 12-31: unused
    };
};

// DPS_TEST_MODE (0x04200004) - Test mode
union DPSTestMode {
    u32 raw;
    struct {
        u32 test_mode : 1;  // 0: Test mode enable
        u32 : 31;           // 1-31: unused
    };
};

// DPS_BUFTEST_ADDR (0x04200008) - Buffer test address
union DPSBuftestAddr {
    u32 raw;
    struct {
        u32 buftest_addr : 7;  // 0-6: Buffer test address
        u32 : 25;              // 7-31: unused
    };
};

// DPS_BUFTEST_DATA (0x0420000C) - Buffer test data
union DPSBuftestData {
    u32 raw;
    struct {
        u32 buftest_data : 32;  // 0-31: Buffer test data
    };
};

} // namespace n64::rdp
