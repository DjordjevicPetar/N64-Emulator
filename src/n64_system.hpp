#pragma once

#include <string>

// Memory
#include "memory/memory_constants.hpp"
#include "memory/rdram.hpp"
#include "memory/rom.hpp"
#include "memory/pif.hpp"
#include "memory/memory_map.hpp"

// Interfaces
#include "interfaces/mi.hpp"
#include "interfaces/ai.hpp"
#include "interfaces/vi/vi.hpp"
#include "interfaces/si.hpp"
#include "interfaces/ri.hpp"
#include "interfaces/pi/pi.hpp"

// RCP
#include "rcp/rdp/rdp.hpp"
#include "rcp/rsp/rsp.hpp"

// CPU
#include "cpu/vr4300.hpp"

namespace n64 {

class N64System {
public:
    explicit N64System(const std::string& rom_path);
    ~N64System() = default;

    // Main emulation loop
    void run();
    
    // Boot the system - loads ROM code into RDRAM
    u32 boot();

    // Component accessors (for debugging/testing)
    [[nodiscard]] cpu::VR4300& cpu() { return cpu_; }
    [[nodiscard]] rcp::RSP& rsp() { return rsp_; }
    [[nodiscard]] memory::RDRAM& rdram() { return rdram_; }
    [[nodiscard]] memory::MemoryMap& memory() { return memory_map_; }

private:
    // ===== Components (order matters for initialization!) =====
    
    // Memory
    memory::RDRAM rdram_;
    
    // Interfaces (some need MI for interrupts)
    interfaces::MI mi_;  // MI must be before PI, AI, SI, VI
    interfaces::PI pi_;
    interfaces::RI ri_;
    
    // ROM (needs PI)
    memory::ROM rom_;
    
    // RDP
    rdp::RDP rdp_;
    
    // RSP (needs MI, RDP, RDRAM)
    rcp::RSP rsp_;
    
    // Other interfaces (need MI)
    interfaces::AI ai_;
    interfaces::VI vi_;
    interfaces::SI si_;
    
    // PIF
    memory::PIF pif_;
    
    // Memory router (takes references to all components)
    memory::MemoryMap memory_map_;
    
    // CPU (needs memory map)
    cpu::VR4300 cpu_;
};

}
