#pragma once

#include "../utils/types.hpp"
#include <array>

namespace n64::rcp {

class VU {
public:
    VU();
    ~VU();
private:
    std::array<u128, 32> gpr_;
    u64 accumulator_;
    u16 vcc_;
    u16 vco_;
    u8 vce_;
};

}