#pragma once

#include <array>

#include "../utils/types.hpp"
#include "../memory/memory_map.hpp"
#include "instruction.hpp"
#include "instruction_table.hpp"
#include "cp0.hpp"
#include "cp1.hpp"

namespace n64::cpu {

class VR4300 {
public:
    VR4300(memory::MemoryMap& memory);
    ~VR4300() = default;

    u32 execute_next_instruction();
    void delay_branch(u64 target);
    u32 translate_address(u64 virtual_address, bool is_write);

    // Register access
    [[nodiscard]] u64 gpr(u8 index) const { return gpr_[index]; }
    void set_gpr(u8 index, u64 value) { if (index != 0 && !exception_pending_) gpr_[index] = value; }
    
    [[nodiscard]] u64 pc() const { return pc_; }
    void set_pc(u64 value) { pc_ = value; }
    
    [[nodiscard]] u64 hi() const { return hi_; }
    [[nodiscard]] u64 lo() const { return lo_; }
    void set_hi(u64 value) { if (!exception_pending_) hi_ = value; }
    void set_lo(u64 value) { if (!exception_pending_) lo_ = value; }

    [[nodiscard]] bool get_LLbit() const { return ll_bit_; }
    void set_LLbit(bool value) { ll_bit_ = value; }

    void set_interrupt_inhibit() { interrupt_inhibit_ = true; }

    [[nodiscard]] bool in_delay_slot() const { return branch_pending_ || should_branch;}
    void reset_delay_slot() { should_branch = false; branch_pending_ = false; }

    void set_exception_pending() { exception_pending_ = true; }
    [[nodiscard]] bool exception_pending() const { return exception_pending_; }

    bool check_address_exception(u64 address, u8 word_size, bool is_load);

    // CP0 access
    [[nodiscard]] CP0& cp0() { return cp0_; }
    [[nodiscard]] const CP0& cp0() const { return cp0_; }

    // CP1 (FPU) access
    [[nodiscard]] CP1& cp1() { return cp1_; }
    [[nodiscard]] const CP1& cp1() const { return cp1_; }

    // Memory access
    template <typename T>
    T read_memory(u64 address);
    template <typename T>
    void write_memory(u64 address, T value);

private:
    // Registers
    std::array<u64, 32> gpr_{};
    u64 pc_ = 0;
    u64 hi_ = 0;
    u64 lo_ = 0;
    bool ll_bit_ = false;

    // Components
    memory::MemoryMap& memory_;
    InstructionTable instruction_table_;
    Instruction current_instruction_{0};
    CP0 cp0_{*this};
    CP1 cp1_;

    // Branch delay slot
    bool branch_pending_ = false;
    u64 branch_target_ = 0;
    bool should_branch = false;

    // ERET inhibits interrupts for 1 instruction (VR4300 hardware behavior)
    bool interrupt_inhibit_ = false;

    // Set when an exception is raised during instruction execution (e.g., TLB miss
    // inside translate_address). Prevents the instruction from continuing to modify
    // registers/memory after the exception, and prevents branch/interrupt handling.
    bool exception_pending_ = false;

    void read_next_instruction();
};

}
