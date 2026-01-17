#include "rom.hpp"
#include <fstream>
#include <stdexcept>
#include <algorithm>

namespace n64::memory {

ROM::ROM(interfaces::PI& pi, const std::string& path) : path_(path), pi_(pi) {
    std::ifstream file(path_, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open ROM file: " + path_);
    }
    auto size = file.tellg();
    file.seekg(0);
    memory_.resize(size);
    file.read(reinterpret_cast<char*>(memory_.data()), size);
    convert_to_big_endian();

    file.close();
}

ROM::~ROM() {
}

template<typename T>
T ROM::read(u32 address) const {
    u32 offset = address - ROM_START_ADDRESS;
    if (offset + sizeof(T) > memory_.size()) {
        throw std::runtime_error("Invalid ROM address: " + std::to_string(address));
    }
    
    // Big-endian read
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value = (value << 8) | memory_[offset + i];
    }
    return value;
}

// Explicit template instantiations
template u8  ROM::read<u8>(u32 address) const;
template u16 ROM::read<u16>(u32 address) const;
template u32 ROM::read<u32>(u32 address) const;
template u64 ROM::read<u64>(u32 address) const;

ROM_FORMAT ROM::detect_format() const {
    if (memory_[0] == 0x80 && memory_[1] == 0x37) {
        return ROM_FORMAT::Z64;
    } else if (memory_[0] == 0x40 && memory_[1] == 0x12) {
        return ROM_FORMAT::N64;
    } else if (memory_[0] == 0x37 && memory_[1] == 0x80) {
        return ROM_FORMAT::V64;
    } else {
        throw std::runtime_error("Unsupported ROM format");
    }
}

void ROM::convert_to_big_endian() {
    switch (detect_format()) {
        case ROM_FORMAT::Z64:
            // Already big-endian, nothing to do
            break;
        case ROM_FORMAT::N64:
            // Word-swapped (little-endian) - swap each 4 bytes
            for (size_t i = 0; i < memory_.size(); i += 4) {
                auto* ptr = reinterpret_cast<u32*>(&memory_[i]);
                *ptr = byte_swap(*ptr);
            }
            break;
        case ROM_FORMAT::V64:
            // Byte-swapped - swap each 2 bytes
            for (size_t i = 0; i < memory_.size(); i += 2) {
                auto* ptr = reinterpret_cast<u16*>(&memory_[i]);
                *ptr = byte_swap(*ptr);
            }
            break;
    }
}

u32 ROM::parse_header() {
    // BSD DOM1
    pi_.write_register(interfaces::PI_REGISTERS_ADDRESS::PI_BSD_DOM1_RLS, memory_[0x00000001] >> 4);
    pi_.write_register(interfaces::PI_REGISTERS_ADDRESS::PI_BSD_DOM1_PGS, memory_[0x00000001]);
    pi_.write_register(interfaces::PI_REGISTERS_ADDRESS::PI_BSD_DOM1_PWD, memory_[0x00000002]);
    pi_.write_register(interfaces::PI_REGISTERS_ADDRESS::PI_BSD_DOM1_LAT, memory_[0x00000003]);

    clock_rate_ = read<u32>(0x00000004 + ROM_START_ADDRESS);
    u32 pc_address = read<u32>(0x00000008 + ROM_START_ADDRESS);
    lib_ultra_version_ = read<u32>(0x0000000C + ROM_START_ADDRESS);
    check_code_ = read<u64>(0x00000010 + ROM_START_ADDRESS);
    for (size_t i = 0; i < 20; ++i) {
        game_name_ += static_cast<char>(memory_[0x00000020 + i]);
    }
    category_code_ = static_cast<char>(memory_[0x0000003B]);
    for (size_t i = 0; i < 2; ++i) {
        unique_code_[i] = static_cast<char>(memory_[0x0000003C + i]);
    }
    destination_code_ = static_cast<char>(memory_[0x0000003E]);
    rom_version_ = memory_[0x0000003F];

    return pc_address;
}

} // namespace n64::memory