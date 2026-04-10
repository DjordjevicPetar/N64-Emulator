#pragma once

#include <array>
#include <string>

#include "../utils/types.hpp"

namespace n64::memory {

class RDRAM;

struct ControllerState {
    u16 buttons = 0;
    s8 analog_x = 0;
    s8 analog_y = 0;
};

class PIF {
public:
    PIF();
    ~PIF();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    void dma_read_to_rdram(RDRAM& rdram, u32 dram_addr);
    void dma_write_from_rdram(RDRAM& rdram, u32 dram_addr);

    void process_commands();
    void set_controller_state(const ControllerState& state) { controller_ = state; }
    void set_save_path(const std::string& path);

private:
    bool parse_channel(int& pos, int channel);
    void save_eeprom();
    void load_eeprom();

    std::array<u8, 64> memory_;
    std::array<u8, 2048> eeprom_data_;
    std::string save_path_;
    ControllerState controller_;
};

} // namespace n64::memory