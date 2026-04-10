#include "n64_system.hpp"
#include <algorithm>
#include <SDL3/SDL.h>

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
    , pif_()
    , si_(mi_, rdram_, pif_)
    , memory_map_(rdram_, rom_, mi_, rdp_, rsp_, ai_, vi_, si_, ri_, pi_, pif_)
    , cpu_(memory_map_)
{
    // Connect PI to ROM and RDRAM for DMA transfers
    pi_.set_dma_targets(rom_, rdram_);

    // EEPROM save path: replace ROM extension with .eep
    std::string save_path = rom_path;
    auto dot = save_path.rfind('.');
    if (dot != std::string::npos)
        save_path = save_path.substr(0, dot);
    pif_.set_save_path(save_path + ".eep");
    
    // Boot the system
    u32 pc_address = boot();
    cpu_.set_pc(pc_address);

    // Simulate what IPL3 boot code sets up before jumping to game code
    // CP0 registers
    cpu_.cp0().set_reg(4,  0x007FFFF0);             // Context
    cpu_.cp0().set_reg(8,  0xFFFFFFFFFFFFFFFF);     // BadVAddr
    cpu_.cp0().set_reg(12, 0x241000E0);             // Status: CU1=1, FR=1, KX=1, SX=1, UX=1
    cpu_.cp0().set_reg(13, 0x00000000);             // Cause
    cpu_.cp0().set_reg(14, 0xFFFFFFFFFFFFFFFF);     // EPC
    cpu_.cp0().set_reg(16, 0x0006E463);             // Config
    cpu_.cp0().set_reg(30, 0xFFFFFFFFFFFFFFFF);     // ErrorEPC

    // GPR registers as left by IPL3
    u8 seed = rom_.cic_seed();
    cpu_.set_gpr(20, 0x00000001);                   // s4 = TV type (1 = NTSC)
    cpu_.set_gpr(22, seed);                         // s6 = CIC seed
    cpu_.set_gpr(29, 0xFFFFFFFFA4001FF0ULL);        // sp = end of RSP DMEM

    // Write CIC seed to PIF RAM so game can verify it
    pif_.write(memory::PIF_START_ADDRESS + 0x24, static_cast<u8>(seed));
    pif_.write(memory::PIF_START_ADDRESS + 0x25, static_cast<u8>(seed));
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

void N64System::poll_input()
{
    const bool* keys = SDL_GetKeyboardState(nullptr);
    memory::ControllerState state;

    if (keys[SDL_SCANCODE_X])      state.buttons |= 0x8000;  // A
    if (keys[SDL_SCANCODE_Z])      state.buttons |= 0x4000;  // B
    if (keys[SDL_SCANCODE_LSHIFT]) state.buttons |= 0x2000;  // Z
    if (keys[SDL_SCANCODE_RETURN]) state.buttons |= 0x1000;  // Start
    if (keys[SDL_SCANCODE_UP])     state.buttons |= 0x0800;  // D-Up
    if (keys[SDL_SCANCODE_DOWN])   state.buttons |= 0x0400;  // D-Down
    if (keys[SDL_SCANCODE_LEFT])   state.buttons |= 0x0200;  // D-Left
    if (keys[SDL_SCANCODE_RIGHT])  state.buttons |= 0x0100;  // D-Right
    if (keys[SDL_SCANCODE_I])      state.buttons |= 0x0008;  // R
    if (keys[SDL_SCANCODE_U])      state.buttons |= 0x0004;  // L
    if (keys[SDL_SCANCODE_K])      state.buttons |= 0x0080;  // C-Right
    if (keys[SDL_SCANCODE_H])      state.buttons |= 0x0040;  // C-Left
    if (keys[SDL_SCANCODE_J])      state.buttons |= 0x0020;  // C-Down
    if (keys[SDL_SCANCODE_Y])      state.buttons |= 0x0010;  // C-Up

    if (keys[SDL_SCANCODE_D]) state.analog_x =  80;
    if (keys[SDL_SCANCODE_A]) state.analog_x = -80;
    if (keys[SDL_SCANCODE_W]) state.analog_y =  80;
    if (keys[SDL_SCANCODE_S]) state.analog_y = -80;

    pif_.set_controller_state(state);
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
        rdp_.process_passed_cycles(cycles);
        ai_.process_passed_cycles(cycles);
        cpu_.cp0().set_mi_interrupt(mi_.check_interrupts());

        event_check_counter += cycles;
        if (event_check_counter >= EVENT_CHECK_INTERVAL) {
            event_check_counter = 0;
            if (!vi_.handle_events()) return;
            poll_input();
        }
    }
}

}
