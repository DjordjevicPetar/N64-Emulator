#pragma once

#include "../utils/types.hpp"
#include "../memory/memory.hpp"

namespace n64::memory {

class ROM : public Memory {
public:
    ROM();
    ~ROM();

    [[nodiscard]] u32 read_memory(u32 address) override;
    void write_memory(u32 address, u32 value) override;
};
}