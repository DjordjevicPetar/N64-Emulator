#pragma once

#include "memory_constants.hpp"
#include "memory.hpp"

namespace n64::memory::memory_map {

class MemoryMap : public Memory {
public:
    MemoryMap();
    ~MemoryMap();


    [[nodiscard]] u32 read_memory(u32 address) override;
    void write_memory(u32 address, u32 value) override;

private:

    struct MemoryRegion {
        u32 start_address;
        u32 end_address;
        std::function<Memory&()> get_memory;
    };

    std::array<MemoryRegion, 8> memory_regions;

    [[nodiscard]] Memory& map_delegation(u32 address);

    RDRAM rdram;
    ROM rom;
    interfaces::AI ai;
    interfaces::MI mi;
    interfaces::VI vi;
    interfaces::SI si;
    interfaces::RI ri;
    interfaces::PI pi;
};
}