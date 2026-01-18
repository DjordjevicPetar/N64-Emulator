#pragma once

#include "../utils/types.hpp"
#include <array>

namespace n64::rcp {

class SU {
public:
    SU();
    ~SU();

    u32 read_gpr(u32 index) const;
    void write_gpr(u32 index, u32 value);

private:
    std::array<u32, 32> gpr_;
};

}