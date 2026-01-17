#include "ai.hpp"
#include <stdexcept>

namespace n64::interfaces {

AI::AI() {}
AI::~AI() {}

u32 AI::read_register(u32 address) const {
    // TODO: Implement AI registers
    return 0;
}

void AI::write_register(u32 address, u32 value) {
    // TODO: Implement AI registers
}

} // namespace n64::interfaces
