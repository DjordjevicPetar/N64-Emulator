#include "n64_system.hpp"
#include <algorithm>
#include <cstdio>
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
    pi_.set_dma_targets(rom_, rdram_);

    std::string save_path = rom_path;
    auto dot = save_path.rfind('.');
    if (dot != std::string::npos)
        save_path = save_path.substr(0, dot);
    pif_.set_save_path(save_path + ".eep");
    
    fprintf(stderr, "[BOOT] ROM loaded: %zu bytes\n", rom_.size());
    fprintf(stderr, "[BOOT] CIC seed: 0x%02X\n", rom_.cic_seed());

    u32 pc_address = boot();
    cpu_.set_pc(pc_address);
    fprintf(stderr, "[BOOT] Entry point: 0x%08X\n", pc_address);

    // CP0 registers as set by IPL3
    cpu_.cp0().set_reg(1,  31);           // Random
    cpu_.cp0().set_reg(4,  0x007FFFF0);   // Context
    cpu_.cp0().set_reg(8,  0xFFFFFFFFFFFFFFFF); // BadVAddr
    cpu_.cp0().set_reg(12, 0x34000000);   // Status: FR=1, CU1=1
    cpu_.cp0().set_reg(13, 0x00000000);   // Cause
    cpu_.cp0().set_reg(14, 0xFFFFFFFFFFFFFFFF); // EPC
    cpu_.cp0().set_reg(15, 0x00000B00);   // PRId: VR4300
    cpu_.cp0().set_reg(16, 0x0006E463);   // Config
    cpu_.cp0().set_reg(30, 0xFFFFFFFFFFFFFFFF); // ErrorEPC

    // GPR registers as set by IPL3
    u8 seed = rom_.cic_seed();
    cpu_.set_gpr(11, 0xFFFFFFFFA4000040ULL); // t3
    cpu_.set_gpr(20, 0x00000001);            // s4 = TV type (NTSC)
    cpu_.set_gpr(22, seed);                  // s6 = CIC seed
    cpu_.set_gpr(29, 0xFFFFFFFFA4001FF0ULL); // sp

    // CIC seed in PIF RAM
    pif_.write(memory::PIF_START_ADDRESS + 0x24, static_cast<u8>(seed));
    pif_.write(memory::PIF_START_ADDRESS + 0x25, static_cast<u8>(seed));

    // libultra OS variables in RDRAM (written by IPL3)
    rdram_.write_memory<u32>(0x00000300, 0x00000001); // osTvType = NTSC
    rdram_.write_memory<u32>(0x00000304, 0x00000000); // osRomType = cartridge
    rdram_.write_memory<u32>(0x00000308, 0xB0000000); // osRomBase
    rdram_.write_memory<u32>(0x0000030C, 0x00000000); // osResetType = cold boot
    rdram_.write_memory<u32>(0x00000310, seed);        // osCicId
    rdram_.write_memory<u32>(0x00000314, 0x00000000); // osVersion
    rdram_.write_memory<u32>(0x00000318, memory::RDRAM_MEMORY_SIZE); // osMemSize = 8MB

    // RI registers (set by IPL3 during RDRAM initialization)
    ri_.write_register(0x04700000, 0x0E); // RI_MODE
    ri_.write_register(0x04700004, 0x40); // RI_CONFIG
    ri_.write_register(0x04700008, 0x00); // RI_CURRENT_LOAD
    ri_.write_register(0x0470000C, 0x14); // RI_SELECT
    ri_.write_register(0x04700010, 0x00063634); // RI_REFRESH

    fprintf(stderr, "[BOOT] Boot complete, starting execution\n");
}

u32 N64System::boot()
{
    u32 entry_point = rom_.parse_header();
    
    constexpr u32 ROM_CODE_OFFSET = 0x1000;
    constexpr size_t IPL3_COPY_SIZE = 0x100000; // IPL3 copies exactly 1MB
    u32 rdram_dest = entry_point & 0x1FFFFFFF;
    
    size_t copy_size = std::min({
        IPL3_COPY_SIZE,
        rom_.size() - ROM_CODE_OFFSET, 
        static_cast<size_t>(memory::RDRAM_MEMORY_SIZE - rdram_dest)
    });
    
    fprintf(stderr, "[BOOT] Copying %zu bytes from ROM:0x%X to RDRAM:0x%X\n", 
            copy_size, ROM_CODE_OFFSET, rdram_dest);

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
    u64 total_instructions = 0;
    constexpr u64 EVENT_CHECK_INTERVAL = 10000;

    while (true) {
        u32 cycles = cpu_.execute_next_instruction();
        total_instructions++;

        if (total_instructions <= 100) {
            fprintf(stderr, "[CPU] #%llu PC=0x%08llX instr=0x%08X\n",
                    (unsigned long long)total_instructions,
                    (unsigned long long)cpu_.pc(),
                    rdram_.read_memory<u32>((cpu_.pc() - 4) & 0x1FFFFFFF));
        } else if (total_instructions == 1000) {
            fprintf(stderr, "[CPU] ... (1K instructions reached, PC=0x%08llX)\n", (unsigned long long)cpu_.pc());
        } else if (total_instructions % 10000 == 0 && total_instructions <= 500000) {
            fprintf(stderr, "[CPU] %lluK PC=0x%08llX\n",
                    (unsigned long long)(total_instructions / 1000),
                    (unsigned long long)cpu_.pc());
        } else if (total_instructions % 1000000 == 0) {
            fprintf(stderr, "[CPU] %lluM instr, PC=0x%08llX, VI: origin=0x%X width=%u type=%u, MI: int=0x%X mask=0x%X, RSP: halt=%u\n",
                    (unsigned long long)(total_instructions / 1000000),
                    (unsigned long long)cpu_.pc(),
                    vi_.origin().origin,
                    vi_.width().width,
                    vi_.ctrl().type,
                    mi_.interrupt_reg(),
                    mi_.mask_reg(),
                    (unsigned)rsp_.status().halt);
        }

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
