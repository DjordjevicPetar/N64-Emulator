#include "si.hpp"
#include <cstdio>

namespace n64::interfaces {

SI::SI(MI& mi, memory::RDRAM& rdram, memory::PIF& pif)
    : mi_(mi)
    , rdram_(rdram)
    , pif_(pif)
    , dram_addr_{.raw = 0}
    , pif_ad_rd64b_{.raw = 0}
    , pif_ad_wr4b_{.raw = 0}
    , pif_ad_wr64b_{.raw = 0}
    , pif_ad_rd4b_{.raw = 0}
    , status_{.raw = 0}
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
            return dram_addr_.raw;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_RD64B:
            return pif_ad_rd64b_.raw;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_WR4B:
            return pif_ad_wr4b_.raw;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_WR64B:
            return pif_ad_wr64b_.raw;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_RD4B:
            return pif_ad_rd4b_.raw;
        case SI_REGISTERS_ADDRESS::SI_STATUS:
            return status_.raw;
        default:
            return 0;
    }
}

void SI::write_register(u32 address, u32 value) {
    switch (address) {
        case SI_REGISTERS_ADDRESS::SI_DRAM_ADDR:
            dram_addr_.raw = value & 0x00FFFFFF;
            break;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_RD64B:
            fprintf(stderr, "[SI] DMA Read (PIF->RDRAM) dram=0x%08X\n", dram_addr_.address);
            pif_ad_rd64b_.raw = value & 0x000007FC;
            pif_.dma_read_to_rdram(rdram_, dram_addr_.address);
            mi_.set_interrupt(MI_INTERRUPT_SI);
            status_.interrupt = 1;
            break;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_WR4B:
            // !! Broken
            pif_ad_wr4b_.raw = value;
            break;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_WR64B:
            fprintf(stderr, "[SI] DMA Write (RDRAM->PIF) dram=0x%08X\n", dram_addr_.address);
            pif_ad_wr64b_.raw = value;
            pif_.dma_write_from_rdram(rdram_, dram_addr_.address);
            mi_.set_interrupt(MI_INTERRUPT_SI);
            status_.interrupt = 1;
            break;
        case SI_REGISTERS_ADDRESS::SI_PIF_AD_RD4B:
            // !! Broken
            pif_ad_rd4b_.raw = value;
            break;
        case SI_REGISTERS_ADDRESS::SI_STATUS:
            // Writing any value clears SI interrupt
            mi_.clear_interrupt(MI_INTERRUPT_SI);
            break;
        default:
            return;
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
