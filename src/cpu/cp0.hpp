#pragma once

#include <array>

#include "../utils/types.hpp"

namespace n64::cpu {

// CP0 Register indices
enum class CP0Reg : u8 {
    INDEX = 0,
    RANDOM = 1,
    ENTRY_LO0 = 2,
    ENTRY_LO1 = 3,
    CONTEXT = 4,
    PAGE_MASK = 5,
    WIRED = 6,
    BAD_VADDR = 8,
    COUNT = 9,
    ENTRY_HI = 10,
    COMPARE = 11,
    STATUS = 12,
    CAUSE = 13,
    EPC = 14,
    PRID = 15,
    CONFIG = 16,
    LL_ADDR = 17,
    WATCH_LO = 18,
    WATCH_HI = 19,
    X_CONTEXT = 20,
    PARITY_ERROR = 26,
    CACHE_ERROR = 27,
    TAG_LO = 28,
    TAG_HI = 29,
    ERROR_EPC = 30,
};

class CP0 {
public:
    CP0();
    ~CP0() = default;

    // Register access
    void set_reg(u8 index, u64 value);
    void set_reg(CP0Reg reg, u64 value) { set_reg(static_cast<u8>(reg), value); }
    [[nodiscard]] u64 get_reg(u8 index) const;
    [[nodiscard]] u64 get_reg(CP0Reg reg) const { return get_reg(static_cast<u8>(reg)); }

    // Address translation
    [[nodiscard]] u32 translate_address(u64 virtual_address) const;

private:
    std::array<u64, 32> regs_;
};

} // namespace n64::cpu
