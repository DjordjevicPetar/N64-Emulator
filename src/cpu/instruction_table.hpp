#pragma once

#include <array>
#include <functional>

#include "../utils/types.hpp"
#include "instruction.hpp"

namespace n64::cpu {

class VR4300;

enum class InstructionType {
    REGISTER_TYPE,
    IMMEDIATE_TYPE,
    JUMP_TYPE,
    BRANCH_TYPE,
    SPECIAL_TYPE,
};

struct InstructionEntry {
    const char* name;
    InstructionType type;
    std::function<u32(VR4300&, const Instruction&)> execute;
};

class InstructionTable {
public:
    InstructionTable();
    ~InstructionTable() = default;

    [[nodiscard]] const InstructionEntry& lookup(const Instruction& instruction) const;

private:
    std::array<InstructionEntry, 64> main_table_;
    std::array<InstructionEntry, 64> special_table_;
    std::array<InstructionEntry, 32> regimm_table_;

    std::array<InstructionEntry, 32> cop0_table_;
    std::array<InstructionEntry, 4> cop0_bc_table_;
    std::array<InstructionEntry, 64> cop0_cofun_table_;

    std::array<InstructionEntry, 32> cop1_table_;
    std::array<InstructionEntry, 4> cop1_bc_table_;
    std::array<InstructionEntry, 64> cop1_cofun_table_;
};

}