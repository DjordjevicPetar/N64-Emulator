#pragma once

#include "../../utils/types.hpp"
#include "memory_constants.hpp"

namespace n64::memory {

class Memory {
public:
    virtual ~Memory() = default;

    [[nodiscard]] virtual u64 read_memory(u32 address, IO_SIZE size) = 0;
    virtual void write_memory(u32 address, u64 value, IO_SIZE size) = 0;

};
}
