#include "rsp_instruction_table.hpp"

namespace n64::rcp {

RSPInstructionTable::RSPInstructionTable()
{
    for (auto& entry : main_table_) {
        entry = {"UNKNOWN", RSPInstructionType::SCALAR_TYPE, nullptr};
    }
    for (auto& entry : special_table_) {
        entry = {"UNKNOWN", RSPInstructionType::VECTOR_TYPE, nullptr};
    }

    // Main table
    main_table_[0x02] = {"J", RSPInstructionType::SCALAR_TYPE, J};
    main_table_[0x03] = {"JAL", RSPInstructionType::SCALAR_TYPE, JAL};
    main_table_[0x04] = {"BEQ", RSPInstructionType::SCALAR_TYPE, BEQ};
    main_table_[0x05] = {"BNE", RSPInstructionType::SCALAR_TYPE, BNE};
    main_table_[0x06] = {"BLEZ", RSPInstructionType::SCALAR_TYPE, BLEZ};
    main_table_[0x07] = {"BGTZ", RSPInstructionType::SCALAR_TYPE, BGTZ};
    main_table_[0x08] = {"ADDI", RSPInstructionType::SCALAR_TYPE, ADDI};
    main_table_[0x09] = {"ADDIU", RSPInstructionType::SCALAR_TYPE, ADDIU};
    main_table_[0x0A] = {"SLTI", RSPInstructionType::SCALAR_TYPE, SLTI};
    main_table_[0x0B] = {"SLTIU", RSPInstructionType::SCALAR_TYPE, SLTIU};
    main_table_[0x0C] = {"ANDI", RSPInstructionType::SCALAR_TYPE, ANDI};
    main_table_[0x0D] = {"ORI", RSPInstructionType::SCALAR_TYPE, ORI};
    main_table_[0x0E] = {"XORI", RSPInstructionType::SCALAR_TYPE, XORI};
    main_table_[0x0F] = {"LUI", RSPInstructionType::SCALAR_TYPE, LUI};
    main_table_[0x20] = {"LB", RSPInstructionType::SCALAR_TYPE, LB};
    main_table_[0x21] = {"LH", RSPInstructionType::SCALAR_TYPE, LH};
    main_table_[0x23] = {"LW", RSPInstructionType::SCALAR_TYPE, LW};
    main_table_[0x24] = {"LBU", RSPInstructionType::SCALAR_TYPE, LBU};
    main_table_[0x25] = {"LHU", RSPInstructionType::SCALAR_TYPE, LHU};
    main_table_[0x28] = {"SB", RSPInstructionType::SCALAR_TYPE, SB};
    main_table_[0x29] = {"SH", RSPInstructionType::SCALAR_TYPE, SH};
    main_table_[0x2B] = {"SW", RSPInstructionType::SCALAR_TYPE, SW};

    // Special table
    special_table_[0x00] = {"SLL", RSPInstructionType::SCALAR_TYPE, SLL};
    special_table_[0x02] = {"SRL", RSPInstructionType::SCALAR_TYPE, SRL};
    special_table_[0x03] = {"SRA", RSPInstructionType::SCALAR_TYPE, SRA};
    special_table_[0x04] = {"SLLV", RSPInstructionType::SCALAR_TYPE, SLLV};
    special_table_[0x06] = {"SRLV", RSPInstructionType::SCALAR_TYPE, SRLV};
    special_table_[0x07] = {"SRAV", RSPInstructionType::SCALAR_TYPE, SRAV};
    special_table_[0x08] = {"JR", RSPInstructionType::SCALAR_TYPE, JR};
    special_table_[0x09] = {"JALR", RSPInstructionType::SCALAR_TYPE, JALR};
    special_table_[0x0D] = {"BREAK", RSPInstructionType::SCALAR_TYPE, BREAK};
    special_table_[0x20] = {"ADD", RSPInstructionType::SCALAR_TYPE, ADD};
    special_table_[0x21] = {"ADDU", RSPInstructionType::SCALAR_TYPE, ADDU};
    special_table_[0x22] = {"SUB", RSPInstructionType::SCALAR_TYPE, SUB};
    special_table_[0x23] = {"SUBU", RSPInstructionType::SCALAR_TYPE, SUBU};
    special_table_[0x24] = {"AND", RSPInstructionType::SCALAR_TYPE, AND};
    special_table_[0x25] = {"OR", RSPInstructionType::SCALAR_TYPE, OR};
    special_table_[0x26] = {"XOR", RSPInstructionType::SCALAR_TYPE, XOR};
    special_table_[0x27] = {"NOR", RSPInstructionType::SCALAR_TYPE, NOR};
    special_table_[0x2A] = {"SLT", RSPInstructionType::SCALAR_TYPE, SLT};
    special_table_[0x2B] = {"SLTU", RSPInstructionType::SCALAR_TYPE, SLTU};

    // Regimm table
    regimm_table_[0x00] = {"BLTZ", RSPInstructionType::SCALAR_TYPE, BLTZ};
    regimm_table_[0x01] = {"BGEZ", RSPInstructionType::SCALAR_TYPE, BGEZ};
    regimm_table_[0x10] = {"BLTZAL", RSPInstructionType::SCALAR_TYPE, BLTZAL};
    regimm_table_[0x11] = {"BGEZAL", RSPInstructionType::SCALAR_TYPE, BGEZAL};

    // COP0 table
    cop0_table_[0x00] = {"MFC0", RSPInstructionType::SCALAR_TYPE, MFC0};
    cop0_table_[0x04] = {"MTC0", RSPInstructionType::SCALAR_TYPE, MTC0};

    // COP2 COFUN table
    cop2_move_table_[0x00] = {"MFC2", RSPInstructionType::SCALAR_TYPE, MFC2};
    cop2_move_table_[0x02] = {"CFC2", RSPInstructionType::SCALAR_TYPE, CFC2};
    cop2_move_table_[0x04] = {"MTC2", RSPInstructionType::SCALAR_TYPE, MTC2};
    cop2_move_table_[0x06] = {"CTC2", RSPInstructionType::SCALAR_TYPE, CTC2};

    // COP2 table
    cop2_compute_table_[0x00] = {"VMULF", RSPInstructionType::VECTOR_TYPE, VMULF};
    cop2_compute_table_[0x01] = {"VMULU", RSPInstructionType::VECTOR_TYPE, VMULU};
    cop2_compute_table_[0x02] = {"VRNDP", RSPInstructionType::VECTOR_TYPE, VRNDP};
    cop2_compute_table_[0x03] = {"VMULQ", RSPInstructionType::VECTOR_TYPE, VMULQ};
    cop2_compute_table_[0x04] = {"VMUDL", RSPInstructionType::VECTOR_TYPE, VMUDL};
    cop2_compute_table_[0x05] = {"VMUDM", RSPInstructionType::VECTOR_TYPE, VMUDM};
    cop2_compute_table_[0x06] = {"VMUDN", RSPInstructionType::VECTOR_TYPE, VMUDN};
    cop2_compute_table_[0x07] = {"VMUDH", RSPInstructionType::VECTOR_TYPE, VMUDH};
    cop2_compute_table_[0x08] = {"VMACF", RSPInstructionType::VECTOR_TYPE, VMACF};
    cop2_compute_table_[0x09] = {"VMACU", RSPInstructionType::VECTOR_TYPE, VMACU};
    cop2_compute_table_[0x0A] = {"VRNDN", RSPInstructionType::VECTOR_TYPE, VRNDN};
    cop2_compute_table_[0x0B] = {"VMACQ", RSPInstructionType::VECTOR_TYPE, VMACQ};
    cop2_compute_table_[0x0C] = {"VMADL", RSPInstructionType::VECTOR_TYPE, VMADL};
    cop2_compute_table_[0x0D] = {"VMADM", RSPInstructionType::VECTOR_TYPE, VMADM};
    cop2_compute_table_[0x0E] = {"VMADN", RSPInstructionType::VECTOR_TYPE, VMADN};
    cop2_compute_table_[0x0F] = {"VMADH", RSPInstructionType::VECTOR_TYPE, VMADH};
    cop2_compute_table_[0x10] = {"VADD", RSPInstructionType::VECTOR_TYPE, VADD};
    cop2_compute_table_[0x11] = {"VSUB", RSPInstructionType::VECTOR_TYPE, VSUB};
    cop2_compute_table_[0x13] = {"VABS", RSPInstructionType::VECTOR_TYPE, VABS};
    cop2_compute_table_[0x14] = {"VADDC", RSPInstructionType::VECTOR_TYPE, VADDC};
    cop2_compute_table_[0x15] = {"VSUBC", RSPInstructionType::VECTOR_TYPE, VSUBC};
    cop2_compute_table_[0x1D] = {"VSAR", RSPInstructionType::VECTOR_TYPE, VSAR};
    cop2_compute_table_[0x20] = {"VLT", RSPInstructionType::VECTOR_TYPE, VLT};
    cop2_compute_table_[0x21] = {"VEQ", RSPInstructionType::VECTOR_TYPE, VEQ};
    cop2_compute_table_[0x22] = {"VNE", RSPInstructionType::VECTOR_TYPE, VNE};
    cop2_compute_table_[0x23] = {"VGE", RSPInstructionType::VECTOR_TYPE, VGE};
    cop2_compute_table_[0x24] = {"VCL", RSPInstructionType::VECTOR_TYPE, VCL};
    cop2_compute_table_[0x25] = {"VCH", RSPInstructionType::VECTOR_TYPE, VCH};
    cop2_compute_table_[0x26] = {"VCR", RSPInstructionType::VECTOR_TYPE, VCR};
    cop2_compute_table_[0x27] = {"VMRG", RSPInstructionType::VECTOR_TYPE, VMRG};
    cop2_compute_table_[0x28] = {"VAND", RSPInstructionType::VECTOR_TYPE, VAND};
    cop2_compute_table_[0x29] = {"VNAND", RSPInstructionType::VECTOR_TYPE, VNAND};
    cop2_compute_table_[0x2A] = {"VOR", RSPInstructionType::VECTOR_TYPE, VOR};
    cop2_compute_table_[0x2B] = {"VNOR", RSPInstructionType::VECTOR_TYPE, VNOR};
    cop2_compute_table_[0x2C] = {"VXOR", RSPInstructionType::VECTOR_TYPE, VXOR};
    cop2_compute_table_[0x2D] = {"VNXOR", RSPInstructionType::VECTOR_TYPE, VNXOR};
    cop2_compute_table_[0x30] = {"VRCP", RSPInstructionType::VECTOR_TYPE, VRCP};
    cop2_compute_table_[0x31] = {"VRCPL", RSPInstructionType::VECTOR_TYPE, VRCPL};
    cop2_compute_table_[0x32] = {"VRCPH", RSPInstructionType::VECTOR_TYPE, VRCPH};
    cop2_compute_table_[0x33] = {"VMOV", RSPInstructionType::VECTOR_TYPE, VMOV};
    cop2_compute_table_[0x34] = {"VRSQ", RSPInstructionType::VECTOR_TYPE, VRSQ};
    cop2_compute_table_[0x35] = {"VRSQL", RSPInstructionType::VECTOR_TYPE, VRSQL};
    cop2_compute_table_[0x36] = {"VRSQH", RSPInstructionType::VECTOR_TYPE, VRSQH};
    cop2_compute_table_[0x37] = {"VNOP", RSPInstructionType::VECTOR_TYPE, VNOP};

    // LWC2 table
    lwc2_table_[0x00] = {"LBV", RSPInstructionType::VECTOR_TYPE, LBV};
    lwc2_table_[0x01] = {"LSV", RSPInstructionType::VECTOR_TYPE, LSV};
    lwc2_table_[0x02] = {"LLV", RSPInstructionType::VECTOR_TYPE, LLV};
    lwc2_table_[0x03] = {"LDV", RSPInstructionType::VECTOR_TYPE, LDV};
    lwc2_table_[0x04] = {"LQV", RSPInstructionType::VECTOR_TYPE, LQV};
    lwc2_table_[0x05] = {"LRV", RSPInstructionType::VECTOR_TYPE, LRV};
    lwc2_table_[0x06] = {"LPV", RSPInstructionType::VECTOR_TYPE, LPV};
    lwc2_table_[0x07] = {"LUV", RSPInstructionType::VECTOR_TYPE, LUV};
    lwc2_table_[0x08] = {"LHV", RSPInstructionType::VECTOR_TYPE, LHV};
    lwc2_table_[0x09] = {"LFV", RSPInstructionType::VECTOR_TYPE, LFV};
    lwc2_table_[0x0B] = {"LTV", RSPInstructionType::VECTOR_TYPE, LTV};

    // SWC2 table
    swc2_table_[0x00] = {"SBV", RSPInstructionType::VECTOR_TYPE, SBV};
    swc2_table_[0x01] = {"SSV", RSPInstructionType::VECTOR_TYPE, SSV}; 
    swc2_table_[0x02] = {"SLV", RSPInstructionType::VECTOR_TYPE, SLV};
    swc2_table_[0x03] = {"SDV", RSPInstructionType::VECTOR_TYPE, SDV};
    swc2_table_[0x04] = {"SQV", RSPInstructionType::VECTOR_TYPE, SQV};
    swc2_table_[0x05] = {"SRV", RSPInstructionType::VECTOR_TYPE, SRV};
    swc2_table_[0x06] = {"SPV", RSPInstructionType::VECTOR_TYPE, SPV};
    swc2_table_[0x07] = {"SUV", RSPInstructionType::VECTOR_TYPE, SUV};
    swc2_table_[0x08] = {"SHV", RSPInstructionType::VECTOR_TYPE, SHV};
    swc2_table_[0x09] = {"SFV", RSPInstructionType::VECTOR_TYPE, SFV};
    swc2_table_[0x0A] = {"SWV", RSPInstructionType::VECTOR_TYPE, SWV};
    swc2_table_[0x0B] = {"STV", RSPInstructionType::VECTOR_TYPE, STV};
}

const RSPInstructionEntry& RSPInstructionTable::lookup(const RSPInstruction& instruction) const
{
    switch (instruction.i_type.opcode) {
        case 0x00:
            return special_table_[instruction.r_type.funct];
        case 0x01:
            return regimm_table_[instruction.r_type.rt];
        case 0x10:
            return cop0_table_[instruction.r_type.rs];
        case 0x12:
            if (instruction.r_type.rs & 0x10) {
                return cop2_compute_table_[instruction.r_type.funct];
            }
            else {
                return cop2_move_table_[instruction.r_type.rs];
            }
        case 0x32:
            return lwc2_table_[instruction.r_type.rd];
        case 0x3A:
            return swc2_table_[instruction.r_type.rd];
        default:
            return main_table_[instruction.i_type.opcode];
    }
}

}