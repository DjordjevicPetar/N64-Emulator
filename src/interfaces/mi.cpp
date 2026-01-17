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
            bool set_upper = get_bit(value, 13);
            bool clear_upper = get_bit(value, 12);
            bool clear_dp = get_bit(value, 11);
            bool set_ebus = get_bit(value, 10);
            bool clear_ebus = get_bit(value, 9);
            bool set_repeat = get_bit(value, 8);
            bool clear_repeat = get_bit(value, 7);
            u8 repeat_count = get_bits(value, 6, 0);
            if (set_upper && !clear_upper) {
                mode_ = set_bit(mode_, 9, true);
            } else if (clear_upper && !set_upper) {
                mode_ = set_bit(mode_, 9, false);
            }
            if (clear_dp) {
                interrupt_ = set_bit(interrupt_, MI_INTERRUPT_DP, false);
            }
            if (set_ebus && !clear_ebus) {
                mode_ = set_bit(mode_, 8, true);
            } else if (clear_ebus && !set_ebus) {
                mode_ = set_bit(mode_, 8, false);
            }
            if (set_repeat && !clear_repeat) {
                mode_ = set_bit(mode_, 7, true);
            } else if (clear_repeat && !set_repeat) {
                mode_ = set_bit(mode_, 7, false);
            }
            mode_ = set_bits(mode_, 6, 0, repeat_count);
            break;
        }
        case MI_REGISTERS_ADDRESS::MI_MASK: {
            bool set_dp = get_bit(value, 11);
            bool clear_dp = get_bit(value, 10);
            bool set_pi = get_bit(value, 9);
            bool clear_pi = get_bit(value, 8);
            bool set_vi = get_bit(value, 7);
            bool clear_vi = get_bit(value, 6);
            bool set_ai = get_bit(value, 5);
            bool clear_ai = get_bit(value, 4);
            bool set_si = get_bit(value, 3);
            bool clear_si = get_bit(value, 2);
            bool set_sp = get_bit(value, 1);
            bool clear_sp = get_bit(value, 0);
            if (set_dp && !clear_dp) {
                mask_ = set_bit(mask_, MI_INTERRUPT_DP, true);
            } else if (clear_dp && !set_dp) {
                mask_ = set_bit(mask_, MI_INTERRUPT_DP, false);
            }
            if (set_pi && !clear_pi) {
                mask_ = set_bit(mask_, MI_INTERRUPT_PI, true);
            } else if (clear_pi && !set_pi) {
                mask_ = set_bit(mask_, MI_INTERRUPT_PI, false);
            }
            if (set_vi && !clear_vi) {
                mask_ = set_bit(mask_, MI_INTERRUPT_VI, true);
            } else if (clear_vi && !set_vi) {
                mask_ = set_bit(mask_, MI_INTERRUPT_VI, false);
            }
            if (set_ai && !clear_ai) {
                mask_ = set_bit(mask_, MI_INTERRUPT_AI, true);
            } else if (clear_ai && !set_ai) {
                mask_ = set_bit(mask_, MI_INTERRUPT_AI, false);
            }
            if (set_si && !clear_si) {
                mask_ = set_bit(mask_, MI_INTERRUPT_SI, true);
            } else if (clear_si && !set_si) {
                mask_ = set_bit(mask_, MI_INTERRUPT_SI, false);
            }
            if (set_sp && !clear_sp) {
                mask_ = set_bit(mask_, MI_INTERRUPT_SP, true);
            } else if (clear_sp && !set_sp) {
                mask_ = set_bit(mask_, MI_INTERRUPT_SP, false);
            }
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

template u8 MI::read<u8>(u32) const;
template u16 MI::read<u16>(u32) const;
template u32 MI::read<u32>(u32) const;
template u64 MI::read<u64>(u32) const;
template void MI::write<u8>(u32, u8);
template void MI::write<u16>(u32, u16);
template void MI::write<u32>(u32, u32);
template void MI::write<u64>(u32, u64);

} // namespace n64::interfaces
