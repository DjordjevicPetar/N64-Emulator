#include "n64_system.hpp"
#include <algorithm>

namespace n64 {

N64System::N64System(const std::string& rom_path)
    : rdram_()
    , mi_()
    , pi_(mi_)
    , ri_()
    , rom_(pi_, rom_path)
    , rdp_()
    , rsp_(mi_, rdp_, rdram_)
    , ai_(mi_)
    , vi_(mi_, rdram_)
    , si_(mi_)
    , pif_()
    , memory_map_(rdram_, rom_, mi_, rdp_, rsp_, ai_, vi_, si_, ri_, pi_, pif_)
    , cpu_(memory_map_)
{
    // Connect PI to ROM and RDRAM for DMA transfers
    pi_.set_dma_targets(rom_, rdram_);
    
    // Boot the system
    u32 pc_address = boot();
    cpu_.set_pc(pc_address);

    // Simulate what PIF boot code sets up after IPL3 runs:
    // CP0 registers as they appear after PIF boot completes
    cpu_.cp0().set_reg(4,  0x007FFFF0);             // Context
    cpu_.cp0().set_reg(8,  0xFFFFFFFFFFFFFFFF);     // BadVAddr = 0xFFFFFFFF
    cpu_.cp0().set_reg(12, 0x241000E0);             // Status: CU1=1, FR=1, SR=1, KX=1, SX=1, UX=1
    cpu_.cp0().set_reg(13, 0x00000000);             // Cause = 0
    cpu_.cp0().set_reg(14, 0xFFFFFFFFFFFFFFFF);     // EPC = 0xFFFFFFFF
    cpu_.cp0().set_reg(16, 0x0006E463);             // Config
    cpu_.cp0().set_reg(30, 0xFFFFFFFFFFFFFFFF);     // ErrorEPC = 0xFFFFFFFF
}

u32 N64System::boot()
{
    u32 entry_point = rom_.parse_header();
    
    // Simulate what IPL3 bootcode does:
    // Copy game code from ROM to RDRAM
    // ROM offset 0x1000 (after header + bootcode) -> RDRAM at entry point
    
    constexpr u32 ROM_CODE_OFFSET = 0x1000;  // After header (0x40) and bootcode (0xFC0)
    u32 rdram_dest = entry_point & 0x1FFFFFFF;  // Convert KSEG0/1 to physical
    
    // Calculate how much to copy (ROM size minus header, limited to RDRAM size)
    size_t copy_size = std::min(
        rom_.size() - ROM_CODE_OFFSET, 
        static_cast<size_t>(memory::RDRAM_MEMORY_SIZE - rdram_dest)
    );
    
    for (size_t i = 0; i < copy_size; ++i) {
        rdram_.write_memory<u8>(
            rdram_dest + i, 
            rom_.read<u8>(memory::ROM_START_ADDRESS + ROM_CODE_OFFSET + i)
        );
    }
    
    return entry_point;
}

void N64System::run()
{
    u64 event_check_counter = 0;
    constexpr u64 EVENT_CHECK_INTERVAL = 10000;

    while (true) {
        u32 cycles = cpu_.execute_next_instruction();
        rsp_.process_passed_cycles(cycles);
        vi_.process_passed_cycles(cycles);
        pi_.process_passed_cycles(cycles);
        cpu_.cp0().set_mi_interrupt(mi_.check_interrupts());

        event_check_counter += cycles;
        if (event_check_counter >= EVENT_CHECK_INTERVAL) {
            event_check_counter = 0;
            if (!vi_.handle_events()) break;
        }
    }
}

}
