#pragma once

#include <array>
#include <functional>
#include "../../utils/types.hpp"
#include "rsp_instruction.hpp"
#include "su_ops.hpp"
#include "vu_ops.hpp"

namespace n64::rcp {

enum class RSPInstructionType {
    SCALAR_TYPE,
    VECTOR_TYPE
};

struct RSPInstructionEntry {
    const char* name;
    RSPInstructionType type;
    std::function<u8(RSP&, const RSPInstruction&)> execute;
};

class RSPInstructionTable {
public:
    RSPInstructionTable();
    ~RSPInstructionTable() = default;

    const RSPInstructionEntry& lookup(const RSPInstruction& instruction) const;
private:
    std::array<RSPInstructionEntry, 64> main_table_;
    std::array<RSPInstructionEntry, 64> special_table_;
    std::array<RSPInstructionEntry, 32> regimm_table_;

    std::array<RSPInstructionEntry, 32> cop0_table_;

    std::array<RSPInstructionEntry, 32> cop2_move_table_;
    std::array<RSPInstructionEntry, 32> cop2_compute_table_;
    std::array<RSPInstructionEntry, 32> lwc2_table_;
    std::array<RSPInstructionEntry, 32> swc2_table_;

};

}