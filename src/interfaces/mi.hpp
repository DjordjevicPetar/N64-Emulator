#pragma once

#include "../utils/types.hpp"

namespace n64::interfaces {

enum MI_REGISTERS_ADDRESS : u32 {
    MI_MODE = 0x04300000,
    MI_VERSION = 0x04300004,
    MI_INTERRUPT = 0x04300008,
    MI_MASK = 0x0430000C,
};

enum MI_INTERRUPT_BITS : u32 {
    MI_INTERRUPT_SP = 0,
    MI_INTERRUPT_SI = 1,
    MI_INTERRUPT_AI = 2,
    MI_INTERRUPT_VI = 3,
    MI_INTERRUPT_PI = 4,
    MI_INTERRUPT_DP = 5
};

class MI {
public:
    MI();
    ~MI();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

    void set_interrupt(MI_INTERRUPT_BITS interrupt);
    void clear_interrupt(MI_INTERRUPT_BITS interrupt);

private:
    u32 mode_;
    u32 version_;
    u32 interrupt_;
    u32 mask_;
};
}