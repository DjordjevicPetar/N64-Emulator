#include "cp0.hpp"

#include <stdexcept>

namespace n64::cpu {

CP0::CP0()
    : regs_{}
{

}

void CP0::set_reg(u8 index, u64 value)
{
    if (index >= 32) return;
    regs_[index] = value;
}

u64 CP0::get_reg(u8 index) const
{
    if (index >= 32) return 0;
    return regs_[index];
}

u32 CP0::translate_address(u64 virtual_address) const
{
    u32 segment = (virtual_address >> 29) & 0x7;
    
    switch (segment) {
        case 0: case 1: case 2: case 3:
            // KUSEG - TLB mapped
            throw std::runtime_error("KUSEG address space not supported (TLB required)");
        case 4:
            // KSEG0 - Direct mapped, cached
            return static_cast<u32>(virtual_address & 0x1FFFFFFF);
        case 5:
            // KSEG1 - Direct mapped, uncached
            return static_cast<u32>(virtual_address & 0x1FFFFFFF);
        case 6:
            // KSSEG - TLB mapped
            throw std::runtime_error("KSSEG address space not supported (TLB required)");
        case 7:
            // KSEG3 - TLB mapped
            throw std::runtime_error("KSEG3 address space not supported (TLB required)");
        default:
            throw std::runtime_error("Invalid address segment");
    }
}

} // namespace n64::cpu
