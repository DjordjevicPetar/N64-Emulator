#pragma once

#include <array>

#include "../utils/types.hpp"

namespace n64::cpu {

// FCR31 - FP Control/Status Register
union FCStatusReg {
    u32 raw;
    struct {
        u32 rounding_mode : 2;              // Bits 0-1
        u32 flag_inexact : 1;               // Bit 2
        u32 flag_underflow : 1;             // Bit 3
        u32 flag_overflow : 1;              // Bit 4
        u32 flag_divide_by_zero : 1;        // Bit 5
        u32 flag_invalid : 1;               // Bit 6
        u32 enable_inexact : 1;             // Bit 7
        u32 enable_underflow : 1;           // Bit 8
        u32 enable_overflow : 1;            // Bit 9
        u32 enable_divide_by_zero : 1;      // Bit 10
        u32 enable_invalid : 1;             // Bit 11
        u32 cause_inexact : 1;              // Bit 12
        u32 cause_underflow : 1;            // Bit 13
        u32 cause_overflow : 1;             // Bit 14
        u32 cause_divide_by_zero : 1;       // Bit 15
        u32 cause_invalid : 1;              // Bit 16
        u32 cause_unimplemented : 1;        // Bit 17
        u32 : 5;                            // Bits 18-22 unused
        u32 condition : 1;                  // Bit 23 - result of comparison is true
        u32 flush_subnormals : 1;           // Bit 24 - subnormals are flushed to zero, instead of exception
        u32 : 7;                            // Bits 25-31 unused
    };
};

union RevisionReg {
    u32 raw;
    struct {
        u32 revision : 8;                   // Bit 0-7 - revision number
        u32 implementation : 8;             // Bit 8-15 - implementation number
        u32 : 16;                           // Bits 16-31 unused
    };
};

// Operation flow:
// 1. Clear all cause bits
// 2. Perform operation
// 3. Set "Cause: Inexact"
// 4. If "Enable: Inexact" is set, raise exception
// Otherwise, set "Flag: Inexact" and put result in FPR

enum class FPException : u8 {
    INEXACT = 0,        // Destination cannot hold the full result, data loss
    UNDERFLOW_EX = 1,   // Result is subnormal (less than minimum normalized value)
    OVERFLOW_EX = 2,    // Result is too large to represent in destination format
    DIV_BY_ZERO = 3,    // Division by zero
    INVALID = 4,        // Invalid operation, e.g. sqrt(-1)
    UNIMPLEMENTED = 5,  // Unimplemented FPU operation
};

enum class FPRoundingMode : u8 {
    ROUND_TO_NEAREST = 0,           // RN
    ROUND_TO_ZERO = 1,              // RZ
    ROUND_TO_POSITIVE_INFINITY = 2, // RP
    ROUND_TO_NEGATIVE_INFINITY = 3, // RM
};

class CP1 {
public:
    CP1();
    ~CP1() = default;

    // Raw integer access (for load/store, move instructions)
    [[nodiscard]] u32 get_fpr_32(u8 index) const;
    [[nodiscard]] u64 get_fpr_64(u8 index) const;
    void set_fpr_32(u8 index, u32 value);
    void set_fpr_64(u8 index, u64 value);

    // Control register access
    [[nodiscard]] u32 get_fcr(u8 index) const;
    void set_fcr(u8 index, u32 value);

private:
    // 32 64-bit registers (can be accessed as 32 singles or 16 doubles depending on FR bit)
    std::array<u64, 32> fpr_{};
    FCStatusReg status_;
    RevisionReg revision_;
};

} // namespace n64::cpu