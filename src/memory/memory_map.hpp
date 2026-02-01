#pragma once

#include <string>

#include "memory_constants.hpp"
#include "rdram.hpp"
#include "rom.hpp"
#include "../interfaces/mi.hpp"
#include "../rcp/rsp/rsp.hpp"
#include "../rcp/rdp/rdp.hpp"
#include "../interfaces/ai.hpp"
#include "../interfaces/vi.hpp"
#include "../interfaces/si.hpp"
#include "../interfaces/ri.hpp"
#include "../interfaces/pi.hpp"
#include "pif.hpp"

namespace n64::memory {

// Forward declarations for component references
class RDRAM;
class ROM;
class PIF;

// MemoryMap is now a lightweight router that routes memory accesses
// to the appropriate component. It does NOT own any components.
class MemoryMap {
public:
    MemoryMap(
        RDRAM& rdram,
        ROM& rom,
        interfaces::MI& mi,
        rdp::RDP& rdp,
        rcp::RSP& rsp,
        interfaces::AI& ai,
        interfaces::VI& vi,
        interfaces::SI& si,
        interfaces::RI& ri,
        interfaces::PI& pi,
        PIF& pif
    );
    
    ~MemoryMap() = default;

    template<typename T>
    [[nodiscard]] T read(u32 address);

    template<typename T>
    void write(u32 address, T value);

private:
    // References to components (not owned)
    RDRAM& rdram_;
    ROM& rom_;
    interfaces::MI& mi_;
    rdp::RDP& rdp_;
    rcp::RSP& rsp_;
    interfaces::AI& ai_;
    interfaces::VI& vi_;
    interfaces::SI& si_;
    interfaces::RI& ri_;
    interfaces::PI& pi_;
    PIF& pif_;
};

}
