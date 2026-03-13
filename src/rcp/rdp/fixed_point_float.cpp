#include "fixed_point_float.hpp"
#include <cstdio>
#include <stdexcept>

namespace n64::rdp {

static constexpr s64 S15_16_MAX = 0x7FFFFFFF;
static constexpr s64 S15_16_MIN = -0x80000000LL;

static void check_overflow(s64 result, const char* op) {
    if (result > S15_16_MAX || result < S15_16_MIN) {
        char buf[128];
        snprintf(buf, sizeof(buf), "FixedPointFloat overflow in %s: result = %lld", op, (long long)result);
        throw std::overflow_error(buf);
    }
}

FixedPointFloat::FixedPointFloat()
    : raw_(0)
    , integer_size_(15)
    , is_signed_(true)
{}

FixedPointFloat::FixedPointFloat(s32 raw)
    : raw_(raw)
    , integer_size_(15)
    , is_signed_(true)
{}

FixedPointFloat::FixedPointFloat(u16 int_part, u16 frac_part, u8 int_size, u8 frac_size, bool is_signed)
    : integer_size_(int_size)
    , is_signed_(is_signed)
{
    s32 integer = int_part;
    if (is_signed && int_size <= 16) {
        integer = sign_extend_n(integer, int_size);
    }

    u32 fraction = static_cast<u32>(frac_part) << (16 - frac_size);

    raw_ = (integer << 16) | static_cast<s32>(fraction & 0xFFFF);
}

s32 FixedPointFloat::integer() const {
    s32 value = raw_ >> 16;
    if (is_signed_) {
        s32 min_val = -(1 << (integer_size_ - 1));
        s32 max_val = (1 << (integer_size_ - 1)) - 1;
        return std::clamp(value, min_val, max_val);
    }
    return std::clamp(value, 0, (1 << integer_size_) - 1);
}

FixedPointFloat& FixedPointFloat::operator+=(const FixedPointFloat& rhs) {
    s64 result = static_cast<s64>(raw_) + rhs.raw_;
    check_overflow(result, "operator+=");
    raw_ = static_cast<s32>(result);
    return *this;
}

FixedPointFloat& FixedPointFloat::operator-=(const FixedPointFloat& rhs) {
    s64 result = static_cast<s64>(raw_) - rhs.raw_;
    check_overflow(result, "operator-=");
    raw_ = static_cast<s32>(result);
    return *this;
}

FixedPointFloat& FixedPointFloat::operator++() {
    raw_ += (1 << 16);
    return *this;
}

FixedPointFloat FixedPointFloat::operator++(int) {
    FixedPointFloat old = *this;
    raw_ += (1 << 16);
    return old;
}

FixedPointFloat operator+(FixedPointFloat lhs, const FixedPointFloat& rhs) {
    lhs += rhs;
    return lhs;
}

FixedPointFloat operator-(FixedPointFloat lhs, const FixedPointFloat& rhs) {
    lhs -= rhs;
    return lhs;
}

FixedPointFloat operator*(const FixedPointFloat& lhs, s64 scalar) {
    s64 result = static_cast<s64>(lhs.raw_) * scalar;
    check_overflow(result, "operator*");
    return FixedPointFloat(static_cast<s32>(result));
}

FixedPointFloat operator*(s64 scalar, const FixedPointFloat& rhs) {
    return rhs * scalar;
}

FixedPointFloat operator>>(const FixedPointFloat& lhs, int shift) {
    return FixedPointFloat(lhs.raw_ >> shift);
}

} // namespace n64::rdp
