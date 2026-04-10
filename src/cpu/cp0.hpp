#pragma once

#include <array>
#include "cp0_registers.hpp"

namespace n64::cpu {

class VR4300;

class CP0 {
public:
    CP0(VR4300& cpu);
    ~CP0() = default;

    // Generic register access (for MTC0/MFC0)
    void set_reg(u8 index, u64 value);
    void set_reg(CP0Reg reg, u64 value) { set_reg(static_cast<u8>(reg), value); }
    [[nodiscard]] u64 get_reg(u8 index) const;
    [[nodiscard]] u64 get_reg(CP0Reg reg) const { return get_reg(static_cast<u8>(reg)); }

    // Address translation
    u32 translate_address(u64 virtual_address, bool is_write);

    // TLB operations
    void write_tlb_entry(u32 index);
    void read_tlb_entry(u32 index);
    void probe_tlb();

    // Direct register accessors
    [[nodiscard]] CP0Status& status() { return status_; }
    [[nodiscard]] const CP0Status& status() const { return status_; }
    [[nodiscard]] CP0Cause& cause() { return cause_; }
    [[nodiscard]] const CP0Cause& cause() const { return cause_; }

    [[nodiscard]] bool get_fr_bit() const { return status_.fr; }
    [[nodiscard]] u64 epc() const { return epc_; }
    [[nodiscard]] u64 error_epc() const { return error_epc_; }    

    void handle_random_register();
    void handle_count_register(u32 cycles);
    void check_interrupts();
    void raise_exception(ExceptionCode code, u8 ce = 0);
    void raise_address_exception(ExceptionCode code, u64 address);
    void set_mi_interrupt(bool active) { cause_.ip = set_bit(cause_.ip, 2, active); }

private:
    u32 tlb_lookup(u64 virtual_address, bool is_write);
    void raise_tlb_exception(ExceptionCode code, u64 virtual_address);

    // TLB (32 entries)
    std::array<TLBEntry, 32> tlb_{};

    // TLB registers
    CP0Index index_;
    CP0Random random_;
    CP0EntryLo entry_lo0_;
    CP0EntryLo entry_lo1_;
    CP0Context context_;
    CP0PageMask page_mask_;
    CP0Wired wired_;
    
    // Exception registers
    u64 bad_vaddr_;
    u32 count_;
    CP0EntryHi entry_hi_;
    u32 compare_;
    CP0Status status_;
    CP0Cause cause_;
    u64 epc_;
    
    // Configuration registers
    CP0PRId prid_;
    CP0Config config_;
    u32 ll_addr_;
    CP0WatchLo watch_lo_;
    CP0WatchHi watch_hi_;
    CP0XContext xcontext_;
    
    // Cache/error registers
    CP0ParityError parity_error_;
    u32 cache_error_;
    CP0TagLo tag_lo_;
    u32 tag_hi_;
    u64 error_epc_;

    VR4300& cpu_;

    bool count_odd_ = false;

    [[nodiscard]] u64 get_exception_vector_address(ExceptionCode code, bool old_exl) const;
};

} // namespace n64::cpu
