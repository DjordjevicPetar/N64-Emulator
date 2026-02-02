#pragma once

#include "../../utils/types.hpp"
#include "../mi.hpp"
#include "vi_registers.hpp"
#include "vi_renderer.hpp"

namespace n64::memory {
class RDRAM;
}

namespace n64::interfaces {

class VI {
public:
    VI(MI& mi, memory::RDRAM& rdram);
    ~VI();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

    // Accessors for registers
    [[nodiscard]] const VICtrl& ctrl() const { return ctrl_; }
    [[nodiscard]] const VIOrigin& origin() const { return origin_; }
    [[nodiscard]] const VIWidth& width() const { return width_; }
    [[nodiscard]] const VIVCurrent& v_current() const { return v_current_; }

    void process_passed_cycles(u32 cycles);
    bool handle_events() { return renderer_.handle_events(); }

private:
    MI& mi_;
    VIRenderer renderer_;
    u64 cycles_counter_;

    VICtrl ctrl_;
    VIOrigin origin_;
    VIWidth width_;
    VIVIntr v_intr_;
    VIVCurrent v_current_;
    VIBurst burst_;
    VIVTotal v_total_;
    VIHTotal h_total_;
    VIHTotalLeap h_total_leap_;
    VIHVideo h_video_;
    VIVVideo v_video_;
    VIVBurst v_burst_;
    VIXScale x_scale_;
    VIYScale y_scale_;
    VITestAddr test_addr_;
    VIStagedData staged_data_;
};

} // namespace n64::interfaces
