#include "si.hpp"
#include <stdexcept>

namespace n64::interfaces {

SI::SI() {}
SI::~SI() {}

u32 SI::read_register(u32 address) const {
    // TODO: Implement SI registers
    return 0;
}

void SI::write_register(u32 address, u32 value) {
    // TODO: Implement SI registers
}

} // namespace n64::interfaces
