#pragma once

#include "../utils/types.hpp"
#include "../memory/memory.hpp"

namespace n64::interfaces {

class AI : public Memory {
public:
    AI();
    ~AI();

    [[nodiscard]] u32 read_memory(u32 address) override;
    void write_memory(u32 address, u32 value) override;
};
}