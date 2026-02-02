#include "pi.hpp"
#include <stdexcept>
#include <string>

namespace n64::interfaces {

PI::PI()
    : dram_addr_(0)
    , cart_addr_(0)
    , rd_len_(0)
    , wr_len_(0)
    , status_(0)
    , bsd_dom1_lat_(0)
    , bsd_dom1_pwd_(0)
    , bsd_dom1_pgs_(0)
    , bsd_dom1_rls_(0)
    , bsd_dom2_lat_(0)
    , bsd_dom2_pwd_(0)
    , bsd_dom2_pgs_(0)
    , bsd_dom2_rls_(0)
{
}
PI::~PI() {}

u32 PI::read_register(u32 address) const {
    switch (address) {
        case PI_REGISTERS_ADDRESS::PI_DRAM_ADDR:
            return dram_addr_;
        case PI_REGISTERS_ADDRESS::PI_CART_ADDR:
            return cart_addr_;
        case PI_REGISTERS_ADDRESS::PI_RD_LEN:
            return rd_len_;
        case PI_REGISTERS_ADDRESS::PI_WR_LEN:
            return wr_len_;
        case PI_REGISTERS_ADDRESS::PI_STATUS:
            return status_;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM1_LAT:
            return bsd_dom1_lat_;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM1_PWD:
            return bsd_dom1_pwd_;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM1_PGS:
            return bsd_dom1_pgs_;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM1_RLS:
            return bsd_dom1_rls_;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM2_LAT:
            return bsd_dom2_lat_;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM2_PWD:
            return bsd_dom2_pwd_;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM2_PGS:
            return bsd_dom2_pgs_;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM2_RLS:
            return bsd_dom2_rls_;
        default:
            throw std::runtime_error("Invalid PI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

void PI::write_register(u32 address, u32 value) {
    // TODO: PI DMA is completely unimplemented - critical for loading game data!
    switch (address) {
        case PI_REGISTERS_ADDRESS::PI_DRAM_ADDR:
            dram_addr_ = value & 0x00FFFFFE;
            break;
        case PI_REGISTERS_ADDRESS::PI_CART_ADDR:
            cart_addr_ = value & 0xFFFFFFFE;
            break;
        case PI_REGISTERS_ADDRESS::PI_RD_LEN:
            rd_len_ = value & 0x00FFFFFF;
            // TODO: Trigger read DMA from ROM (cart_addr) to RDRAM (dram_addr)
            // TODO: Set PI_STATUS busy bit during transfer
            // TODO: Generate PI interrupt when DMA completes
            // TODO: Implement DMA timing based on BSD domain settings
            break;
        case PI_REGISTERS_ADDRESS::PI_WR_LEN:
            wr_len_ = value & 0x00FFFFFF;
            // TODO: Trigger write DMA from RDRAM (dram_addr) to ROM/SRAM (cart_addr)
            // TODO: Handle writes to cartridge SRAM/FlashRAM
            break;
        case PI_REGISTERS_ADDRESS::PI_STATUS:
            // TODO: PI_STATUS is mostly read-only; writing should only clear interrupt/error bits
            // Bit 0: Reset DMA controller, Bit 1: Clear interrupt
            if (value & 0x02) {
                // TODO: Clear PI interrupt via MI
            }
            break;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM1_LAT:
            bsd_dom1_lat_ = value & 0x000000FF;
            break;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM1_PWD:
            bsd_dom1_pwd_ = value & 0x000000FF;
            break;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM1_PGS:
            bsd_dom1_pgs_ = value & 0x0000000F;
            break;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM1_RLS:
            bsd_dom1_rls_ = value & 0x00000003;
            break;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM2_LAT:
            bsd_dom2_lat_ = value & 0x000000FF;
            break;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM2_PWD:
            bsd_dom2_pwd_ = value & 0x000000FF;
            break;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM2_PGS:
            bsd_dom2_pgs_ = value & 0x0000000F;
            break;
        case PI_REGISTERS_ADDRESS::PI_BSD_DOM2_RLS:
            bsd_dom2_rls_ = value & 0x00000003;
            break;
        default:
            throw std::runtime_error("Invalid PI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

} // namespace n64::interfaces
