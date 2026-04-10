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

enum class CIC_TYPE {
    CIC_6101,
    CIC_6102,  // Most common (SM64, OoT, etc.)
    CIC_6103,
    CIC_6105,
    CIC_6106,
    CIC_UNKNOWN
};

class ROM {
public:
    ROM(interfaces::PI& pi, const std::string& path);
    ~ROM();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;

    u32 parse_header();

    [[nodiscard]] size_t size() const { return memory_.size(); }
    [[nodiscard]] CIC_TYPE cic_type() const { return cic_type_; }
    [[nodiscard]] u8 cic_seed() const;

private:

    [[nodiscard]] ROM_FORMAT detect_format() const;
    void convert_to_big_endian();
    void detect_cic();
    [[nodiscard]] static u32 crc32(const u8* data, size_t len);

    std::vector<u8> memory_;
    std::string path_;
    interfaces::PI& pi_;
    CIC_TYPE cic_type_ = CIC_TYPE::CIC_UNKNOWN;

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