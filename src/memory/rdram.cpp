#include "rdram.hpp"
#include <stdexcept>
#include <string>

namespace n64::memory {

RDRAM::RDRAM()
    : memory_(RDRAM_MEMORY_SIZE, 0)  // Allocate 8MB on heap
{
    device_type_ = 0;
    device_id_ = 0;
    delay_ = 0;
    mode_ = 0;
    ref_interval_ = 0;
    ref_row_ = 0;
    ras_interval_ = 0;
    min_interval_ = 0;
    address_select_ = 0;
    device_manufacturer_ = 0;
    row_ = 0;
}

RDRAM::~RDRAM()
{
}

template <typename T>
T RDRAM::read_memory(u32 address) const
{
    if (address >= RDRAM_MEMORY_START_ADDRESS && address <= RDRAM_MEMORY_END_ADDRESS) {
        u32 offset = address - RDRAM_MEMORY_START_ADDRESS;
        T result = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            result = (result << 8) | memory_[offset + i];  // Big-endian read
        }
        return result;
    }

    if (address >= RDRAM_REGISTER_START_ADDRESS && address <= RDRAM_REGISTER_END_ADDRESS) {
        u32 reg_offset = address - RDRAM_REGISTER_START_ADDRESS;
        return static_cast<T>(read_register(static_cast<RDRAM_REGISTERS_ADDRESS>(reg_offset)));
    }
    
    throw std::runtime_error("Invalid RDRAM address: " + std::to_string(address));
}

template <typename T>
void RDRAM::write_memory(u32 address, T value)
{
    if (address >= RDRAM_MEMORY_START_ADDRESS && address <= RDRAM_MEMORY_END_ADDRESS) {
        u32 offset = address - RDRAM_MEMORY_START_ADDRESS;
        for (size_t i = 0; i < sizeof(T); i++) {
            memory_[offset + i] = static_cast<u8>(value >> ((sizeof(T) - 1 - i) * 8));  // Big-endian write
        }
        return;
    }
    
    if (address >= RDRAM_REGISTER_START_ADDRESS && address <= RDRAM_REGISTER_END_ADDRESS) {
        u32 reg_offset = address - RDRAM_REGISTER_START_ADDRESS;
        write_register(static_cast<RDRAM_REGISTERS_ADDRESS>(reg_offset), static_cast<u32>(value));
        return;
    }
    
    throw std::runtime_error("Invalid RDRAM address: " + std::to_string(address));
}

u32 RDRAM::read_register(RDRAM_REGISTERS_ADDRESS address) const
{
    switch (address) {
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_DEVICE_TYPE:
            return device_type_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_DEVICE_ID:
            return device_id_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_DELAY:
            return delay_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_MODE:
            return mode_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_REF_INTERVAL:
            return ref_interval_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_REF_ROW:
            return ref_row_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_RAS_INTERVAL:
            return ras_interval_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_MIN_INTERVAL:
            return min_interval_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_ADDRESS_SELECT:
            return address_select_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_DEVICE_MANUFACTURER:
            return device_manufacturer_;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_ROW:
            return row_;
        default:
            throw std::runtime_error("Invalid RDRAM register address: " + std::to_string(static_cast<u32>(address)));
    }
}

void RDRAM::write_register(RDRAM_REGISTERS_ADDRESS address, u32 value)
{
    switch (address) {
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_DEVICE_TYPE:
            device_type_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_DEVICE_ID:
            device_id_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_DELAY:
            delay_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_MODE:
            mode_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_REF_INTERVAL:
            ref_interval_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_REF_ROW:
            ref_row_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_RAS_INTERVAL:
            ras_interval_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_MIN_INTERVAL:
            min_interval_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_ADDRESS_SELECT:
            address_select_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_DEVICE_MANUFACTURER:
            device_manufacturer_ = value;
            break;
        case RDRAM_REGISTERS_ADDRESS::RDRAM_REGISTER_ROW:
            row_ = value;
            break;
        default:
            throw std::runtime_error("Invalid RDRAM register address: " + std::to_string(static_cast<u32>(address)));
    }
}

template u8 RDRAM::read_memory<u8>(u32 address) const;
template u16 RDRAM::read_memory<u16>(u32 address) const;
template u32 RDRAM::read_memory<u32>(u32 address) const;
template u64 RDRAM::read_memory<u64>(u32 address) const;
template void RDRAM::write_memory<u8>(u32 address, u8 value);
template void RDRAM::write_memory<u16>(u32 address, u16 value);
template void RDRAM::write_memory<u32>(u32 address, u32 value);
template void RDRAM::write_memory<u64>(u32 address, u64 value);

}