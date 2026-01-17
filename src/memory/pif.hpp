#pragma once

#include <array>

#include "../utils/types.hpp"

namespace n64::memory {

class PIF {
public:
    PIF();
    ~PIF();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

private:
    std::array<u8, 64> memory_;
};

} // namespace n64::memory