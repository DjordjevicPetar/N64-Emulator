#include "memory_map.hpp"
#include <stdexcept>
#include <string>

namespace n64::memory {

MemoryMap::MemoryMap(
    RDRAM& rdram,
    ROM& rom,
    interfaces::MI& mi,
    rdp::RDP& rdp,
    rcp::RSP& rsp,
    interfaces::AI& ai,
    interfaces::VI& vi,
    interfaces::SI& si,
    interfaces::RI& ri,
    interfaces::PI& pi,
    PIF& pif
)
    : rdram_(rdram)
    , rom_(rom)
    , mi_(mi)
    , rdp_(rdp)
    , rsp_(rsp)
    , ai_(ai)
    , vi_(vi)
    , si_(si)
    , ri_(ri)
    , pi_(pi)
    , pif_(pif)
{
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

    // RDP
    if (address >= RDP_START_ADDRESS && address <= RDP_END_ADDRESS) {
        return rdp_.read<T>(address);
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

    // RDP
    if (address >= RDP_START_ADDRESS && address <= RDP_END_ADDRESS) {
        rdp_.write<T>(address, value);
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
