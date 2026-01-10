#pragma once

#include "../../utils/types.hpp"
#include "../../memory/memory.hpp"

namespace n64::rcp {

class RDP : public Memory {
public:
    RDP();
    ~RDP();

    [[nodiscard]] u32 read_memory(u32 address) override;
    void write_memory(u32 address, u32 value) override;
};
}