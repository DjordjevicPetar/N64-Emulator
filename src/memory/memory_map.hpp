#pragma once

#include <string>

#include "memory_constants.hpp"
#include "rdram.hpp"
#include "rom.hpp"
#include "../interfaces/ai.hpp"
#include "../interfaces/mi.hpp"
#include "../interfaces/vi.hpp"
#include "../interfaces/si.hpp"
#include "../interfaces/ri.hpp"
#include "../interfaces/pi.hpp"
#include "pif.hpp"

namespace n64::memory {

class MemoryMap {
public:
    MemoryMap(const std::string& rom_path);
    ~MemoryMap();

    template<typename T>
    [[nodiscard]] T read(u32 address);

    template<typename T>
    void write(u32 address, T value);

    u32 boot();

private:
    RDRAM rdram_;
    ROM rom_;
    interfaces::MI mi_;
    interfaces::AI ai_;
    interfaces::VI vi_;
    interfaces::SI si_;
    interfaces::RI ri_;
    interfaces::PI pi_;
    PIF pif_;
};

}
