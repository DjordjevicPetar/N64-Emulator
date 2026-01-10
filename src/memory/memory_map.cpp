#include "memory_map.hpp"

namespace n64::memory::memory_map {

MemoryMap::MemoryMap()
: rdram(), rom(), ai(), mi(), vi(), si(), ri(), pi()
{
    memory_regions = {
        {RDRAM_START_ADDRESS, RDRAM_END_ADDRESS, [this] { return rdram; }},
        {RSP_START_ADDRESS, RSP_END_ADDRESS, [this] { return rsp; }},
        {RDP_START_ADDRESS, RDP_END_ADDRESS, [this] { return rdp; }},
        {MI_START_ADDRESS, MI_END_ADDRESS, [this] { return mi; }},
        {VI_START_ADDRESS, VI_END_ADDRESS, [this] { return vi; }},
        {AI_START_ADDRESS, AI_END_ADDRESS, [this] { return ai; }},
        {PI_START_ADDRESS, PI_END_ADDRESS, [this] { return pi; }},
        {RI_START_ADDRESS, RI_END_ADDRESS, [this] { return ri; }},
        {SI_START_ADDRESS, SI_END_ADDRESS, [this] { return si; }},
    };
}

MemoryMap::~MemoryMap()
{
}

u32 MemoryMap::read_memory(u32 address)
{
    return map_delegation(address).read_memory(address);
}

void MemoryMap::write_memory(u32 address, u32 value)
{
    map_delegation(address).write_memory(address, value);
}

Memory& MemoryMap::map_delegation(u32 address)
{
    for (const auto& region : memory_regions) {
        if (address >= region.start_address && address <= region.end_address) {
            return region.get_memory();
        }
    }

    throw std::runtime_error("Memory map delegation failed for address: " + std::to_string(address));
}
}