#include "cp0.hpp"

#include <stdexcept>

// TODO: CP0 (System Coprocessor) needs major work:
// TODO: Implement TLB (32 entries) for virtual memory translation
// TODO: Implement TLB instructions (TLBR, TLBWI, TLBWR, TLBP)
// TODO: Implement COUNT/COMPARE timer system (COUNT increments every cycle)
// TODO: Generate timer interrupt when COUNT == COMPARE
// TODO: Implement STATUS register behavior (interrupt enable, exception level, etc.)
// TODO: Implement CAUSE register (exception code, BD bit, interrupt pending)
// TODO: Implement EPC register (exception program counter)
// TODO: Implement exception handling framework (save state, jump to handler)
// TODO: Implement RANDOM register (decrements, wraps at WIRED)
// TODO: Initialize CP0 registers with proper reset values (PRID, CONFIG, etc.)
// TODO: Implement cache simulation for accurate timing

namespace n64::cpu {

CP0::CP0()
    : regs_{}
{
    // TODO: Initialize CP0 registers with proper reset values:
    // PRId = 0x0B22 (VR4300 rev 2.2)
    // Config = appropriate value for N64
    // Status = boot state
}

void CP0::set_reg(u8 index, u64 value)
{
    // TODO: Implement register-specific write behavior:
    // TODO: STATUS - update interrupt enable, exception level, etc.
    // TODO: CAUSE - only IP0, IP1 are writable
    // TODO: COMPARE - writing clears timer interrupt
    // TODO: COUNT - writing sets the counter
    // TODO: WIRED - writing affects RANDOM register bounds
    // TODO: Some bits are read-only and should be masked
    if (index >= 32) return;
    regs_[index] = value;
}

u64 CP0::get_reg(u8 index) const
{
    // TODO: Implement register-specific read behavior:
    // TODO: RANDOM - should return value between WIRED and 31
    // TODO: COUNT - should return current cycle count
    if (index >= 32) return 0;
    return regs_[index];
}

u32 CP0::translate_address(u64 virtual_address) const
{
    u32 segment = (virtual_address >> 29) & 0x7;
    
    switch (segment) {
        case 0: case 1: case 2: case 3:
            // TODO: Implement TLB lookup for KUSEG
            // TODO: Generate TLB miss exception if no matching entry
            // KUSEG - TLB mapped
            throw std::runtime_error("KUSEG address space not supported (TLB required)");
        case 4:
            // KSEG0 - Direct mapped, cached
            // TODO: Implement cache for KSEG0 access
            return static_cast<u32>(virtual_address & 0x1FFFFFFF);
        case 5:
            // KSEG1 - Direct mapped, uncached
            return static_cast<u32>(virtual_address & 0x1FFFFFFF);
        case 6:
            // TODO: Implement TLB lookup for KSSEG
            // KSSEG - TLB mapped
            throw std::runtime_error("KSSEG address space not supported (TLB required)");
        case 7:
            // TODO: Implement TLB lookup for KSEG3
            // KSEG3 - TLB mapped
            throw std::runtime_error("KSEG3 address space not supported (TLB required)");
        default:
            throw std::runtime_error("Invalid address segment");
    }
}

} // namespace n64::cpu
