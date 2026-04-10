#pragma once

#include "../../utils/types.hpp"
#include "../mi.hpp"
#include "../../memory/rdram.hpp"
#include "../../memory/pif.hpp"
#include "si_registers.hpp"

namespace n64::interfaces {

class SI {
public:
    SI(MI& mi, memory::RDRAM& rdram, memory::PIF& pif);
    ~SI();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

private:
    MI& mi_;
    memory::RDRAM& rdram_;
    memory::PIF& pif_;
    
    SIDramAddr dram_addr_;
    SIPIFAdRD64B pif_ad_rd64b_;
    SIPIFAdWR4B pif_ad_wr4b_;
    SIPIFAdWR64B pif_ad_wr64b_;
    SIPIFAdRD4B pif_ad_rd4b_;
    SIStatus status_;
};
}