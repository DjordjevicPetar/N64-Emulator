#pragma once

#include <vector>
#include "../utils/types.hpp"
#include "memory_constants.hpp"

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

class RDRAM {
public:
    RDRAM();
    ~RDRAM();

    template <typename T>
    [[nodiscard]] T read_memory(u32 address) const;

    template <typename T>
    void write_memory(u32 address, T value);

    [[nodiscard]] u32 read_register(RDRAM_REGISTERS_ADDRESS address) const;
    void write_register(RDRAM_REGISTERS_ADDRESS address, u32 value);

private:
    std::vector<u8> memory_;  // 8MB on heap, not stack!
    u32 device_type_;
    u32 device_id_;
    u32 delay_;
    u32 mode_;
    u32 ref_interval_;
    u32 ref_row_;
    u32 ras_interval_;
    u32 min_interval_;
    u32 address_select_;
    u32 device_manufacturer_;
    u32 row_;
};
}