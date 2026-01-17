#pragma once

#include "../utils/types.hpp"
#include "mi.hpp"

namespace n64::interfaces {

enum SI_REGISTERS_ADDRESS : u32 {
    SI_DRAM_ADDR = 0x04800000,
    SI_PIF_AD_RD64B = 0x04800004,
    SI_PIF_AD_WR4B = 0x04800008,
    SI_PIF_AD_WR64B = 0x04800010,
    SI_PIF_AD_RD4B = 0x04800014,
    SI_STATUS = 0x04800018,
};

class SI {
public:
    SI(MI& mi);
    ~SI();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

private:
    MI& mi_;
    
    u32 dram_addr_;
    u32 pif_ad_rd64b_;
    u32 pif_ad_wr4b_;
    u32 pif_ad_wr64b_;
    u32 pif_ad_rd4b_;
    u32 status_;
};
}