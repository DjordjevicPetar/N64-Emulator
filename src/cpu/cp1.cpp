#include "cp1.hpp"

namespace n64::cpu {

CP1::CP1()
    : fpr_{}
    , status_{.raw = 0}
    , revision_{.raw = 0x0A00}  // VR4300 FPU revision
{
}

u32 CP1::get_fcr(u8 index) const {
    switch (index) {
        case 0:  return revision_.raw;
        case 31: return status_.raw;
        default: return 0;
    }
}

void CP1::set_fcr(u8 index, u32 value) {
    switch (index) {
        case 31:
            // Mask writable bits (condition bit 23, FS bit 24, and lower 18 bits)
            status_.raw = value & 0x0183FFFF;
            break;
        default:
            // FCR0 is read-only, other FCRs don't exist
            break;
    }
}

u32 CP1::get_fpr_32(u8 index) const {
    u8 reg_index = index & ~1;
    bool is_low = (index & 1) == 0;
    if (is_low) {
        return static_cast<u32>(fpr_[reg_index]);
    } else {
        return static_cast<u32>(fpr_[reg_index] >> 32);
    }
}

u64 CP1::get_fpr_64(u8 index) const {
    return fpr_[index];
}

void CP1::set_fpr_32(u8 index, u32 value) {
    u8 reg_index = index & ~1;
    bool is_low = (index & 1) == 0;
    if (is_low) {
        fpr_[reg_index] = (fpr_[reg_index] & 0xFFFFFFFF00000000ULL) | value;
    } else {
        fpr_[reg_index] = (fpr_[reg_index] & 0xFFFFFFFF) | (static_cast<u64>(value) << 32);
    }
}

void CP1::set_fpr_64(u8 index, u64 value) {
    fpr_[index] = value;
}

} // namespace n64::cpu
