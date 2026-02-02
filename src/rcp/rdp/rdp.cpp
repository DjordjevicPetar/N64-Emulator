#include "rdp.hpp"

#include <stdexcept>
#include <string>

namespace n64::rdp {

// TODO: The RDP is almost entirely unimplemented. Major work needed:
// TODO: Implement RDP command FIFO processing
// TODO: Implement RDP rendering pipeline (triangle setup, rasterization)
// TODO: Implement RDP texture mapping and TMEM management
// TODO: Implement RDP color combiner (configurable per-pixel color math)
// TODO: Implement RDP blender (alpha blending, fog, coverage)
// TODO: Implement RDP Z-buffer handling
// TODO: Implement RDP span buffer rendering
// TODO: Implement cycle-accurate RDP timing
// TODO: Generate DP interrupt when command buffer completes
// TODO: Parse and execute all RDP commands (Set_*, Triangle, Rectangle, etc.)

RDP::RDP()
    : start_(0)
    , end_(0)
    , current_(0)
    , status_(0)
    , clock_(0)
    , buf_busy_(0)
    , pipe_busy_(0)
    , tmem_busy_(0)
{
}

RDP::~RDP() {}

template<typename T>
T RDP::read(u32 address) const {
    return read_register(address);
}

template<typename T>
void RDP::write(u32 address, T value) {
    write_register(address, value);
}

u32 RDP::read_register(u32 address) const {
    switch (address) {
        case RDP_REGISTERS_ADDRESS::DPC_START:
            return start_;
        case RDP_REGISTERS_ADDRESS::DPC_END:
            return end_;
        case RDP_REGISTERS_ADDRESS::DPC_CURRENT:
            return current_;
        case RDP_REGISTERS_ADDRESS::DPC_STATUS:
            return status_;
        case RDP_REGISTERS_ADDRESS::DPC_CLOCK:
            return clock_;
        case RDP_REGISTERS_ADDRESS::DPC_BUF_BUSY:
            return buf_busy_;
        case RDP_REGISTERS_ADDRESS::DPC_PIPE_BUSY:
            return pipe_busy_;
        case RDP_REGISTERS_ADDRESS::DPC_TMEM_BUSY:
            return tmem_busy_;
        default:
            throw std::runtime_error("Invalid RDP register address: " + std::to_string(address));
    }
}

void RDP::write_register(u32 address, u32 value) {
    switch (address) {
        case RDP_REGISTERS_ADDRESS::DPC_START:
            start_ = value & 0x00FFFFFF;
            // TODO: Writing DPC_START should set the command buffer start address
            break;
        case RDP_REGISTERS_ADDRESS::DPC_END:
            end_ = value & 0x00FFFFFF;
            // TODO: Writing DPC_END should trigger RDP command processing
            // TODO: Parse commands from start_ to end_ and execute them
            break;
        case RDP_REGISTERS_ADDRESS::DPC_STATUS: {
            bool clr_clock = get_bit(value, 9);
            bool clr_buffer_busy = get_bit(value, 8);
            bool clr_pipe_busy = get_bit(value, 7);
            bool clr_tmem_busy = get_bit(value, 6);
            bool set_flush = get_bit(value, 5);
            bool clr_flush = get_bit(value, 4);
            bool set_freeze = get_bit(value, 3);
            bool clr_freeze = get_bit(value, 2);
            bool set_xbus = get_bit(value, 1);
            bool clr_xbus = get_bit(value, 0);

            if (clr_clock) {
                clock_ = 0;
            }
            if (clr_buffer_busy) {
                buf_busy_ = 0;
            }
            if (clr_pipe_busy) {
                pipe_busy_ = 0;
            }
            if (clr_tmem_busy) {
                tmem_busy_ = 0;
            }
            if (set_flush & !clr_flush) {
                status_ = set_bit(status_, 2, true);
            }
            if (clr_flush & !set_flush) {
                status_ = set_bit(status_, 2, false);
            }
            if (set_freeze & !clr_freeze) {
                status_ = set_bit(status_, 1, true);
            }
            if (clr_freeze & !set_freeze) {
                status_ = set_bit(status_, 1, false);
            }
            if (set_xbus & !clr_xbus) {
                status_ = set_bit(status_, 0, true);
            }
            if (clr_xbus & !set_xbus) {
                status_ = set_bit(status_, 0, false);
            }
            break;
        }
        default:
            throw std::runtime_error("Invalid RDP register address: " + std::to_string(address));
    }
}

template u8 RDP::read<u8>(u32) const;
template u16 RDP::read<u16>(u32) const;
template u32 RDP::read<u32>(u32) const;
template u64 RDP::read<u64>(u32) const;
template void RDP::write<u8>(u32, u8);
template void RDP::write<u16>(u32, u16);
template void RDP::write<u32>(u32, u32);
template void RDP::write<u64>(u32, u64);

} // namespace n64::rcp