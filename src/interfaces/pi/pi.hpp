#pragma once

#include "../../utils/types.hpp"
#include "../mi.hpp"
#include "pi_registers.hpp"

namespace n64::memory {
    class ROM;
    class RDRAM;
}

namespace n64::interfaces {

constexpr float PI_CYCLE_PER_CYCLE_FP = 2.0f / 3.0f;

class PI {
public:
    explicit PI(MI& mi);
    ~PI();

    // Must be called after ROM and RDRAM are constructed
    void set_dma_targets(memory::ROM& rom, memory::RDRAM& rdram);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

    void process_passed_cycles(u32 cycles);

    // Accessors for registers
    [[nodiscard]] const PIDramAddr& dram_addr() const { return dram_addr_; }
    [[nodiscard]] const PICartAddr& cart_addr() const { return cart_addr_; }
    [[nodiscard]] const PIStatus& status() const { return status_; }

private:
    // Returns 1 for Domain 1, 2 for Domain 2
    [[nodiscard]] u8 get_address_domain(u32 address) const;
    void set_counters();
    void transfer_page();

    MI& mi_;
    memory::ROM* rom_ = nullptr;
    memory::RDRAM* rdram_ = nullptr;

    // Registers
    PIDramAddr dram_addr_;
    PICartAddr cart_addr_;
    PILen rd_len_;
    PILen wr_len_;
    PIStatus status_;
    
    // Domain 1 BSD timing
    PIBsdLat bsd_dom1_lat_;
    PIBsdPwd bsd_dom1_pwd_;
    PIBsdPgs bsd_dom1_pgs_;
    PIBsdRls bsd_dom1_rls_;
    
    // Domain 2 BSD timing
    PIBsdLat bsd_dom2_lat_;
    PIBsdPwd bsd_dom2_pwd_;
    PIBsdPgs bsd_dom2_pgs_;
    PIBsdRls bsd_dom2_rls_;

    // DMA state
    bool dma_busy_;
    float pi_cycle_accumulator_;
    u32 latency_counter_;
    u32 pulse_width_counter_;
    u32 release_counter_;
    bool is_reading_;
    bool is_writing_;
};

} // namespace n64::interfaces
