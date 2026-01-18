#pragma once

#include <cstdint>

namespace n64 {

// Unsigned integers
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using u128 = __uint128_t;

// Signed integers
using s8  = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;
using s128 = __int128_t;

// Floating point
using f32 = float;
using f64 = double;

// Verify sizes at compile time
static_assert(sizeof(u8)  == 1, "u8 must be 1 byte");
static_assert(sizeof(u16) == 2, "u16 must be 2 bytes");
static_assert(sizeof(u32) == 4, "u32 must be 4 bytes");
static_assert(sizeof(u64) == 8, "u64 must be 8 bytes");
static_assert(sizeof(u128) == 16, "u128 must be 16 bytes");
static_assert(sizeof(s8) == 1, "s8 must be 1 byte");
static_assert(sizeof(s16) == 2, "s16 must be 2 bytes");
static_assert(sizeof(s32) == 4, "s32 must be 4 bytes");
static_assert(sizeof(s64) == 8, "s64 must be 8 bytes");
static_assert(sizeof(s128) == 16, "s128 must be 16 bytes");
static_assert(sizeof(f32) == 4, "f32 must be 4 bytes");
static_assert(sizeof(f64) == 8, "f64 must be 8 bytes");

// ============================================================================
// Sign Extension Helpers
// ============================================================================

// Sign-extend 8-bit to 64-bit
[[nodiscard]] constexpr s64 sign_extend8(u8 value) {
    return static_cast<s64>(static_cast<s8>(value));
}

// Sign-extend 16-bit to 64-bit
[[nodiscard]] constexpr s64 sign_extend16(u16 value) {
    return static_cast<s64>(static_cast<s16>(value));
}

// Sign-extend 32-bit to 64-bit
[[nodiscard]] constexpr s64 sign_extend32(u32 value) {
    return static_cast<s64>(static_cast<s32>(value));
}

// Extend 8-bit to 64-bit
[[nodiscard]] constexpr u64 extend8(u8 value) {
    return static_cast<u64>(value);
}

// Extend 16-bit to 64-bit
[[nodiscard]] constexpr u64 extend16(u16 value) {
    return static_cast<u64>(value);
}

// Extend 32-bit to 64-bit
[[nodiscard]] constexpr u64 extend32(u32 value) {
    return static_cast<u64>(value);
}

// ============================================================================
// Bit Manipulation Helpers
// ============================================================================

// Extract bits [hi:lo] from value
[[nodiscard]] constexpr u64 get_bits(u64 value, unsigned hi, unsigned lo) {
    u64 mask = ((1ULL << (hi - lo + 1)) - 1) << lo;
    return (value & mask) >> lo;
}

// Get single bit
[[nodiscard]] constexpr bool get_bit(u64 value, unsigned bit) {
    return (value >> bit) & 1;
}

// Set bits [hi:lo] in value
[[nodiscard]] constexpr u64 set_bits(u64 value, unsigned hi, unsigned lo, u64 new_bits) {
    u64 mask = ((1ULL << (hi - lo + 1)) - 1) << lo;
    return (value & ~mask) | ((new_bits << lo) & mask);
}

// Set single bit
[[nodiscard]] constexpr u64 set_bit(u64 value, unsigned bit, bool new_val) {
    if (new_val) {
        return value | (1ULL << bit);
    }
    return value & ~(1ULL << bit);
}

// ============================================================================
// Byte Swap (for endianness conversion)
// ============================================================================

template<typename T>
[[nodiscard]] constexpr T byte_swap(T value) {
    if constexpr (sizeof(T) == 1) {
        return value;
    } else if constexpr (sizeof(T) == 2) {
        auto v = static_cast<u16>(value);
        return static_cast<T>((v >> 8) | (v << 8));
    } else if constexpr (sizeof(T) == 4) {
        auto v = static_cast<u32>(value);
        return static_cast<T>(
            ((v >> 24) & 0x000000FF) |
            ((v >> 8)  & 0x0000FF00) |
            ((v << 8)  & 0x00FF0000) |
            ((v << 24) & 0xFF000000)
        );
    } else if constexpr (sizeof(T) == 8) {
        auto v = static_cast<u64>(value);
        return static_cast<T>(
            ((v >> 56) & 0x00000000000000FFULL) |
            ((v >> 40) & 0x000000000000FF00ULL) |
            ((v >> 24) & 0x0000000000FF0000ULL) |
            ((v >> 8)  & 0x00000000FF000000ULL) |
            ((v << 8)  & 0x000000FF00000000ULL) |
            ((v << 24) & 0x0000FF0000000000ULL) |
            ((v << 40) & 0x00FF000000000000ULL) |
            ((v << 56) & 0xFF00000000000000ULL)
        );
    }
}

} // namespace n64