#include "ri.hpp"
#include <stdexcept>
#include <string>

namespace n64::interfaces {

RI::RI()
    : mode_(0)
    , config_(0)
    , current_load_(0)
    , select_(0)
    , refresh_(0)
    , latency_(0)
    , error_(0)
    , bank_status_(0)
{
}
RI::~RI() {}

template<typename T>
T RI::read(u32 address) const {
    return read_register(address);
}

template<typename T>
void RI::write(u32 address, T value) {
    write_register(address, value);
}

u32 RI::read_register(u32 address) const {
    switch (address) {
        case RI_REGISTERS_ADDRESS::RI_MODE:
            return mode_;
        case RI_REGISTERS_ADDRESS::RI_CONFIG:
            return config_;
        case RI_REGISTERS_ADDRESS::RI_CURRENT_LOAD:
            return current_load_;
        case RI_REGISTERS_ADDRESS::RI_SELECT:
            return select_;
        case RI_REGISTERS_ADDRESS::RI_REFRESH:
            return refresh_;
        case RI_REGISTERS_ADDRESS::RI_LATENCY:
            return latency_;
        case RI_REGISTERS_ADDRESS::RI_ERROR:
            return error_;
        case RI_REGISTERS_ADDRESS::RI_BANK_STATUS:
            return bank_status_;
        default:
            throw std::runtime_error("Invalid RI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

void RI::write_register(u32 address, u32 value) {
    switch (address) {
        case RI_REGISTERS_ADDRESS::RI_MODE:
            mode_ = value & 0x0000000F;
            break;
        case RI_REGISTERS_ADDRESS::RI_CONFIG:
            config_ = value & 0x0000007F;
            break;
        case RI_REGISTERS_ADDRESS::RI_CURRENT_LOAD:
            current_load_ = value;
            break;
        case RI_REGISTERS_ADDRESS::RI_SELECT:
            select_ = value & 0x000000FF;
            break;
        case RI_REGISTERS_ADDRESS::RI_REFRESH:
            refresh_ = value & 0x007FFFFF;
            break;
        case RI_REGISTERS_ADDRESS::RI_LATENCY:
            latency_ = value & 0x0000000F;
            break;
        case RI_REGISTERS_ADDRESS::RI_ERROR:
            error_ = value & 0x00000007;
            break;
        case RI_REGISTERS_ADDRESS::RI_BANK_STATUS:
            bank_status_ = value & 0x0000FFFF;
            break;
        default:
            throw std::runtime_error("Invalid RI register address: " + std::to_string(static_cast<u32>(address)));
    }
}

template u8 RI::read<u8>(u32) const;
template u16 RI::read<u16>(u32) const;
template u32 RI::read<u32>(u32) const;
template u64 RI::read<u64>(u32) const;
template void RI::write<u8>(u32, u8);
template void RI::write<u16>(u32, u16);
template void RI::write<u32>(u32, u32);
template void RI::write<u64>(u32, u64);

} // namespace n64::interfaces
