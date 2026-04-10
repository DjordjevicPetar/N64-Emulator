#pragma once

#include "../utils/types.hpp"

namespace n64::cpu {

// =============================================================================
// CP0 Register Unions
// =============================================================================

// Register 0: Index
union CP0Index {
    u32 raw;
    struct {
        u32 index : 6;      // Bits 0-5: TLB entry index (0-31)
        u32 : 25;           // Bits 6-30: Unused
        u32 probe : 1;      // Bit 31: Set to 1 if TLBP found no match
    };
};

// Register 1: Random (decrements each instruction, wraps from 31 to Wired)
union CP0Random {
    u32 raw;
    struct {
        u32 random : 6;     // Bits 0-5: Random value (Wired to 31)
        u32 : 26;
    };
};

// Register 2/3: EntryLo0/EntryLo1
union CP0EntryLo {
    u64 raw;
    struct {
        u64 global : 1;     // Bit 0: Global bit (ignores ASID)
        u64 valid : 1;      // Bit 1: Valid bit
        u64 dirty : 1;      // Bit 2: Dirty/writable bit
        u64 cache : 3;      // Bits 3-5: Cache algorithm
        u64 pfn : 20;       // Bits 6-25: Page Frame Number
        u64 : 38;
    };
};

// Register 4: Context
union CP0Context {
    u64 raw;
    struct {
        u64 : 4;
        u64 bad_vpn2 : 19;  // Bits 4-22: Virtual Page Number / 2
        u64 pte_base : 41;  // Bits 23-63: Page Table Entry base
    };
};

// Register 5: PageMask
union CP0PageMask {
    u32 raw;
    struct {
        u32 : 13;
        u32 mask : 12;      // Bits 13-24: Page size mask
        u32 : 7;
    };
    // mask=0x000 -> 4KB, 0x003 -> 16KB, 0x00F -> 64KB,
    // 0x03F -> 256KB, 0x0FF -> 1MB, 0x3FF -> 4MB, 0xFFF -> 16MB
};

// Register 6: Wired
union CP0Wired {
    u32 raw;
    struct {
        u32 wired : 6;      // Bits 0-5: Wired boundary (0 to 31)
        u32 : 26;
    };
};

// Register 10: EntryHi
union CP0EntryHi {
    u64 raw;
    struct {
        u64 asid : 8;       // Bits 0-7: Address Space ID
        u64 : 5;
        u64 vpn2 : 27;      // Bits 13-39: Virtual Page Number / 2
        u64 : 22;
        u64 region : 2;     // Bits 62-63: Region (00=user, 01=supervisor, 11=kernel)
    };
};

// Register 12: Status
union CP0Status {
    u32 raw;
    struct {
        u32 ie : 1;         // Bit 0: Interrupt Enable
        u32 exl : 1;        // Bit 1: Exception Level
        u32 erl : 1;        // Bit 2: Error Level
        u32 ksu : 2;        // Bits 3-4: Mode (00=kernel, 01=supervisor, 10=user)
        u32 ux : 1;         // Bit 5: User mode 64-bit addressing
        u32 sx : 1;         // Bit 6: Supervisor mode 64-bit addressing
        u32 kx : 1;         // Bit 7: Kernel mode 64-bit addressing
        u32 im : 8;         // Bits 8-15: Interrupt Mask
        u32 ds : 9;         // Bits 16-24: Diagnostic Status
        u32 re : 1;         // Bit 25: Reverse Endian in user mode
        u32 fr : 1;         // Bit 26: FPU register mode (0=32x32bit, 1=32x64bit)
        u32 rp : 1;         // Bit 27: Reduced Power mode
        u32 cu0 : 1;        // Bit 28: COP0 usable
        u32 cu1 : 1;        // Bit 29: COP1 (FPU) usable
        u32 cu2 : 1;        // Bit 30: COP2 usable
        u32 cu3 : 1;        // Bit 31: COP3 usable
    };
};

// Register 13: Cause
union CP0Cause {
    u32 raw;
    struct {
        u32 : 2;
        u32 exc_code : 5;   // Bits 2-6: Exception code
        u32 : 1;
        u32 ip : 7;         // Bits 8-14: Interrupt Pending (IP0-IP6)
        u32 timer_int : 1;  // Bit 15: Timer interrupt (IP7)
        u32 : 12;
        u32 ce : 2;         // Bits 28-29: Coprocessor error number
        u32 : 1;
        u32 bd : 1;         // Bit 31: Branch Delay
    };
};

// Register 15: PRId
union CP0PRId {
    u32 raw;
    struct {
        u32 revision : 8;
        u32 implementation : 8; // 0x0B for VR4300
        u32 : 16;
    };
};

// Register 16: Config
union CP0Config {
    u32 raw;
    struct {
        u32 k0 : 3;         // Bits 0-2: KSEG0 cache algorithm
        u32 cu : 1;
        u32 : 11;
        u32 be : 1;         // Bit 15: Big Endian
        u32 : 8;
        u32 ep : 4;         // Bits 24-27: Transfer data pattern
        u32 ec : 3;         // Bits 28-30: System clock ratio (read-only)
        u32 : 1;
    };
};

// Register 18: WatchLo
union CP0WatchLo {
    u32 raw;
    struct {
        u32 w : 1;          // Bit 0: Trap on store
        u32 r : 1;          // Bit 1: Trap on load
        u32 : 1;
        u32 paddr0 : 29;    // Bits 3-31: Physical address bits 3-31
    };
};

// Register 19: WatchHi
union CP0WatchHi {
    u32 raw;
    struct {
        u32 paddr1 : 4;     // Bits 0-3: Physical address bits 32-35
        u32 : 28;
    };
};

// Register 20: XContext
union CP0XContext {
    u64 raw;
    struct {
        u64 : 4;
        u64 bad_vpn2 : 27;  // Bits 4-30: Virtual Page Number / 2
        u64 r : 2;          // Bits 31-32: Region
        u64 pte_base : 31;  // Bits 33-63: Page Table Entry base
    };
};

// Register 26: ParityError
union CP0ParityError {
    u32 raw;
    struct {
        u32 diagnostic : 8;
        u32 : 24;
    };
};

// Register 28: TagLo
union CP0TagLo {
    u32 raw;
    struct {
        u32 : 6;
        u32 p_state : 2;    // Bits 6-7: Primary cache state
        u32 p_tag_lo : 20;  // Bits 8-27: Physical tag
        u32 : 4;
    };
};

// =============================================================================
// CP0 Register Index Enum
// =============================================================================

enum class CP0Reg : u8 {
    INDEX       = 0,
    RANDOM      = 1,
    ENTRY_LO0   = 2,
    ENTRY_LO1   = 3,
    CONTEXT     = 4,
    PAGE_MASK   = 5,
    WIRED       = 6,
    BAD_VADDR   = 8,
    COUNT       = 9,
    ENTRY_HI    = 10,
    COMPARE     = 11,
    STATUS      = 12,
    CAUSE       = 13,
    EPC         = 14,
    PRID        = 15,
    CONFIG      = 16,
    LL_ADDR     = 17,
    WATCH_LO    = 18,
    WATCH_HI    = 19,
    X_CONTEXT   = 20,
    PARITY_ERR  = 26,
    CACHE_ERR   = 27,
    TAG_LO      = 28,
    TAG_HI      = 29,
    ERROR_EPC   = 30,
};

// =============================================================================
// Exception Codes
// =============================================================================

enum class ExceptionCode : u8 {
    INT     = 0,    // Interrupt
    MOD     = 1,    // TLB modification
    TLBL    = 2,    // TLB miss (load/fetch)
    TLBS    = 3,    // TLB miss (store)
    ADEL    = 4,    // Address error (load/fetch)
    ADES    = 5,    // Address error (store)
    IBE     = 6,    // Bus error (instruction fetch)
    DBE     = 7,    // Bus error (data load/store)
    SYS     = 8,    // Syscall
    BP      = 9,    // Breakpoint
    RI      = 10,   // Reserved instruction
    CPU     = 11,   // Coprocessor unusable
    OV      = 12,   // Arithmetic overflow
    TR      = 13,   // Trap
    FPE     = 15,   // Floating point exception
    WATCH   = 23,   // Watch exception
};

// =============================================================================
// TLB Entry
// =============================================================================

struct TLBEntry {
    CP0EntryHi entry_hi;
    CP0EntryLo entry_lo0;
    CP0EntryLo entry_lo1;
    CP0PageMask page_mask;
    bool global;
};

// =============================================================================
// Exception Vector Addresses
// =============================================================================

constexpr u64 EXCEPTION_VECTOR_ADDRESS_64_RESET    = 0xFFFF'FFFF'BFC0'0000;
constexpr u32 EXCEPTION_VECTOR_ADDRESS_32_RESET    = 0xBFC0'0000;
constexpr u64 EXCEPTION_VECTOR_ADDRESS_64_BEV      = 0xFFFF'FFFF'BFC0'0200;
constexpr u64 EXCEPTION_VECTOR_ADDRESS_64_NO_BEV   = 0xFFFF'FFFF'8000'0000;
constexpr u32 EXCEPTION_VECTOR_ADDRESS_32_BEV      = 0xBFC0'0200;
constexpr u32 EXCEPTION_VECTOR_ADDRESS_32_NO_BEV   = 0x8000'0000;

} // namespace n64::cpu
