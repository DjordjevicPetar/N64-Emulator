#include "n64_system.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>

namespace n64 {

N64System::N64System(const std::string& rom_path)
    : rdram_()
    , mi_()
    , pi_(mi_)
    , ri_()
    , rom_(pi_, rom_path)
    , rdp_(rdram_, mi_)
    , rsp_(mi_, rdp_, rdram_)
    , ai_(mi_, rdram_)
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

    // TODO: Replace hardcoded boot stub with actual PIF ROM execution (IPL1/IPL2/IPL3)
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
    using clock = std::chrono::high_resolution_clock;

    u64 event_check_counter = 0;
    constexpr u64 EVENT_CHECK_INTERVAL = 10000;

    u64 total_cycles = 0;
    u64 instr_count = 0;
    double t_cpu = 0, t_rsp = 0, t_vi = 0, t_pi = 0, t_rdp = 0, t_ai = 0, t_mi = 0, t_events = 0;
    auto wall_start = clock::now();
    constexpr u64 PROFILE_INTERVAL = 5000000;
    constexpr u64 SAMPLE_BATCH = 1000;

    // Phase tracking: measure one subsystem per SAMPLE_BATCH instructions
    u8 measure_phase = 0;
    u64 batch_counter = 0;

    while (true) {
        u32 cycles;

        if (batch_counter == 0) {
            // Start timing for current phase
            auto t_start = clock::now();

            for (u64 i = 0; i < SAMPLE_BATCH; i++) {
                cycles = cpu_.execute_next_instruction();
                rsp_.process_passed_cycles(cycles);
                vi_.process_passed_cycles(cycles);
                pi_.process_passed_cycles(cycles);
                rdp_.process_passed_cycles(cycles);
                ai_.process_passed_cycles(cycles);
                cpu_.cp0().set_mi_interrupt(mi_.check_interrupts());
                total_cycles += cycles;
                instr_count++;

                event_check_counter += cycles;
                if (event_check_counter >= EVENT_CHECK_INTERVAL) {
                    event_check_counter = 0;
                    if (!vi_.handle_events()) return;
                }
            }

            auto t_all = std::chrono::duration<double, std::micro>(clock::now() - t_start).count();

            // Now run SAMPLE_BATCH with one subsystem excluded, to measure its cost
            t_start = clock::now();

            for (u64 i = 0; i < SAMPLE_BATCH; i++) {
                cycles = cpu_.execute_next_instruction();
                if (measure_phase != 1) rsp_.process_passed_cycles(cycles);
                if (measure_phase != 2) vi_.process_passed_cycles(cycles);
                if (measure_phase != 3) pi_.process_passed_cycles(cycles);
                if (measure_phase != 4) rdp_.process_passed_cycles(cycles);
                if (measure_phase != 5) ai_.process_passed_cycles(cycles);
                if (measure_phase != 6) cpu_.cp0().set_mi_interrupt(mi_.check_interrupts());
                total_cycles += cycles;
                instr_count++;

                event_check_counter += cycles;
                if (event_check_counter >= EVENT_CHECK_INTERVAL) {
                    event_check_counter = 0;
                    if (!vi_.handle_events()) return;
                }
            }

            double t_without = std::chrono::duration<double, std::micro>(clock::now() - t_start).count();
            double diff = t_all - t_without;
            if (diff < 0) diff = 0;

            switch (measure_phase) {
                case 0: t_cpu += diff; break; // CPU is always present; this measures "all" baseline
                case 1: t_rsp += diff; break;
                case 2: t_vi  += diff; break;
                case 3: t_pi  += diff; break;
                case 4: t_rdp += diff; break;
                case 5: t_ai  += diff; break;
                case 6: t_mi  += diff; break;
            }

            measure_phase = (measure_phase + 1) % 7;
            batch_counter = SAMPLE_BATCH * 10;
        } else {
            // Normal execution without measurement
            cycles = cpu_.execute_next_instruction();
            rsp_.process_passed_cycles(cycles);
            vi_.process_passed_cycles(cycles);
            pi_.process_passed_cycles(cycles);
            rdp_.process_passed_cycles(cycles);
            ai_.process_passed_cycles(cycles);
            cpu_.cp0().set_mi_interrupt(mi_.check_interrupts());
            total_cycles += cycles;
            instr_count++;
            batch_counter--;

            event_check_counter += cycles;
            if (event_check_counter >= EVENT_CHECK_INTERVAL) {
                event_check_counter = 0;
                if (!vi_.handle_events()) return;
            }
        }

        if (instr_count >= PROFILE_INTERVAL) {
            double wall_ms = std::chrono::duration<double, std::milli>(clock::now() - wall_start).count();
            double emu_ms = (double)total_cycles / (CPU_CLOCK / 1000.0);
            double total_us = t_rsp + t_vi + t_pi + t_rdp + t_ai + t_mi;
            double other_us = (wall_ms * 1000.0) - total_us;
            double grand = wall_ms * 1000.0;

            printf("\n=== Profile @ %lluM instr (%.1fs wall, %.1fms emu, %.1f%% speed) ===\n",
                (unsigned long long)(instr_count / 1000000), wall_ms / 1000.0, emu_ms, (emu_ms / wall_ms) * 100.0);
            printf("  CPU+loop: %7.1f ms  (%5.1f%%)\n", other_us / 1000.0, (other_us / grand) * 100.0);
            printf("  RSP:      %7.1f ms  (%5.1f%%)\n", t_rsp / 1000.0,    (t_rsp / grand) * 100.0);
            printf("  VI:       %7.1f ms  (%5.1f%%)\n", t_vi / 1000.0,     (t_vi / grand) * 100.0);
            printf("  PI:       %7.1f ms  (%5.1f%%)\n", t_pi / 1000.0,     (t_pi / grand) * 100.0);
            printf("  RDP:      %7.1f ms  (%5.1f%%)\n", t_rdp / 1000.0,    (t_rdp / grand) * 100.0);
            printf("  AI:       %7.1f ms  (%5.1f%%)\n", t_ai / 1000.0,     (t_ai / grand) * 100.0);
            printf("  MI:       %7.1f ms  (%5.1f%%)\n", t_mi / 1000.0,     (t_mi / grand) * 100.0);
            printf("  Avg cycles/instr: %.2f\n", (double)total_cycles / instr_count);
            fflush(stdout);

            t_cpu = t_rsp = t_vi = t_pi = t_rdp = t_ai = t_mi = t_events = 0;
            wall_start = clock::now();
            total_cycles = 0;
            instr_count = 0;
        }
    }
}

}
