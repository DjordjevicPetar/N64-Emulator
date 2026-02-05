#pragma once

#include <string>
#include <vector>

#include "../utils/types.hpp"
#include "memory_constants.hpp"
#include "../interfaces/pi/pi.hpp"

namespace n64::memory {

enum class ROM_FORMAT {
    N64,    // Little Endian
    Z64,    // Big Endian
    V64     // Byte Swapped
};

class ROM {
public:
    ROM(interfaces::PI& pi, const std::string& path);
    ~ROM();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;

    u32 parse_header();

    [[nodiscard]] size_t size() const { return memory_.size(); }

private:

    [[nodiscard]] ROM_FORMAT detect_format() const;
    void convert_to_big_endian();

    std::vector<u8> memory_;
    std::string path_;
    interfaces::PI& pi_;

    u32 clock_rate_;
    u32 lib_ultra_version_;
    u64 check_code_;
    std::string game_name_;
    char category_code_;
    char unique_code_[2];
    char destination_code_;
    u8 rom_version_;
};
}