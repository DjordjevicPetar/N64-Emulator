#include "ai.hpp"
#include <stdexcept>
#include <string>

namespace n64::interfaces {

AI::AI(MI& mi)
    : mi_(mi)
    , dram_addr_(0)
    , length_(0)
    , control_(0)
    , status_(0)
    , dacrate_(0)
    , bitrate_(0)
{
}
AI::~AI() {}

template<typename T>
T AI::read(u32 address) const {
    return read_register(address);
}

template<typename T>
void AI::write(u32 address, T value) {
    write_register(address, value);
}

u32 AI::read_register(u32 address) const {
    switch (address) {
        case AI_REGISTERS_ADDRESS::AI_DRAM_ADDR:
            return length_;
        case AI_REGISTERS_ADDRESS::AI_LENGTH:
            return length_;
        case AI_REGISTERS_ADDRESS::AI_CONTROL:
            return length_;
        case AI_REGISTERS_ADDRESS::AI_STATUS:
            return status_;
        case AI_REGISTERS_ADDRESS::AI_DACRATE:
            return length_;
        case AI_REGISTERS_ADDRESS::AI_BITRATE:
            return length_;
        default:
            throw std::runtime_error("Invalid AI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

void AI::write_register(u32 address, u32 value) {
    switch (address) {
        case AI_REGISTERS_ADDRESS::AI_DRAM_ADDR:
            dram_addr_ = value & 0x00FFFFF8;
            break;
        case AI_REGISTERS_ADDRESS::AI_LENGTH:
            length_ = value & 0x0003FFF8;
            break;
        case AI_REGISTERS_ADDRESS::AI_CONTROL:
            control_ = value & 0x00000001;
            break;
        case AI_REGISTERS_ADDRESS::AI_STATUS:
            // Writing any value clears AI interrupt
            mi_.clear_interrupt(MI_INTERRUPT_AI);
            break;
        case AI_REGISTERS_ADDRESS::AI_DACRATE:
            dacrate_ = value & 0x00000FFF;
            break;
        case AI_REGISTERS_ADDRESS::AI_BITRATE:
            bitrate_ = value & 0x0000000F;
            break;
        default:
            throw std::runtime_error("Invalid AI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

template u8 AI::read<u8>(u32) const;
template u16 AI::read<u16>(u32) const;
template u32 AI::read<u32>(u32) const;
template u64 AI::read<u64>(u32) const;
template void AI::write<u8>(u32, u8);
template void AI::write<u16>(u32, u16);
template void AI::write<u32>(u32, u32);
template void AI::write<u64>(u32, u64);

} // namespace n64::interfaces
