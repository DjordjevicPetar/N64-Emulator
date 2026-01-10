#pragma once

#include "../utils/types.hpp"
#include "../memory/memory.hpp"

namespace n64::memory {

enum class RDRAM_REGISTERS_ADDRESS {

    RDRAM_REGISTER_DEVICE_TYPE = 0x00,
    RDRAM_REGISTER_DEVICE_ID = 0x04,
    RDRAM_REGISTER_DELAY = 0x08,
    RDRAM_REGISTER_MODE = 0x0C,
    RDRAM_REGISTER_REF_INTERVAL = 0x10,
    RDRAM_REGISTER_REF_ROW = 0x14,
    RDRAM_REGISTER_RAS_INTERVAL = 0x18,
    RDRAM_REGISTER_MIN_INTERVAL = 0x1C,
    RDRAM_REGISTER_ADDRESS_SELECT = 0x20,
    RDRAM_REGISTER_DEVICE_MANUFACTURER = 0x24,
    RDRAM_REGISTER_ROW = 0x200
};

class RDRAM : public Memory {
public:
    RDRAM();
    ~RDRAM();

    [[nodiscard]] u32 read_memory(u32 address) override;
    void write_memory(u32 address, u32 value) override;

private:
    std::array<u32, RDRAM_MEMORY_SIZE> memory;

    u32 translate_address(u32 address) const;
};
}