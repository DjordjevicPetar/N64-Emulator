#include "vi.hpp"
#include <stdexcept>

namespace n64::interfaces {

VI::VI() {}
VI::~VI() {}

u32 VI::read_register(u32 address) const {
    // TODO: Implement VI registers
    return 0;
}

void VI::write_register(u32 address, u32 value) {
    // TODO: Implement VI registers
}

} // namespace n64::interfaces
