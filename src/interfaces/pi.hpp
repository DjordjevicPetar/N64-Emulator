#pragma once

#include "../utils/types.hpp"

namespace n64::interfaces {

enum PI_REGISTERS_ADDRESS : u32 {
    PI_DRAM_ADDR = 0x04600000,
    PI_CART_ADDR = 0x04600004,
    PI_RD_LEN = 0x04600008,
    PI_WR_LEN = 0x0460000C,
    PI_STATUS = 0x04600010,
    PI_BSD_DOM1_LAT = 0x04600014,
    PI_BSD_DOM1_PWD = 0x04600018,
    PI_BSD_DOM1_PGS = 0x0460001C,
    PI_BSD_DOM1_RLS = 0x04600020,
    PI_BSD_DOM2_LAT = 0x04600024,
    PI_BSD_DOM2_PWD = 0x04600028,
    PI_BSD_DOM2_PGS = 0x0460002C,
    PI_BSD_DOM2_RLS = 0x04600030,
};

class PI {
public:
    PI();
    ~PI();

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

private:
    u32 dram_addr_;
    u32 cart_addr_;
    u32 rd_len_;
    u32 wr_len_;
    u32 status_;
    u32 bsd_dom1_lat_;
    u32 bsd_dom1_pwd_;
    u32 bsd_dom1_pgs_;
    u32 bsd_dom1_rls_;
    u32 bsd_dom2_lat_;
    u32 bsd_dom2_pwd_;
    u32 bsd_dom2_pgs_;
    u32 bsd_dom2_rls_;
};
}