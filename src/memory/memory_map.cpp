#include "memory_map.hpp"
#include <stdexcept>
#include <algorithm>

namespace n64::memory {

MemoryMap::MemoryMap(const std::string& rom_path)
    : rdram_()
    , rom_(pi_, rom_path)
    , mi_()
    , rsp_(mi_)
    , ai_(mi_)
    , vi_(mi_)
    , si_(mi_)
    , ri_()
    , pi_()
    , pif_()
{
}

MemoryMap::~MemoryMap()
{
}

u32 MemoryMap::boot()
{
    u32 entry_point = rom_.parse_header();
    
    // Simulate what IPL3 bootcode does:
    // Copy game code from ROM to RDRAM
    // ROM offset 0x1000 (after header + bootcode) -> RDRAM at entry point
    
    constexpr u32 ROM_CODE_OFFSET = 0x1000;  // After header (0x40) and bootcode (0xFC0)
    u32 rdram_dest = entry_point & 0x1FFFFFFF;  // Convert KSEG0/1 to physical
    
    // Calculate how much to copy (ROM size minus header, limited to RDRAM size)
    size_t copy_size = std::min(rom_.size() - ROM_CODE_OFFSET, 
                                static_cast<size_t>(RDRAM_MEMORY_SIZE - rdram_dest));
    
    for (size_t i = 0; i < copy_size; ++i) {
        rdram_.write_memory<u8>(rdram_dest + i, rom_.read<u8>(ROM_START_ADDRESS + ROM_CODE_OFFSET + i));
    }
    
    return entry_point;
}

template<typename T>
T MemoryMap::read(u32 address)
{
    // RDRAM
    if (address >= RDRAM_START_ADDRESS && address <= RDRAM_END_ADDRESS) {
        return rdram_.read_memory<T>(address);
    }

    // RSP
    if (address >= RSP_START_ADDRESS && address <= RSP_END_ADDRESS) {
        return rsp_.read<T>(address);
    }

    // MI
    if (address >= MI_START_ADDRESS && address <= MI_END_ADDRESS) {
        return mi_.read_register(address);
    }

    // VI
    if (address >= VI_START_ADDRESS && address <= VI_END_ADDRESS) {
        return vi_.read_register(address);
    }

    // AI
    if (address >= AI_START_ADDRESS && address <= AI_END_ADDRESS) {
        return ai_.read_register(address);
    }

    // RI
    if (address >= RI_START_ADDRESS && address <= RI_END_ADDRESS) {
        return ri_.read_register(address);
    }
    
    // PI
    if (address >= PI_START_ADDRESS && address <= PI_END_ADDRESS) {
        return pi_.read_register(address);
    }

    // SI
    if (address >= SI_START_ADDRESS && address <= SI_END_ADDRESS) {
        return si_.read_register(address);
    }
    
    // ROM
    if (address >= ROM_START_ADDRESS && address <= ROM_END_ADDRESS) {
        return rom_.read<T>(address);
    }

    // PIF
    if (address >= PIF_START_ADDRESS && address <= PIF_END_ADDRESS) {
        return pif_.read<T>(address);
    }
    
    // TODO: Add other memory regions (RSP, RDP, MI, VI, AI, RI, SI)
    
    throw std::runtime_error("Unmapped memory read at address: " + std::to_string(address));
}

template<typename T>
void MemoryMap::write(u32 address, T value)
{
    // RDRAM
    if (address >= RDRAM_START_ADDRESS && address <= RDRAM_END_ADDRESS) {
        rdram_.write_memory<T>(address, value);
        return;
    }

    // RSP
    if (address >= RSP_START_ADDRESS && address <= RSP_END_ADDRESS) {
        rsp_.write<T>(address, value);
        return;
    }

    // MI
    if (address >= MI_START_ADDRESS && address <= MI_END_ADDRESS) {
        mi_.write_register(address, value);
        return;
    }

    // VI
    if (address >= VI_START_ADDRESS && address <= VI_END_ADDRESS) {
        vi_.write_register(address, value);
        return;
    }

    // AI
    if (address >= AI_START_ADDRESS && address <= AI_END_ADDRESS) {
        ai_.write_register(address, value);
        return;
    }

    // RI
    if (address >= RI_START_ADDRESS && address <= RI_END_ADDRESS) {
        ri_.write_register(address, value);
        return;
    }

    // SI
    if (address >= SI_START_ADDRESS && address <= SI_END_ADDRESS) {
        si_.write_register(address, value);
        return;
    }
    
    // PI
    if (address >= PI_START_ADDRESS && address <= PI_END_ADDRESS) {
        pi_.write_register(address, value);
        return;
    }
    
    // PIF
    if (address >= PIF_START_ADDRESS && address <= PIF_END_ADDRESS) {
        pif_.write<T>(address, value);
        return;
    }
    
    // TODO: Add other memory regions (ROM, RSP, RDP, MI, VI, AI, RI, SI)
    
    throw std::runtime_error("Unmapped memory write at address: " + std::to_string(address));
}

// Explicit template instantiations
template u8  MemoryMap::read<u8>(u32 address);
template u16 MemoryMap::read<u16>(u32 address);
template u32 MemoryMap::read<u32>(u32 address);
template u64 MemoryMap::read<u64>(u32 address);

template void MemoryMap::write<u8>(u32 address, u8 value);
template void MemoryMap::write<u16>(u32 address, u16 value);
template void MemoryMap::write<u32>(u32 address, u32 value);
template void MemoryMap::write<u64>(u32 address, u64 value);

}
