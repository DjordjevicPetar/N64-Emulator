#include "si.hpp"
#include <stdexcept>
#include <string>

namespace n64::interfaces {

SI::SI(MI& mi)
    : mi_(mi)
    , dram_addr_(0)
    , pif_ad_rd64b_(0)
    , pif_ad_wr4b_(0)
    , pif_ad_wr64b_(0)
    , pif_ad_rd4b_(0)
    , status_(0)
{
}
SI::~SI() {}

template<typename T>
T SI::read(u32 address) const {
    return read_register(address);
}

template<typename T>
void SI::write(u32 address, T value) {
    write_register(address, value);
}

u32 SI::read_register(u32 address) const {
    switch (address) {
        case SI_REGISTERS_ADDRESS::SI_DRAM_ADDR:
            return dram_addr_;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_RD64B:
            return pif_ad_rd64b_;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_WR4B:
            return pif_ad_wr4b_;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_WR64B:
            return pif_ad_wr64b_;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_RD4B:
            return pif_ad_rd4b_;
        case SI_REGISTERS_ADDRESS::SI_STATUS:
            return status_;
        default:
            throw std::runtime_error("Invalid SI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

void SI::write_register(u32 address, u32 value) {
    // TODO: SI DMA is completely unimplemented - critical for controller input!
    switch (address) {
        case SI_REGISTERS_ADDRESS::SI_DRAM_ADDR:
            dram_addr_ = value & 0x00FFFFFF;
            break;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_RD64B:
            pif_ad_rd64b_ = value & 0x000007FC;
            // TODO: Trigger 64-byte DMA read from PIF RAM to RDRAM
            // TODO: Set SI_STATUS busy bit during transfer
            // TODO: Generate SI interrupt when DMA completes
            break;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_WR4B:
            pif_ad_wr4b_ = value;
            // TODO: Trigger 4-byte DMA write from RDRAM to PIF RAM
            break;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_WR64B:
            pif_ad_wr64b_ = value;
            // TODO: Trigger 64-byte DMA write from RDRAM to PIF RAM
            // TODO: This triggers PIF command processing (controller polling, etc.)
            break;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_RD4B:
            pif_ad_rd4b_ = value;
            // TODO: Trigger 4-byte DMA read from PIF RAM to RDRAM
            break;
        case SI_REGISTERS_ADDRESS::SI_STATUS:
            // Writing any value clears SI interrupt
            mi_.clear_interrupt(MI_INTERRUPT_SI);
            break;
        default:
            throw std::runtime_error("Invalid SI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

template u8 SI::read<u8>(u32) const;
template u16 SI::read<u16>(u32) const;
template u32 SI::read<u32>(u32) const;
template u64 SI::read<u64>(u32) const;
template void SI::write<u8>(u32, u8);
template void SI::write<u16>(u32, u16);
template void SI::write<u32>(u32, u32);
template void SI::write<u64>(u32, u64);

} // namespace n64::interfaces
