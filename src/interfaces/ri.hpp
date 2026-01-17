#pragma once

#include "../utils/types.hpp"

namespace n64::interfaces {

enum RI_REGISTERS_ADDRESS : u32 {
    RI_MODE = 0x04700000,
    RI_CONFIG = 0x04700004,
    RI_CURRENT_LOAD = 0x04700008,
    RI_SELECT = 0x0470000C,
    RI_REFRESH = 0x04700010,
    RI_LATENCY = 0x04700014,
    RI_ERROR = 0x04700018,
    RI_BANK_STATUS = 0x0470001C,
};

class RI {
public:
    RI();
    ~RI();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

private:
    u32 mode_;
    u32 config_;
    u32 current_load_;
    u32 select_;
    u32 refresh_;
    u32 latency_;
    u32 error_;
    u32 bank_status_;
};
}