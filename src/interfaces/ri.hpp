#pragma once

#include "../utils/types.hpp"

namespace n64::interfaces {

class RI {
public:
    RI();
    ~RI();

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);
};
}