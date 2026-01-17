#include "pif.hpp"
#include "memory_constants.hpp"
#include <stdexcept>
#include <string>

namespace n64::memory {

PIF::PIF()
{
}

PIF::~PIF()
{
}

template<typename T>
[[nodiscard]] T PIF::read(u32 address) const
{
    u32 offset = address - PIF_START_ADDRESS;
    if (offset + sizeof(T) > memory_.size()) {
        throw std::runtime_error("Invalid PIF address: " + std::to_string(address));
    }

    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value = (value << 8) | memory_[offset + i];
    }
    return value;
}

template<typename T>
void PIF::write(u32 address, T value)
{
    u32 offset = address - PIF_START_ADDRESS;
    if (offset + sizeof(T) > memory_.size()) {
        throw std::runtime_error("Invalid PIF address: " + std::to_string(address));
    }

    for (size_t i = 0; i < sizeof(T); ++i) {
        memory_[offset + i] = static_cast<u8>(value >> ((sizeof(T) - 1 - i) * 8));
    }
}

// Explicit template instantiations
template u8  PIF::read<u8>(u32 address) const;
template u16 PIF::read<u16>(u32 address) const;
template u32 PIF::read<u32>(u32 address) const;
template u64 PIF::read<u64>(u32 address) const;

template void PIF::write<u8>(u32 address, u8 value);
template void PIF::write<u16>(u32 address, u16 value);
template void PIF::write<u32>(u32 address, u32 value);
template void PIF::write<u64>(u32 address, u64 value);
}