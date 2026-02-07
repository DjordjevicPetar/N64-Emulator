#include "cp0.hpp"
#include <stdexcept>

namespace n64::cpu {

CP0::CP0()
    : index_{.raw = 0}
    , random_{.raw = 31}            // Starts at 31
    , entry_lo0_{.raw = 0}
    , entry_lo1_{.raw = 0}
    , context_{.raw = 0}
    , page_mask_{.raw = 0}
    , wired_{.raw = 0}
    , bad_vaddr_(0)
    , count_(0)
    , entry_hi_{.raw = 0}
    , compare_(0)
    , status_{.raw = 0x00400004}    // ERL=1, BEV=1 on reset
    , cause_{.raw = 0}
    , epc_(0)
    , prid_{.raw = 0x0B22}          // VR4300 rev 2.2
    , config_{.raw = 0x7006E463}    // N64 default config
    , ll_addr_(0)
    , watch_lo_{.raw = 0}
    , watch_hi_{.raw = 0}
    , xcontext_{.raw = 0}
    , parity_error_{.raw = 0}
    , cache_error_(0)
    , tag_lo_{.raw = 0}
    , tag_hi_(0)
    , error_epc_(0)
{
}

void CP0::set_reg(u8 index, u64 value) {
    switch (index) {
        case 0:  index_.raw = static_cast<u32>(value) & 0x8000003F; break;
        case 1:  /* RANDOM is read-only */ break;
        case 2:  entry_lo0_.raw = value & 0x3FFFFFFF; break;
        case 3:  entry_lo1_.raw = value & 0x3FFFFFFF; break;
        case 4:  context_.raw = value; break;
        case 5:  page_mask_.raw = static_cast<u32>(value) & 0x01FFE000; break;
        case 6:  
            wired_.raw = static_cast<u32>(value) & 0x3F;
            random_.raw = 31;  // Reset random when wired is written
            break;
        case 8:  bad_vaddr_ = value; break;
        case 9:  count_ = static_cast<u32>(value); break;
        case 10: entry_hi_.raw = value; break;
        case 11: 
            compare_ = static_cast<u32>(value);
            // Writing COMPARE clears timer interrupt (IP7)
            cause_.raw &= ~0x8000;
            break;
        case 12: status_.raw = static_cast<u32>(value) & 0xFF57FFFF; break;
        case 13: 
            // Only IP0 and IP1 (bits 8-9) are writable in CAUSE
            cause_.raw = (cause_.raw & ~0x300) | (static_cast<u32>(value) & 0x300);
            break;
        case 14: epc_ = value; break;
        case 15: /* PRID is read-only */ break;
        case 16: config_.raw = static_cast<u32>(value) & 0x0F00800F; break;
        case 17: ll_addr_ = static_cast<u32>(value); break;
        case 18: watch_lo_.raw = static_cast<u32>(value); break;
        case 19: watch_hi_.raw = static_cast<u32>(value) & 0x0F; break;
        case 20: xcontext_.raw = value; break;
        case 26: parity_error_.raw = static_cast<u32>(value) & 0xFF; break;
        case 27: cache_error_ = static_cast<u32>(value); break;
        case 28: tag_lo_.raw = static_cast<u32>(value) & 0x0FFFFFC0; break;
        case 29: tag_hi_ = static_cast<u32>(value); break;
        case 30: error_epc_ = value; break;
        default: break;  // Reserved registers
    }
}

u64 CP0::get_reg(u8 index) const {
    switch (index) {
        case 0:  return index_.raw;
        case 1:  return random_.raw;
        case 2:  return entry_lo0_.raw;
        case 3:  return entry_lo1_.raw;
        case 4:  return context_.raw;
        case 5:  return page_mask_.raw;
        case 6:  return wired_.raw;
        case 8:  return bad_vaddr_;
        case 9:  return count_;
        case 10: return entry_hi_.raw;
        case 11: return compare_;
        case 12: return status_.raw;
        case 13: return cause_.raw;
        case 14: return epc_;
        case 15: return prid_.raw;
        case 16: return config_.raw;
        case 17: return ll_addr_;
        case 18: return watch_lo_.raw;
        case 19: return watch_hi_.raw;
        case 20: return xcontext_.raw;
        case 26: return parity_error_.raw;
        case 27: return cache_error_;
        case 28: return tag_lo_.raw;
        case 29: return tag_hi_;
        case 30: return error_epc_;
        default: return 0;  // Reserved registers
    }
}

u32 CP0::translate_address(u64 virtual_address) const {
    u32 segment = (virtual_address >> 29) & 0x7;
    
    switch (segment) {
        case 0: case 1: case 2: case 3:
            // KUSEG - TLB mapped (user segment)
            // TODO: Implement TLB lookup
            throw std::runtime_error("KUSEG address space not supported (TLB required)");
        case 4:
            // KSEG0 - Direct mapped, cached (0x80000000 - 0x9FFFFFFF)
            return static_cast<u32>(virtual_address & 0x1FFFFFFF);
        case 5:
            // KSEG1 - Direct mapped, uncached (0xA0000000 - 0xBFFFFFFF)
            return static_cast<u32>(virtual_address & 0x1FFFFFFF);
        case 6:
            // KSSEG - TLB mapped (supervisor segment)
            throw std::runtime_error("KSSEG address space not supported (TLB required)");
        case 7:
            // KSEG3 - TLB mapped (kernel segment)
            throw std::runtime_error("KSEG3 address space not supported (TLB required)");
        default:
            throw std::runtime_error("Invalid address segment");
    }
}

} // namespace n64::cpu
