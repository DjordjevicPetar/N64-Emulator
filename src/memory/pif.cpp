#include "pif.hpp"
#include "rdram.hpp"
#include "memory_constants.hpp"
#include <stdexcept>
#include <string>
#include <fstream>

namespace n64::memory {

PIF::PIF()
{
    memory_.fill(0);
    eeprom_data_.fill(0xFF);
}

PIF::~PIF()
{
    if (!save_path_.empty())
        save_eeprom();
}

void PIF::set_save_path(const std::string& path)
{
    save_path_ = path;
    load_eeprom();
}

void PIF::load_eeprom()
{
    std::ifstream file(save_path_, std::ios::binary);
    if (file)
        file.read(reinterpret_cast<char*>(eeprom_data_.data()), eeprom_data_.size());
}

void PIF::save_eeprom()
{
    std::ofstream file(save_path_, std::ios::binary);
    if (file)
        file.write(reinterpret_cast<const char*>(eeprom_data_.data()), eeprom_data_.size());
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

void PIF::dma_read_to_rdram(RDRAM& rdram, u32 dram_addr)
{
    memory_[63] |= 0x01;
    process_commands();
    for (int i = 0; i < 64; i++) {
        rdram.write_memory<u8>(dram_addr + i, memory_[i]);
    }
}

void PIF::dma_write_from_rdram(RDRAM& rdram, u32 dram_addr)
{
    for (int i = 0; i < 64; i++) {
        memory_[i] = rdram.read_memory<u8>(dram_addr + i);
    }
    process_commands();
}

void PIF::process_commands()
{
    u8 cmd_byte = memory_[63];

    if (cmd_byte & 0x02) {
        // CIC challenge - respond with dummy values to keep the game happy
        // Games check this periodically and reset if it fails
        // A proper implementation would compute the CIC challenge-response,
        // but zeroing the challenge area is enough for most games
        for (int i = 48; i < 63; i++)
            memory_[i] = 0;
        memory_[63] &= ~0x02;
    }

    if (cmd_byte & 0x08) {
        // Terminate PIF boot (clear ROM lockout)
        memory_[63] &= ~0x08;
    }

    if (cmd_byte & 0x10) {
        // Clear PIF RAM
        for (int i = 0; i < 64; i++)
            memory_[i] = 0;
        return;
    }

    if (cmd_byte & 0x20) {
        memory_[63] &= ~0x20;
    }

    if (cmd_byte & 0x40) {
        memory_[63] &= ~0x40;
    }

    if ((cmd_byte & 0x01) == 0) return;

    for (int channel = 0, pos = 0; channel < 5; channel++) {
        if (!parse_channel(pos, channel)) break;
    }

    memory_[63] &= ~0x01;
}

bool PIF::parse_channel(int& pos, int channel)
{
    // Skip past any 0xFF nops
    while (pos < 63 && memory_[pos] == 0xFF) pos++;
    if (pos >= 63) return false;

    u8 tx, rx;
    u8 escape_code = memory_[pos++];

    // Escape codes
    switch (escape_code) {
        case 0x00:
            // Skip channel
            return true;
        case 0xFD:
            // Reset channel
            return true;
        case 0xFE:
            // End of frame
            return false;
    }

    tx = escape_code & 0x3F;
    rx = memory_[pos++] & 0x3F;

    if (channel > 0 && channel < 4) {
        memory_[pos - 1] = rx | 0x80;
        pos += tx + rx;
        return true;
    }

    u8 response_pos = pos + tx;
    u8 cmd_byte = memory_[pos++];

    switch (cmd_byte) {
        case 0x00: case 0xFF:
            if (channel == 0) {
                memory_[response_pos++] = 0x05;
                memory_[response_pos++] = 0x00;
                memory_[response_pos++] = 0x02;
            } else {
                memory_[response_pos++] = 0x00;
                memory_[response_pos++] = 0xC0;
                memory_[response_pos++] = 0x00;
            }
            break;
        case 0x01:
            memory_[response_pos++] = (controller_.buttons >> 8) & 0xFF;
            memory_[response_pos++] = controller_.buttons & 0xFF;
            memory_[response_pos++] = static_cast<u8>(controller_.analog_x);
            memory_[response_pos++] = static_cast<u8>(controller_.analog_y);
            break;
        case 0x04: {
            u8 block = memory_[pos];
            for (int i = 0; i < 8; i++)
                memory_[response_pos++] = eeprom_data_[(block << 3) + i];
            break;
        }
        case 0x05: {
            u8 block = memory_[pos];
            for (int i = 0; i < 8; i++)
                eeprom_data_[(block << 3) + i] = memory_[pos + 1 + i];
            memory_[response_pos++] = 0x00;
            save_eeprom();
            break;
        }
        default:
            pos += tx + rx;
            return true;
    }

    pos = response_pos;
    return true;
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