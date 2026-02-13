#include "mi.hpp"
#include <stdexcept>
#include <string>

namespace n64::interfaces {

MI::MI(): mode_(), version_(0x02020102), interrupt_(), mask_() {}
MI::~MI() {}

template<typename T>
T MI::read(u32 address) const {
    return read_register(address);
}

template<typename T>
void MI::write(u32 address, T value) {
    write_register(address, value);
}

u32 MI::read_register(u32 address) const {
    switch (address) {
        case MI_REGISTERS_ADDRESS::MI_MODE:
            return mode_;
        case MI_REGISTERS_ADDRESS::MI_VERSION:
            return version_;
        case MI_REGISTERS_ADDRESS::MI_INTERRUPT:
            return interrupt_;
        case MI_REGISTERS_ADDRESS::MI_MASK:
            return mask_;
        default:
            throw std::runtime_error("Invalid MI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

void MI::write_register(u32 address, u32 value) {
    switch (address) {
        case MI_REGISTERS_ADDRESS::MI_MODE: {
            clear_set_resolver(mode_, get_bit(value, 12), get_bit(value, 13), 9);  // upper
            clear_set_resolver(mode_, get_bit(value, 9),  get_bit(value, 10), 8);  // ebus
            clear_set_resolver(mode_, get_bit(value, 7),  get_bit(value, 8),  7);  // repeat
            mode_ = static_cast<u32>(set_bits(mode_, 6, 0, get_bits(value, 6, 0)));  // repeat count
            if (get_bit(value, 11)) {  // clear DP interrupt
                interrupt_ = static_cast<u32>(set_bit(interrupt_, MI_INTERRUPT_DP, false));
            }
            break;
        }
        case MI_REGISTERS_ADDRESS::MI_MASK: {
            clear_set_resolver(mask_, get_bit(value, 0),  get_bit(value, 1),  MI_INTERRUPT_SP);
            clear_set_resolver(mask_, get_bit(value, 2),  get_bit(value, 3),  MI_INTERRUPT_SI);
            clear_set_resolver(mask_, get_bit(value, 4),  get_bit(value, 5),  MI_INTERRUPT_AI);
            clear_set_resolver(mask_, get_bit(value, 6),  get_bit(value, 7),  MI_INTERRUPT_VI);
            clear_set_resolver(mask_, get_bit(value, 8),  get_bit(value, 9),  MI_INTERRUPT_PI);
            clear_set_resolver(mask_, get_bit(value, 10), get_bit(value, 11), MI_INTERRUPT_DP);
            break;
        }
        default:
            throw std::runtime_error("Invalid MI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

void MI::set_interrupt(MI_INTERRUPT_BITS interrupt) {
    interrupt_ = set_bit(interrupt_, static_cast<u32>(interrupt), true);
}

void MI::clear_interrupt(MI_INTERRUPT_BITS interrupt) {
    interrupt_ = set_bit(interrupt_, static_cast<u32>(interrupt), false);
}

bool MI::check_interrupts() const {
    return (interrupt_ & mask_) != 0;
}

template u8 MI::read<u8>(u32) const;
template u16 MI::read<u16>(u32) const;
template u32 MI::read<u32>(u32) const;
template u64 MI::read<u64>(u32) const;
template void MI::write<u8>(u32, u8);
template void MI::write<u16>(u32, u16);
template void MI::write<u32>(u32, u32);
template void MI::write<u64>(u32, u64);

} // namespace n64::interfaces
