#include "su.hpp"

namespace n64::rcp {

SU::SU()
    : gpr_()
{
}

SU::~SU()
{
}

u32 SU::read_gpr(u32 index) const
{
    return gpr_[index];
}

void SU::write_gpr(u32 index, u32 value)
{
    if (index == 0) {
        return;
    }
    gpr_[index] = value;
}

}