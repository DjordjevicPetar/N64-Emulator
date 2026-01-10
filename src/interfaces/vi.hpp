#pragma once

#include "../utils/types.hpp"
#include "../memory/memory.hpp"

namespace n64::interfaces {

class VI : public Memory {
public:
    VI();
    ~VI();

    [[nodiscard]] u32 read_memory(u32 address) override;
    void write_memory(u32 address, u32 value) override;
};
}