#include "instruction_table.hpp"
#include "cpu_ops.hpp"

namespace n64::cpu {

InstructionTable::InstructionTable()
{
    for (auto& entry : main_table_) {
        entry = {"UNKNOWN", InstructionType::REGISTER_TYPE, nullptr};
    }
    for (auto& entry : special_table_) {
        entry = {"UNKNOWN", InstructionType::REGISTER_TYPE, nullptr};
    }
    for (auto& entry : regimm_table_) {
        entry = {"UNKNOWN", InstructionType::IMMEDIATE_TYPE, nullptr};
    }
    for (auto& entry : cop0_table_) {
        entry = {"UNKNOWN", InstructionType::REGISTER_TYPE, nullptr};
    }
    for (auto& entry : cop0_bc_table_) {
        entry = {"UNKNOWN", InstructionType::BRANCH_TYPE, nullptr};
    }
    for (auto& entry : cop0_cofun_table_) {
        entry = {"UNKNOWN", InstructionType::SPECIAL_TYPE, nullptr};
    }
    for (auto& entry : cop1_table_) {
        entry = {"UNKNOWN", InstructionType::REGISTER_TYPE, nullptr};
    }
    for (auto& entry : cop1_bc_table_) {
        entry = {"UNKNOWN", InstructionType::BRANCH_TYPE, nullptr};
    }
    for (auto& entry : cop1_cofun_table_) {
        entry = {"UNKNOWN", InstructionType::SPECIAL_TYPE, nullptr};
    }

    // Main table
    main_table_[0x02] = {"J", InstructionType::JUMP_TYPE, J};
    main_table_[0x03] = {"JAL", InstructionType::JUMP_TYPE, JAL};
    main_table_[0x04] = {"BEQ", InstructionType::BRANCH_TYPE, BEQ};
    main_table_[0x05] = {"BNE", InstructionType::BRANCH_TYPE, BNE};
    main_table_[0x06] = {"BLEZ", InstructionType::BRANCH_TYPE, BLEZ};
    main_table_[0x07] = {"BGTZ", InstructionType::BRANCH_TYPE, BGTZ};
    main_table_[0x08] = {"ADDI", InstructionType::IMMEDIATE_TYPE, ADDI};
    main_table_[0x09] = {"ADDIU", InstructionType::IMMEDIATE_TYPE, ADDIU};
    main_table_[0x0A] = {"SLTI", InstructionType::IMMEDIATE_TYPE, SLTI};
    main_table_[0x0B] = {"SLTIU", InstructionType::IMMEDIATE_TYPE, SLTIU};
    main_table_[0x0C] = {"ANDI", InstructionType::IMMEDIATE_TYPE, ANDI};
    main_table_[0x0D] = {"ORI", InstructionType::IMMEDIATE_TYPE, ORI};
    main_table_[0x0E] = {"XORI", InstructionType::IMMEDIATE_TYPE, XORI};
    main_table_[0x0F] = {"LUI", InstructionType::IMMEDIATE_TYPE, LUI};
    main_table_[0x14] = {"BEQL", InstructionType::BRANCH_TYPE, BEQL};
    main_table_[0x15] = {"BNEL", InstructionType::BRANCH_TYPE, BNEL};
    main_table_[0x16] = {"BLEZL", InstructionType::BRANCH_TYPE, BLEZL};
    main_table_[0x17] = {"BGTZL", InstructionType::BRANCH_TYPE, BGTZL};
    main_table_[0x18] = {"DADDI", InstructionType::IMMEDIATE_TYPE, DADDI};
    main_table_[0x19] = {"DADDIU", InstructionType::IMMEDIATE_TYPE, DADDIU};
    main_table_[0x1A] = {"LDL", InstructionType::IMMEDIATE_TYPE, LDL};
    main_table_[0x1B] = {"LDR", InstructionType::IMMEDIATE_TYPE, LDR};
    main_table_[0x20] = {"LB", InstructionType::IMMEDIATE_TYPE, LB};
    main_table_[0x21] = {"LH", InstructionType::IMMEDIATE_TYPE, LH};
    main_table_[0x22] = {"LWL", InstructionType::IMMEDIATE_TYPE, LWL};
    main_table_[0x23] = {"LW", InstructionType::IMMEDIATE_TYPE, LW};
    main_table_[0x24] = {"LBU", InstructionType::IMMEDIATE_TYPE, LBU};
    main_table_[0x25] = {"LHU", InstructionType::IMMEDIATE_TYPE, LHU};
    main_table_[0x26] = {"LWR", InstructionType::IMMEDIATE_TYPE, LWR};
    main_table_[0x27] = {"LWU", InstructionType::IMMEDIATE_TYPE, LWU};
    main_table_[0x28] = {"SB", InstructionType::IMMEDIATE_TYPE, SB};
    main_table_[0x29] = {"SH", InstructionType::IMMEDIATE_TYPE, SH};
    main_table_[0x2A] = {"SWL", InstructionType::IMMEDIATE_TYPE, SWL};
    main_table_[0x2B] = {"SW", InstructionType::IMMEDIATE_TYPE, SW};
    main_table_[0x2C] = {"SDL", InstructionType::IMMEDIATE_TYPE, SDL};
    main_table_[0x2D] = {"SDR", InstructionType::IMMEDIATE_TYPE, SDR};
    main_table_[0x2E] = {"SWR", InstructionType::IMMEDIATE_TYPE, SWR};
    main_table_[0x2F] = {"CACHE", InstructionType::SPECIAL_TYPE, CACHE};
    main_table_[0x30] = {"LL", InstructionType::IMMEDIATE_TYPE, LL};
    main_table_[0x31] = {"LWC1", InstructionType::IMMEDIATE_TYPE, LWC1};
    main_table_[0x34] = {"LLD", InstructionType::IMMEDIATE_TYPE, LLD};
    main_table_[0x35] = {"LDC1", InstructionType::IMMEDIATE_TYPE, LDC1};
    main_table_[0x37] = {"LD", InstructionType::IMMEDIATE_TYPE, LD};
    main_table_[0x38] = {"SC", InstructionType::IMMEDIATE_TYPE, SC};
    main_table_[0x39] = {"SWC1", InstructionType::IMMEDIATE_TYPE, SWC1};
    main_table_[0x3C] = {"SCD", InstructionType::IMMEDIATE_TYPE, SCD};
    main_table_[0x3D] = {"SDC1", InstructionType::IMMEDIATE_TYPE, SDC1};
    main_table_[0x3F] = {"SD", InstructionType::IMMEDIATE_TYPE, SD};

    // Special table
    special_table_[0x00] = {"SLL", InstructionType::REGISTER_TYPE, SLL};
    special_table_[0x02] = {"SRL", InstructionType::REGISTER_TYPE, SRL};
    special_table_[0x03] = {"SRA", InstructionType::REGISTER_TYPE, SRA};
    special_table_[0x04] = {"SLLV", InstructionType::REGISTER_TYPE, SLLV};
    special_table_[0x06] = {"SRLV", InstructionType::REGISTER_TYPE, SRLV};
    special_table_[0x07] = {"SRAV", InstructionType::REGISTER_TYPE, SRAV};
    special_table_[0x08] = {"JR", InstructionType::REGISTER_TYPE, JR};
    special_table_[0x09] = {"JALR", InstructionType::REGISTER_TYPE, JALR};
    special_table_[0x0C] = {"SYSCALL", InstructionType::SPECIAL_TYPE, SYSCALL};
    special_table_[0x0D] = {"BREAK", InstructionType::SPECIAL_TYPE, BREAK};
    special_table_[0x0F] = {"SYNC", InstructionType::SPECIAL_TYPE, SYNC};
    special_table_[0x10] = {"MFHI", InstructionType::REGISTER_TYPE, MFHI};
    special_table_[0x11] = {"MTHI", InstructionType::REGISTER_TYPE, MTHI};
    special_table_[0x12] = {"MFLO", InstructionType::REGISTER_TYPE, MFLO};
    special_table_[0x13] = {"MTLO", InstructionType::REGISTER_TYPE, MTLO};
    special_table_[0x14] = {"DSLLV", InstructionType::REGISTER_TYPE, DSLLV};
    special_table_[0x16] = {"DSRLV", InstructionType::REGISTER_TYPE, DSRLV};
    special_table_[0x17] = {"DSRAV", InstructionType::REGISTER_TYPE, DSRAV};
    special_table_[0x18] = {"MULT", InstructionType::REGISTER_TYPE, MULT};
    special_table_[0x19] = {"MULTU", InstructionType::REGISTER_TYPE, MULTU};
    special_table_[0x1A] = {"DIV", InstructionType::REGISTER_TYPE, DIV};
    special_table_[0x1B] = {"DIVU", InstructionType::REGISTER_TYPE, DIVU};
    special_table_[0x1C] = {"DMULT", InstructionType::REGISTER_TYPE, DMULT};
    special_table_[0x1D] = {"DMULTU", InstructionType::REGISTER_TYPE, DMULTU};
    special_table_[0x1E] = {"DDIV", InstructionType::REGISTER_TYPE, DDIV};
    special_table_[0x1F] = {"DDIVU", InstructionType::REGISTER_TYPE, DDIVU};
    special_table_[0x20] = {"ADD", InstructionType::REGISTER_TYPE, ADD};
    special_table_[0x21] = {"ADDU", InstructionType::REGISTER_TYPE, ADDU};
    special_table_[0x22] = {"SUB", InstructionType::REGISTER_TYPE, SUB};
    special_table_[0x23] = {"SUBU", InstructionType::REGISTER_TYPE, SUBU};
    special_table_[0x24] = {"AND", InstructionType::REGISTER_TYPE, AND};
    special_table_[0x25] = {"OR", InstructionType::REGISTER_TYPE, OR};
    special_table_[0x26] = {"XOR", InstructionType::REGISTER_TYPE, XOR};
    special_table_[0x27] = {"NOR", InstructionType::REGISTER_TYPE, NOR};
    special_table_[0x2A] = {"SLT", InstructionType::REGISTER_TYPE, SLT};
    special_table_[0x2B] = {"SLTU", InstructionType::REGISTER_TYPE, SLTU};
    special_table_[0x2C] = {"DADD", InstructionType::REGISTER_TYPE, DADD};
    special_table_[0x2D] = {"DADDU", InstructionType::REGISTER_TYPE, DADDU};
    special_table_[0x2E] = {"DSUB", InstructionType::REGISTER_TYPE, DSUB};
    special_table_[0x2F] = {"DSUBU", InstructionType::REGISTER_TYPE, DSUBU};
    special_table_[0x30] = {"TGE", InstructionType::REGISTER_TYPE, TGE};
    special_table_[0x31] = {"TGEU", InstructionType::REGISTER_TYPE, TGEU};
    special_table_[0x32] = {"TLT", InstructionType::REGISTER_TYPE, TLT};
    special_table_[0x33] = {"TLTU", InstructionType::REGISTER_TYPE, TLTU};
    special_table_[0x34] = {"TEQ", InstructionType::REGISTER_TYPE, TEQ};
    special_table_[0x36] = {"TNE", InstructionType::REGISTER_TYPE, TNE};
    special_table_[0x38] = {"DSLL", InstructionType::REGISTER_TYPE, DSLL};
    special_table_[0x3A] = {"DSRL", InstructionType::REGISTER_TYPE, DSRL};
    special_table_[0x3B] = {"DSRA", InstructionType::REGISTER_TYPE, DSRA};
    special_table_[0x3C] = {"DSLL32", InstructionType::REGISTER_TYPE, DSLL32};
    special_table_[0x3E] = {"DSRL32", InstructionType::REGISTER_TYPE, DSRL32};
    special_table_[0x3F] = {"DSRA32", InstructionType::REGISTER_TYPE, DSRA32};

    // REGIMM table
    regimm_table_[0x00] = {"BLTZ", InstructionType::BRANCH_TYPE, BLTZ};
    regimm_table_[0x01] = {"BGEZ", InstructionType::BRANCH_TYPE, BGEZ};
    regimm_table_[0x02] = {"BLTZL", InstructionType::BRANCH_TYPE, BLTZL};
    regimm_table_[0x03] = {"BGEZL", InstructionType::BRANCH_TYPE, BGEZL};
    regimm_table_[0x08] = {"TGEI", InstructionType::IMMEDIATE_TYPE, TGEI};
    regimm_table_[0x09] = {"TGEIU", InstructionType::IMMEDIATE_TYPE, TGEIU};
    regimm_table_[0x0A] = {"TLTI", InstructionType::IMMEDIATE_TYPE, TLTI};
    regimm_table_[0x0B] = {"TLTIU", InstructionType::IMMEDIATE_TYPE, TLTIU};
    regimm_table_[0x0C] = {"TEQI", InstructionType::IMMEDIATE_TYPE, TEQI};
    regimm_table_[0x0E] = {"TNEI", InstructionType::IMMEDIATE_TYPE, TNEI};
    regimm_table_[0x10] = {"BLTZAL", InstructionType::BRANCH_TYPE, BLTZAL};
    regimm_table_[0x11] = {"BGEZAL", InstructionType::BRANCH_TYPE, BGEZAL};
    regimm_table_[0x12] = {"BLTZALL", InstructionType::BRANCH_TYPE, BLTZALL};
    regimm_table_[0x13] = {"BGEZALL", InstructionType::BRANCH_TYPE, BGEZALL};

    // COP0 table
    cop0_table_[0x00] = {"MFC0", InstructionType::REGISTER_TYPE, MFC0};
    cop0_table_[0x01] = {"DMFC0", InstructionType::REGISTER_TYPE, DMFC0};
    cop0_table_[0x04] = {"MTC0", InstructionType::REGISTER_TYPE, MTC0};
    cop0_table_[0x05] = {"DMTC0", InstructionType::REGISTER_TYPE, DMTC0};

    cop0_cofun_table_[0x01] = {"TLBR", InstructionType::SPECIAL_TYPE, TLBR};
    cop0_cofun_table_[0x02] = {"TLBWI", InstructionType::SPECIAL_TYPE, TLBWI};
    cop0_cofun_table_[0x06] = {"TLBWR", InstructionType::SPECIAL_TYPE, TLBWR};
    cop0_cofun_table_[0x08] = {"TLBP", InstructionType::SPECIAL_TYPE, TLBP};
    cop0_cofun_table_[0x18] = {"ERET", InstructionType::SPECIAL_TYPE, ERET};

    cop0_bc_table_[0x00] = {"BC0F", InstructionType::BRANCH_TYPE, BC0F};
    cop0_bc_table_[0x01] = {"BC0T", InstructionType::BRANCH_TYPE, BC0T};
    cop0_bc_table_[0x02] = {"BC0FL", InstructionType::BRANCH_TYPE, BC0FL};
    cop0_bc_table_[0x03] = {"BC0TL", InstructionType::BRANCH_TYPE, BC0TL};

    // COP1 table
    cop1_table_[0x00] = {"MFC1", InstructionType::REGISTER_TYPE, MFC1};
    cop1_table_[0x01] = {"DMFC1", InstructionType::REGISTER_TYPE, DMFC1};
    cop1_table_[0x02] = {"CFC1", InstructionType::REGISTER_TYPE, CFC1};
    cop1_table_[0x04] = {"MTC1", InstructionType::REGISTER_TYPE, MTC1};
    cop1_table_[0x05] = {"DMTC1", InstructionType::REGISTER_TYPE, DMTC1};
    cop1_table_[0x06] = {"CTC1", InstructionType::REGISTER_TYPE, CTC1};

    cop1_bc_table_[0x00] = {"BC1F", InstructionType::BRANCH_TYPE, BC1F};
    cop1_bc_table_[0x01] = {"BC1T", InstructionType::BRANCH_TYPE, BC1T};
    cop1_bc_table_[0x02] = {"BC1FL", InstructionType::BRANCH_TYPE, BC1FL};
    cop1_bc_table_[0x03] = {"BC1TL", InstructionType::BRANCH_TYPE, BC1TL};

    // COP1 FPU operations
    cop1_cofun_table_[0x00] = {"ADD_FMT", InstructionType::REGISTER_TYPE, ADD_FMT};
    cop1_cofun_table_[0x01] = {"SUB_FMT", InstructionType::REGISTER_TYPE, SUB_FMT};
    cop1_cofun_table_[0x02] = {"MUL_FMT", InstructionType::REGISTER_TYPE, MUL_FMT};
    cop1_cofun_table_[0x03] = {"DIV_FMT", InstructionType::REGISTER_TYPE, DIV_FMT};
    cop1_cofun_table_[0x04] = {"SQRT_FMT", InstructionType::REGISTER_TYPE, SQRT_FMT};
    cop1_cofun_table_[0x05] = {"ABS_FMT", InstructionType::REGISTER_TYPE, ABS_FMT};
    cop1_cofun_table_[0x06] = {"MOV_FMT", InstructionType::REGISTER_TYPE, MOV_FMT};
    cop1_cofun_table_[0x07] = {"NEG_FMT", InstructionType::REGISTER_TYPE, NEG_FMT};
    cop1_cofun_table_[0x08] = {"ROUND_L_FMT", InstructionType::REGISTER_TYPE, ROUND_L_FMT};
    cop1_cofun_table_[0x09] = {"TRUNC_L_FMT", InstructionType::REGISTER_TYPE, TRUNC_L_FMT};
    cop1_cofun_table_[0x0A] = {"CEIL_L_FMT", InstructionType::REGISTER_TYPE, CEIL_L_FMT};
    cop1_cofun_table_[0x0B] = {"FLOOR_L_FMT", InstructionType::REGISTER_TYPE, FLOOR_L_FMT};
    cop1_cofun_table_[0x0C] = {"ROUND_W_FMT", InstructionType::REGISTER_TYPE, ROUND_W_FMT};
    cop1_cofun_table_[0x0D] = {"TRUNC_W_FMT", InstructionType::REGISTER_TYPE, TRUNC_W_FMT};
    cop1_cofun_table_[0x0E] = {"CEIL_W_FMT", InstructionType::REGISTER_TYPE, CEIL_W_FMT};
    cop1_cofun_table_[0x0F] = {"FLOOR_W_FMT", InstructionType::REGISTER_TYPE, FLOOR_W_FMT};
    cop1_cofun_table_[0x20] = {"CVT_S_FMT", InstructionType::REGISTER_TYPE, CVT_S_FMT};
    cop1_cofun_table_[0x21] = {"CVT_D_FMT", InstructionType::REGISTER_TYPE, CVT_D_FMT};
    cop1_cofun_table_[0x24] = {"CVT_W_FMT", InstructionType::REGISTER_TYPE, CVT_W_FMT};
    cop1_cofun_table_[0x25] = {"CVT_L_FMT", InstructionType::REGISTER_TYPE, CVT_L_FMT};
    cop1_cofun_table_[0x30] = {"C_F_FMT", InstructionType::REGISTER_TYPE, C_F_FMT};
    cop1_cofun_table_[0x31] = {"C_UN_FMT", InstructionType::REGISTER_TYPE, C_UN_FMT};
    cop1_cofun_table_[0x32] = {"C_EQ_FMT", InstructionType::REGISTER_TYPE, C_EQ_FMT};
    cop1_cofun_table_[0x33] = {"C_UEQ_FMT", InstructionType::REGISTER_TYPE, C_UEQ_FMT};
    cop1_cofun_table_[0x34] = {"C_OLT_FMT", InstructionType::REGISTER_TYPE, C_OLT_FMT};
    cop1_cofun_table_[0x35] = {"C_ULT_FMT", InstructionType::REGISTER_TYPE, C_ULT_FMT};
    cop1_cofun_table_[0x36] = {"C_OLE_FMT", InstructionType::REGISTER_TYPE, C_OLE_FMT};
    cop1_cofun_table_[0x37] = {"C_ULE_FMT", InstructionType::REGISTER_TYPE, C_ULE_FMT};
    cop1_cofun_table_[0x38] = {"C_SF_FMT", InstructionType::REGISTER_TYPE, C_SF_FMT};
    cop1_cofun_table_[0x39] = {"C_NGLE_FMT", InstructionType::REGISTER_TYPE, C_NGLE_FMT};
    cop1_cofun_table_[0x3A] = {"C_SEQ_FMT", InstructionType::REGISTER_TYPE, C_SEQ_FMT};
    cop1_cofun_table_[0x3B] = {"C_NGL_FMT", InstructionType::REGISTER_TYPE, C_NGL_FMT};
    cop1_cofun_table_[0x3C] = {"C_LT_FMT", InstructionType::REGISTER_TYPE, C_LT_FMT};
    cop1_cofun_table_[0x3D] = {"C_NGE_FMT", InstructionType::REGISTER_TYPE, C_NGE_FMT};
    cop1_cofun_table_[0x3E] = {"C_LE_FMT", InstructionType::REGISTER_TYPE, C_LE_FMT};
    cop1_cofun_table_[0x3F] = {"C_NGT_FMT", InstructionType::REGISTER_TYPE, C_NGT_FMT};
}

const InstructionEntry& InstructionTable::lookup(const Instruction& instruction) const
{
    switch (instruction.i_type.opcode) {
        case 0x00:
            return special_table_[instruction.r_type.funct];
        case 0x01:
            return regimm_table_[instruction.i_type.rt];
        case 0x10:
            if (instruction.i_type.rs == 0x08) {
                return cop0_bc_table_[instruction.i_type.rt];
            }
            if (instruction.i_type.rs & 0x10) {
                return cop0_cofun_table_[instruction.r_type.funct];
            }
            return cop0_table_[instruction.i_type.rs];
        case 0x11:
            if (instruction.i_type.rs == 0x08) {
                return cop1_bc_table_[instruction.i_type.rt];
            }
            if (instruction.i_type.rs & 0x10) {
                return cop1_cofun_table_[instruction.r_type.funct];
            }
            return cop1_table_[instruction.i_type.rs];
        default:
            return main_table_[instruction.i_type.opcode];
    }
}

}
