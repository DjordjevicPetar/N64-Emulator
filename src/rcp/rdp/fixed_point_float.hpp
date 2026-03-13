#pragma once

#include "../../utils/types.hpp"
#include <algorithm>
#include <stdexcept>
#include <cstdio>

namespace n64::rdp {

class FixedPointFloat {
public:
    FixedPointFloat();
    FixedPointFloat(s32 raw);
    FixedPointFloat(u16 int_part, u16 frac_part, u8 int_size, u8 frac_size, bool is_signed);

    [[nodiscard]] s32 integer() const;
    [[nodiscard]] u16 frac() const { return static_cast<u16>(raw_ & 0xFFFF); }
    [[nodiscard]] s32 raw() const { return raw_; }

    FixedPointFloat& operator+=(const FixedPointFloat& rhs);
    FixedPointFloat& operator-=(const FixedPointFloat& rhs);
    FixedPointFloat& operator++();
    FixedPointFloat operator++(int);

    friend FixedPointFloat operator+(FixedPointFloat lhs, const FixedPointFloat& rhs);
    friend FixedPointFloat operator-(FixedPointFloat lhs, const FixedPointFloat& rhs);
    friend FixedPointFloat operator*(const FixedPointFloat& lhs, s64 scalar);
    friend FixedPointFloat operator*(s64 scalar, const FixedPointFloat& rhs);
    friend FixedPointFloat operator>>(const FixedPointFloat& lhs, int shift);

    friend bool operator==(const FixedPointFloat& lhs, const FixedPointFloat& rhs) { return lhs.raw_ == rhs.raw_; }
    friend bool operator!=(const FixedPointFloat& lhs, const FixedPointFloat& rhs) { return lhs.raw_ != rhs.raw_; }
    friend bool operator<(const FixedPointFloat& lhs, const FixedPointFloat& rhs) { return lhs.raw_ < rhs.raw_; }
    friend bool operator>(const FixedPointFloat& lhs, const FixedPointFloat& rhs) { return lhs.raw_ > rhs.raw_; }
    friend bool operator<=(const FixedPointFloat& lhs, const FixedPointFloat& rhs) { return lhs.raw_ <= rhs.raw_; }
    friend bool operator>=(const FixedPointFloat& lhs, const FixedPointFloat& rhs) { return lhs.raw_ >= rhs.raw_; }

private:
    s32 raw_ = 0;
    u8 integer_size_ = 15;
    bool is_signed_ = true;
};

} // namespace n64::rdp
