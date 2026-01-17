#pragma once

#include "../utils/types.hpp"
#include "mi.hpp"

namespace n64::interfaces {

enum AI_REGISTERS_ADDRESS : u32 {
    AI_DRAM_ADDR = 0x04500000,
    AI_LENGTH = 0x04500004,
    AI_CONTROL = 0x04500008,
    AI_STATUS = 0x0450000C,
    AI_DACRATE = 0x04500010,
    AI_BITRATE = 0x04500014,
};

class AI {
public:
    AI(MI& mi);
    ~AI();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

private:
    MI& mi_;
    
    u32 dram_addr_;
    u32 length_;
    u32 control_;
    u32 status_;
    u32 dacrate_;
    u32 bitrate_;
};
}