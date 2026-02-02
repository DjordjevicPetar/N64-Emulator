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
    // TODO: Implement AI_STATUS register read behavior - should return busy flags, FIFO status
    switch (address) {
        case AI_REGISTERS_ADDRESS::AI_DRAM_ADDR:
            return dram_addr_;  // BUG FIX: was returning length_
        case AI_REGISTERS_ADDRESS::AI_LENGTH:
            return length_;
        case AI_REGISTERS_ADDRESS::AI_CONTROL:
            return control_;  // BUG FIX: was returning length_
        case AI_REGISTERS_ADDRESS::AI_STATUS:
            return status_;
        case AI_REGISTERS_ADDRESS::AI_DACRATE:
            return dacrate_;  // BUG FIX: was returning length_
        case AI_REGISTERS_ADDRESS::AI_BITRATE:
            return bitrate_;  // BUG FIX: was returning length_
        default:
            throw std::runtime_error("Invalid AI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

void AI::write_register(u32 address, u32 value) {
    // TODO: Implement audio DMA transfer system
    // TODO: Implement audio FIFO buffer management (double-buffered)
    // TODO: Implement audio playback timing based on DAC rate
    // TODO: Generate AI interrupt when DMA buffer completes
    switch (address) {
        case AI_REGISTERS_ADDRESS::AI_DRAM_ADDR:
            dram_addr_ = value & 0x00FFFFF8;
            break;
        case AI_REGISTERS_ADDRESS::AI_LENGTH:
            length_ = value & 0x0003FFF8;
            // TODO: Trigger DMA transfer when length is written (if control bit is set)
            break;
        case AI_REGISTERS_ADDRESS::AI_CONTROL:
            control_ = value & 0x00000001;
            // TODO: Enable/disable audio DMA based on control bit
            break;
        case AI_REGISTERS_ADDRESS::AI_STATUS:
            // Writing any value clears AI interrupt
            mi_.clear_interrupt(MI_INTERRUPT_AI);
            break;
        case AI_REGISTERS_ADDRESS::AI_DACRATE:
            dacrate_ = value & 0x00000FFF;
            // TODO: Implement DAC rate calculation and audio sample timing
            break;
        case AI_REGISTERS_ADDRESS::AI_BITRATE:
            bitrate_ = value & 0x0000000F;
            // TODO: Implement bitrate effects on audio quality
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
