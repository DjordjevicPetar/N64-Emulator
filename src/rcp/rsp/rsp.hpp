#pragma once

#include <array>
#include "../../utils/types.hpp"
#include "../../interfaces/mi.hpp"
#include "rsp_instruction.hpp"
#include "rsp_instruction_table.hpp"
#include "rsp_registers.hpp"
#include "rsp_dma.hpp"
#include "su.hpp"
#include "vu.hpp"

namespace n64::rdp {
    class RDP;  // Forward declaration
}

namespace n64::memory {
    class RDRAM;  // Forward declaration
}

namespace n64::rcp {

class RSP {
public:
    RSP(interfaces::MI& mi, rdp::RDP& rdp, memory::RDRAM& rdram);
    ~RSP();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

    // COP0 access (registers 0-7 = RSP, 8-15 = RDP)
    [[nodiscard]] u32 read_cop0(u32 reg) const;
    void write_cop0(u32 reg, u32 value);

    void execute_next_instruction();
    void delay_branch(u32 target);

    [[nodiscard]] SU& su() { return su_; }
    [[nodiscard]] VU& vu() { return vu_; }
    [[nodiscard]] u32 pc() const { return pc_; }

    void set_breakpoint();
    void process_passed_cycles(u32 cycles);

    // RSP DMA functions
    [[nodiscard]] RSPStatus& status() { return status_; }
    [[nodiscard]] u8 read_dmem(u32 address) const { return dmem_[address]; }
    [[nodiscard]] u8 read_imem(u32 address) const { return imem_[address]; }
    void write_dmem(u32 address, u8 value) { dmem_[address] = value; }
    void write_imem(u32 address, u8 value) { imem_[address] = value; }

private:

    [[nodiscard]] RSPInstruction fetch_instruction();

    interfaces::MI& mi_;
    rdp::RDP& rdp_;
    RSPInstructionTable instruction_table_;
    SU su_;
    VU vu_;

    std::array<u8, 4096> dmem_;
    std::array<u8, 4096> imem_;

    RSPDmaSPAddr dma_spaddr_;
    RSPDmaRamAddr dma_ramaddr_;
    RSPDmaLen dma_rdlen_;
    RSPDmaLen dma_wrlen_;
    RSPStatus status_;
    u32 semaphore_;
    u32 pc_;

    u32 delay_pc_;
    bool delay_branch_pending_;

    RSPDMA dma_;
    float rsp_cycle_accumulator_ = 0.0f;
};

} // namespace n64::rcp