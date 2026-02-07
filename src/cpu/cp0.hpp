#pragma once

#include <array>
#include "../utils/types.hpp"

namespace n64::cpu {

// =============================================================================
// CP0 Register Unions
// =============================================================================

// Register 0: Index - TLB entry index for read/write
union CP0Index {
    u32 raw;
    struct {
        u32 index : 6;      // Bits 0-5: TLB entry index (0-31)
        u32 : 25;           // Bits 6-30: Unused
        u32 probe : 1;      // Bit 31: Set to 1 if TLBP found no match
    };
};

// Register 1: Random - Pseudo-random TLB entry for TLBWR
// Decrements each cycle, wraps from 31 to WIRED value
union CP0Random {
    u32 raw;
    struct {
        u32 random : 6;     // Bits 0-5: Random value (WIRED to 31)
        u32 : 26;           // Bits 6-31: Unused
    };
};

// Register 2/3: EntryLo0/EntryLo1 - TLB entry low bits (even/odd pages)
union CP0EntryLo {
    u64 raw;
    struct {
        u64 global : 1;     // Bit 0: Global bit (ignores ASID)
        u64 valid : 1;      // Bit 1: Valid bit
        u64 dirty : 1;      // Bit 2: Dirty/writable bit
        u64 cache : 3;      // Bits 3-5: Cache algorithm
        u64 pfn : 20;       // Bits 6-25: Page Frame Number
        u64 : 38;           // Bits 26-63: Unused
    };
};

// Register 4: Context - Pointer to PTE in memory (32-bit mode)
union CP0Context {
    u64 raw;
    struct {
        u64 : 4;            // Bits 0-3: Zero
        u64 bad_vpn2 : 19;  // Bits 4-22: Virtual Page Number / 2
        u64 pte_base : 9;   // Bits 23-31: Page Table Entry base
        u64 : 32;           // Bits 32-63: Unused in 32-bit mode
    };
};

// Register 5: PageMask - TLB page size mask
union CP0PageMask {
    u32 raw;
    struct {
        u32 : 13;           // Bits 0-12: Zero
        u32 mask : 12;      // Bits 13-24: Page size mask
        u32 : 7;            // Bits 25-31: Unused
    };
    // mask=0x000 -> 4KB, mask=0x003 -> 16KB, mask=0x00F -> 64KB, etc.
};

// Register 6: Wired - Number of wired TLB entries (protected from TLBWR)
union CP0Wired {
    u32 raw;
    struct {
        u32 wired : 6;      // Bits 0-5: Wired boundary (0 to 31)
        u32 : 26;           // Bits 6-31: Unused
    };
};

// Register 8: BadVAddr - Address that caused exception
// This is a simple 64-bit register, no bitfields needed

// Register 9: Count - Timer counter (increments every other cycle)
// This is a simple 32-bit register, no bitfields needed

// Register 10: EntryHi - TLB entry high bits (VPN2 and ASID)
union CP0EntryHi {
    u64 raw;
    struct {
        u64 asid : 8;       // Bits 0-7: Address Space ID
        u64 : 5;            // Bits 8-12: Unused
        u64 vpn2 : 27;      // Bits 13-39: Virtual Page Number / 2
        u64 : 22;           // Bits 40-61: Unused
        u64 region : 2;     // Bits 62-63: Region (00=user, 01=supervisor, 11=kernel)
    };
};

// Register 11: Compare - Timer compare value (triggers interrupt when Count == Compare)
// This is a simple 32-bit register, no bitfields needed

// Register 12: Status - Processor status and control
union CP0Status {
    u32 raw;
    struct {
        u32 ie : 1;         // Bit 0: Interrupt Enable
        u32 exl : 1;        // Bit 1: Exception Level (1 = in exception)
        u32 erl : 1;        // Bit 2: Error Level (1 = in error)
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

// Register 13: Cause - Exception cause
union CP0Cause {
    u32 raw;
    struct {
        u32 : 2;            // Bits 0-1: Unused
        u32 exc_code : 5;   // Bits 2-6: Exception code
        u32 : 1;            // Bit 7: Unused
        u32 ip : 8;         // Bits 8-15: Interrupt Pending (IP0-IP7)
        u32 : 12;           // Bits 16-27: Unused
        u32 ce : 2;         // Bits 28-29: Coprocessor error number
        u32 : 1;            // Bit 30: Unused
        u32 bd : 1;         // Bit 31: Branch Delay (exception in delay slot)
    };
};

// Register 14: EPC - Exception Program Counter
// This is a simple 64-bit register, no bitfields needed

// Register 15: PRId - Processor Revision Identifier (read-only)
union CP0PRId {
    u32 raw;
    struct {
        u32 revision : 8;       // Bits 0-7: Revision number
        u32 implementation : 8; // Bits 8-15: Implementation number (0x0B for VR4300)
        u32 : 16;               // Bits 16-31: Unused
    };
};

// Register 16: Config - Cache and system configuration
union CP0Config {
    u32 raw;
    struct {
        u32 k0 : 3;         // Bits 0-2: KSEG0 cache algorithm
        u32 cu : 1;         // Bit 3: Reserved
        u32 : 11;           // Bits 4-14: Fixed pattern (11001000110)
        u32 be : 1;         // Bit 15: Big Endian (1 = big endian)
        u32 : 8;            // Bits 16-23: Fixed pattern (00000110)
        u32 ep : 4;         // Bits 24-27: Transfer data pattern
        u32 ec : 3;         // Bits 28-30: System clock ratio (read-only)
        u32 : 1;            // Bit 31: Zero
    };
};

// Register 17: LLAddr - Load Linked Address (physical address >> 4)
// This is a simple 32-bit register, no bitfields needed

// Register 18: WatchLo - Memory trap address low bits
union CP0WatchLo {
    u32 raw;
    struct {
        u32 w : 1;          // Bit 0: Trap on store
        u32 r : 1;          // Bit 1: Trap on load
        u32 : 1;            // Bit 2: Unused
        u32 paddr0 : 29;    // Bits 3-31: Physical address bits 3-31
    };
};

// Register 19: WatchHi - Memory trap address high bits
union CP0WatchHi {
    u32 raw;
    struct {
        u32 paddr1 : 4;     // Bits 0-3: Physical address bits 32-35
        u32 : 28;           // Bits 4-31: Unused
    };
};

// Register 20: XContext - Extended context for 64-bit mode
union CP0XContext {
    u64 raw;
    struct {
        u64 : 4;            // Bits 0-3: Zero
        u64 bad_vpn2 : 27;  // Bits 4-30: Virtual Page Number / 2
        u64 r : 2;          // Bits 31-32: Region
        u64 pte_base : 31;  // Bits 33-63: Page Table Entry base
    };
};

// Register 26: ParityError - Cache parity error
union CP0ParityError {
    u32 raw;
    struct {
        u32 diagnostic : 8; // Bits 0-7: Diagnostic field
        u32 : 24;           // Bits 8-31: Unused
    };
};

// Register 27: CacheError - Cache error status
// Implementation-specific, usually just raw access

// Register 28: TagLo - Cache tag low bits
union CP0TagLo {
    u32 raw;
    struct {
        u32 : 6;            // Bits 0-5: Unused
        u32 p_state : 2;    // Bits 6-7: Primary cache state
        u32 p_tag_lo : 20;  // Bits 8-27: Physical tag
        u32 : 4;            // Bits 28-31: Unused
    };
};

// Register 29: TagHi - Cache tag high bits (unused on VR4300)
// Just a 32-bit register

// Register 30: ErrorEPC - Error Exception Program Counter
// This is a simple 64-bit register, no bitfields needed

// =============================================================================
// CP0 Register indices enum
// =============================================================================

enum class CP0Reg : u8 {
    INDEX       = 0,
    RANDOM      = 1,
    ENTRY_LO0   = 2,
    ENTRY_LO1   = 3,
    CONTEXT     = 4,
    PAGE_MASK   = 5,
    WIRED       = 6,
    // 7 is reserved
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
    // 21-25 reserved
    PARITY_ERR  = 26,
    CACHE_ERR   = 27,
    TAG_LO      = 28,
    TAG_HI      = 29,
    ERROR_EPC   = 30,
    // 31 reserved
};

// Exception codes for Cause register
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
// CP0 Class
// =============================================================================

class CP0 {
public:
    CP0();
    ~CP0() = default;

    // Generic register access (for MTC0/MFC0)
    void set_reg(u8 index, u64 value);
    void set_reg(CP0Reg reg, u64 value) { set_reg(static_cast<u8>(reg), value); }
    [[nodiscard]] u64 get_reg(u8 index) const;
    [[nodiscard]] u64 get_reg(CP0Reg reg) const { return get_reg(static_cast<u8>(reg)); }

    // Address translation
    [[nodiscard]] u32 translate_address(u64 virtual_address) const;

    // Direct register accessors
    [[nodiscard]] CP0Status& status() { return status_; }
    [[nodiscard]] const CP0Status& status() const { return status_; }
    [[nodiscard]] CP0Cause& cause() { return cause_; }
    [[nodiscard]] const CP0Cause& cause() const { return cause_; }

    [[nodiscard]] bool get_fr_bit() const { return status_.fr; }

private:
    // TLB registers
    CP0Index index_;
    CP0Random random_;
    CP0EntryLo entry_lo0_;
    CP0EntryLo entry_lo1_;
    CP0Context context_;
    CP0PageMask page_mask_;
    CP0Wired wired_;
    
    // Exception registers
    u64 bad_vaddr_;
    u32 count_;
    CP0EntryHi entry_hi_;
    u32 compare_;
    CP0Status status_;
    CP0Cause cause_;
    u64 epc_;
    
    // Configuration registers
    CP0PRId prid_;
    CP0Config config_;
    u32 ll_addr_;
    CP0WatchLo watch_lo_;
    CP0WatchHi watch_hi_;
    CP0XContext xcontext_;
    
    // Cache/error registers
    CP0ParityError parity_error_;
    u32 cache_error_;
    CP0TagLo tag_lo_;
    u32 tag_hi_;
    u64 error_epc_;
};

} // namespace n64::cpu
