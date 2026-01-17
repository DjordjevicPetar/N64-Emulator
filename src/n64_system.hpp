#pragma once

#include <string>

#include "memory/memory_map.hpp"
#include "cpu/vr4300.hpp"

namespace n64 {

class N64System {
    public:
    N64System(const std::string& rom_path) : memory_map_(rom_path), cpu_(memory_map_) {
        u32 pc_address = memory_map_.boot();
        cpu_.set_pc(pc_address);
    }

    void run();

private:
    n64::memory::MemoryMap memory_map_;
    n64::cpu::VR4300 cpu_;
};
}