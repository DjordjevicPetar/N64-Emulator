#include "pi.hpp"
#include "../../memory/memory_constants.hpp"
#include "../../memory/rom.hpp"
#include "../../memory/rdram.hpp"
#include <stdexcept>
#include <string>

namespace n64::interfaces {

PI::PI(MI& mi)
    : mi_(mi)
    , dram_addr_{.raw = 0}
    , cart_addr_{.raw = 0}
    , rd_len_{.raw = 0}
    , wr_len_{.raw = 0}
    , status_{.raw = 0}
    , bsd_dom1_lat_{.raw = 0}
    , bsd_dom1_pwd_{.raw = 0}
    , bsd_dom1_pgs_{.raw = 0}
    , bsd_dom1_rls_{.raw = 0}
    , bsd_dom2_lat_{.raw = 0}
    , bsd_dom2_pwd_{.raw = 0}
    , bsd_dom2_pgs_{.raw = 0}
    , bsd_dom2_rls_{.raw = 0}
    , dma_busy_(false)
    , pi_cycle_accumulator_(0.0f)
    , latency_counter_(0)
    , pulse_width_counter_(0)
    , release_counter_(0)
    , is_reading_(false)
    , is_writing_(false)
{
}

PI::~PI() {}

void PI::set_dma_targets(memory::ROM& rom, memory::RDRAM& rdram) {
    rom_ = &rom;
    rdram_ = &rdram;
}

u32 PI::read_register(u32 address) const {
    switch (address) {
        case PI_DRAM_ADDR:
            return dram_addr_.raw;
        case PI_CART_ADDR:
            return cart_addr_.raw;
        case PI_RD_LEN:
            return rd_len_.raw;
        case PI_WR_LEN:
            return wr_len_.raw;
        case PI_STATUS:
            return status_.raw;
        case PI_BSD_DOM1_LAT:
            return bsd_dom1_lat_.raw;
        case PI_BSD_DOM1_PWD:
            return bsd_dom1_pwd_.raw;
        case PI_BSD_DOM1_PGS:
            return bsd_dom1_pgs_.raw;
        case PI_BSD_DOM1_RLS:
            return bsd_dom1_rls_.raw;
        case PI_BSD_DOM2_LAT:
            return bsd_dom2_lat_.raw;
        case PI_BSD_DOM2_PWD:
            return bsd_dom2_pwd_.raw;
        case PI_BSD_DOM2_PGS:
            return bsd_dom2_pgs_.raw;
        case PI_BSD_DOM2_RLS:
            return bsd_dom2_rls_.raw;
        default:
            throw std::runtime_error("Invalid PI register address: " + std::to_string(address));
    }
}

void PI::write_register(u32 address, u32 value) {
    switch (address) {
        case PI_DRAM_ADDR:
            dram_addr_.raw = value & 0x00FFFFFE;
            break;
        case PI_CART_ADDR:
            cart_addr_.raw = value & 0xFFFFFFFE;
            break;
        case PI_RD_LEN:
            rd_len_.raw = value & 0x00FFFFFF;
            // Start read DMA: Cart -> RDRAM
            dma_busy_ = true;
            status_.dma_busy = 1;
            set_counters();
            is_reading_ = true;
            break;
        case PI_WR_LEN:
            wr_len_.raw = value & 0x00FFFFFF;
            // Start write DMA: RDRAM -> Cart (SRAM/FlashRAM)
            dma_busy_ = true;
            status_.dma_busy = 1;
            set_counters();
            is_writing_ = true;
            break;
        case PI_STATUS:
            // Writing to status register
            if (value & 0x02) {  // Bit 1: Clear interrupt
                mi_.clear_interrupt(MI_INTERRUPT_PI);
                status_.interrupt = 0;
            }
            if (value & 0x01) {  // Bit 0: Reset DMA controller
                dma_busy_ = false;
                status_.dma_busy = 0;
                is_reading_ = false;
                is_writing_ = false;
            }
            break;
        case PI_BSD_DOM1_LAT:
            bsd_dom1_lat_.raw = value & 0x000000FF;
            break;
        case PI_BSD_DOM1_PWD:
            bsd_dom1_pwd_.raw = value & 0x000000FF;
            break;
        case PI_BSD_DOM1_PGS:
            bsd_dom1_pgs_.raw = value & 0x0000000F;
            break;
        case PI_BSD_DOM1_RLS:
            bsd_dom1_rls_.raw = value & 0x00000003;
            break;
        case PI_BSD_DOM2_LAT:
            bsd_dom2_lat_.raw = value & 0x000000FF;
            break;
        case PI_BSD_DOM2_PWD:
            bsd_dom2_pwd_.raw = value & 0x000000FF;
            break;
        case PI_BSD_DOM2_PGS:
            bsd_dom2_pgs_.raw = value & 0x0000000F;
            break;
        case PI_BSD_DOM2_RLS:
            bsd_dom2_rls_.raw = value & 0x00000003;
            break;
        default:
            throw std::runtime_error("Invalid PI register address: " + std::to_string(address));
    }
}

void PI::process_passed_cycles(u32 cycles) {
    if (!dma_busy_) return;
    
    pi_cycle_accumulator_ += cycles * PI_CYCLE_PER_CYCLE_FP;

    // Wait for latency
    while (latency_counter_ > 0 && pi_cycle_accumulator_ >= 1) {
        latency_counter_--;
        pi_cycle_accumulator_ -= 1;
    }
    if (latency_counter_ > 0) return;

    // Wait for pulse width (transfer time)
    while (pulse_width_counter_ > 0 && pi_cycle_accumulator_ >= 1) {
        pulse_width_counter_--;
        pi_cycle_accumulator_ -= 1;
    }
    if (pulse_width_counter_ > 0) return;

    // Transfer one page of data
    transfer_page();

    // Wait for release
    while (release_counter_ > 0 && pi_cycle_accumulator_ >= 1) {
        release_counter_--;
        pi_cycle_accumulator_ -= 1;
    }
    if (release_counter_ > 0) return;

    // Check if transfer complete
    u32& len = is_reading_ ? rd_len_.raw : wr_len_.raw;
    if (len == 0) {
        dma_busy_ = false;
        status_.dma_busy = 0;
        is_reading_ = false;
        is_writing_ = false;
        pi_cycle_accumulator_ = 0.0f;
        mi_.set_interrupt(MI_INTERRUPT_PI);
    } else {
        // More pages to transfer, reset counters for next page
        set_counters();
    }
}

u8 PI::get_address_domain(u32 address) const {
    // 64DD registers - Domain 2
    if (address >= memory::PI_DOM2_ADDR1_START && address <= memory::PI_DOM2_ADDR1_END) {
        return 2;
    }
    // SRAM - Domain 2
    if (address >= memory::PI_DOM2_ADDR2_START && address <= memory::PI_DOM2_ADDR2_END) {
        return 2;
    }
    // Everything else is Domain 1 (ROM, 64DD ROM, etc.)
    return 1;
}

void PI::set_counters() {
    if (get_address_domain(cart_addr_.raw) == 1) {
        latency_counter_ = bsd_dom1_lat_.latency;
        pulse_width_counter_ = bsd_dom1_pwd_.pulse_width;
        release_counter_ = bsd_dom1_rls_.release;
    } else {
        latency_counter_ = bsd_dom2_lat_.latency;
        pulse_width_counter_ = bsd_dom2_pwd_.pulse_width;
        release_counter_ = bsd_dom2_rls_.release;
    }
}

void PI::transfer_page() {
    if (!rom_ || !rdram_) return;
    
    // Get page size based on domain
    const PIBsdPgs& pgs = (get_address_domain(cart_addr_.raw) == 1) 
        ? bsd_dom1_pgs_ 
        : bsd_dom2_pgs_;
    
    u32 page_size = 1u << (pgs.page_size + 2);
    u32 page_mask = page_size - 1;
    
    // Calculate bytes until page boundary
    u32 bytes_in_page = page_size - (cart_addr_.raw & page_mask);
    
    // Don't transfer more than remaining length
    u32& len = is_reading_ ? rd_len_.raw : wr_len_.raw;
    u32 bytes_to_transfer = (bytes_in_page < len + 1) ? bytes_in_page : (len + 1);
    
    if (is_reading_) {
        // Cart -> RDRAM
        for (u32 i = 0; i < bytes_to_transfer; i++) {
            u8 byte = rom_->read<u8>(cart_addr_.raw);
            rdram_->write_memory<u8>(dram_addr_.raw, byte);
            cart_addr_.raw++;
            dram_addr_.raw++;
        }
    } else if (is_writing_) {
        // RDRAM -> Cart (SRAM/FlashRAM)
        // TODO: Implement SRAM/FlashRAM writes
        cart_addr_.raw += bytes_to_transfer;
        dram_addr_.raw += bytes_to_transfer;
    }
    
    // Update remaining length
    if (len >= bytes_to_transfer) {
        len -= bytes_to_transfer;
    } else {
        len = 0;
    }
}

} // namespace n64::interfaces
