#include "ri.hpp"
#include <stdexcept>

namespace n64::interfaces {

RI::RI() {}
RI::~RI() {}

u32 RI::read_register(u32 address) const {
    // TODO: Implement RI registers
    return 0;
}

void RI::write_register(u32 address, u32 value) {
    // TODO: Implement RI registers
}

} // namespace n64::interfaces
