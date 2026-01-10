#include "rdram.hpp"

namespace n64::memory {

RDRAM::RDRAM()
{
    memory.fill(0);
}

RDRAM::~RDRAM()
{
}

u32 RDRAM::translate_address(u32 address) const {

    u32 translated_address = 0;
    
    if (address >= RDRAM_MEMORY_START_ADDRESS && address <= RDRAM_MEMORY_END_ADDRESS) {
        translated_address = (address >> 20) & 0x3F;
        translated_address <<= 10;
        translated_address += (address >> 11) & 0x1FF;
        translated_address <<= 10;
        translated_address += address & 0x7FF;
    }
    else if (address >= RDRAM_REGISTER_START_ADDRESS && address <= RDRAM_REGISTER_END_ADDRESS) {
        translated_address = address - RDRAM_REGISTER_START_ADDRESS;
    }
    else {
        throw std::runtime_error("Invalid RDRAM address: " + std::to_string(address));
    }

}

}