#pragma once

#include <array>
#include "../../utils/types.hpp"
#include "../../interfaces/mi.hpp"
#include "rsp_instruction.hpp"
#include "rsp_instruction_table.hpp"
#include "su.hpp"
#include "vu.hpp"

namespace n64::rdp {
    class RDP;  // Forward declaration
}

namespace n64::rcp {

enum RSP_REGISTERS_ADDRESS : u32 {
    RSP_DMA_SPADDR = 0x04040000,
    RSP_DMA_RAMADDR = 0x04040004,
    RSP_DMA_RDLEN = 0x04040008,
    RSP_DMA_WRLEN = 0x0404000C,
    RSP_STATUS = 0x04040010,
    RSP_DMA_FULL = 0x04040014,
    RSP_DMA_BUSY = 0x04040018,
    RSP_SEMAPHORE = 0x0404001C,
    RSP_PC = 0x04080000
};

class RSP {
public:
    RSP(interfaces::MI& mi, rdp::RDP& rdp);
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

private:

    [[nodiscard]] RSPInstruction fetch_instruction();

    interfaces::MI& mi_;
    rdp::RDP& rdp_;
    RSPInstructionTable instruction_table_;
    SU su_;
    VU vu_;

    std::array<u8, 4096> dmem_;
    std::array<u8, 4096> imem_;

    u32 dma_spaddr_;
    u32 dma_ramaddr_;
    u32 dma_rdlen_;
    u32 dma_wrlen_;
    u32 status_;
    u32 dma_full_;
    u32 dma_busy_;
    u32 semaphore_;
    u32 pc_;

    u32 delay_pc_;
    bool delay_branch_pending_;
};
}