#pragma once

#include <array>

#include "../utils/types.hpp"
#include "../memory/memory.hpp"
#include "instruction.hpp"
#include "instruction_table.hpp"
#include "cp0.hpp"

namespace n64::cpu {

class VR4300 {
public:
    VR4300(Memory& memory);
    ~VR4300() = default;

    void execute_next_instruction();
    void delay_branch(u64 target);
    [[nodiscard]] u32 translate_address(u64 virtual_address) const;

    // Register access
    [[nodiscard]] u64 gpr(u8 index) const { return gpr_[index]; }
    void set_gpr(u8 index, u64 value) { if (index != 0) gpr_[index] = value; }
    
    [[nodiscard]] u64 pc() const { return pc_; }
    void set_pc(u64 value) { pc_ = value; }
    
    [[nodiscard]] u64 hi() const { return hi_; }
    [[nodiscard]] u64 lo() const { return lo_; }
    void set_hi(u64 value) { hi_ = value; }
    void set_lo(u64 value) { lo_ = value; }

    [[nodiscard]] bool get_LLbit() const { return ll_bit_; }
    void set_LLbit(bool value) { ll_bit_ = value; }

    // CP0 access
    [[nodiscard]] CP0& cp0() { return cp0_; }
    [[nodiscard]] const CP0& cp0() const { return cp0_; }

private:
    // Registers
    std::array<u64, 32> gpr_{};
    u64 pc_ = 0;
    u64 hi_ = 0;
    u64 lo_ = 0;
    bool ll_bit_ = false;

    // Components
    Memory& memory_;
    InstructionTable instruction_table_;
    Instruction current_instruction_{0};
    CP0 cp0_;

    // Branch delay slot
    bool branch_pending_ = false;
    u64 branch_target_ = 0;

    void read_next_instruction();

    [[nodiscard]] u64 read_memory(u64 address, IO_SIZE size) const;
    void write_memory(u64 address, u64 value, IO_SIZE size);
};

}
