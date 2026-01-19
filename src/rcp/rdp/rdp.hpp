#pragma once

#include "../../utils/types.hpp"

namespace n64::rdp {

enum RDP_REGISTERS_ADDRESS : u32 {
    DPC_START = 0x04100000,
    DPC_END = 0x04100004,
    DPC_CURRENT = 0x04100008,
    DPC_STATUS = 0x0410000C,
    DPC_CLOCK = 0x04100010,
    DPC_BUF_BUSY = 0x04100014,
    DPC_PIPE_BUSY = 0x04100018,
    DPC_TMEM_BUSY = 0x0410001C
};

class RDP {
public:
    RDP();
    ~RDP();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

    void set_reg(u32 index, u32 value);

private:
    u32 start_;
    u32 end_;
    u32 current_;
    u32 status_;
    u32 clock_;
    u32 buf_busy_;
    u32 pipe_busy_;
    u32 tmem_busy_;
};
}