#include "rsp_dma.hpp"
#include "rsp.hpp"
#include "rsp_registers.hpp"
#include "../../memory/rdram.hpp"
#include <stdexcept>

namespace n64::rcp {

RSPDMA::RSPDMA(RSP& rsp, memory::RDRAM& rdram)
    : rsp_(rsp)
    , rdram_(rdram)
    , request_queue_()
    , to_transfer_(0.0f)
{
}

RSPDMA::~RSPDMA()
{
}

void RSPDMA::add_request(DMARequest request)
{
    // Ako queue pun, završi trenutni transfer (simulira CPU stall)
    while (request_queue_.size() >= 2) {
        rsp_.process_passed_cycles(1);
    }
    request_queue_.push(request);
    
    rsp_.status().dma_busy = 1;
    if (request_queue_.size() == 2) {
        rsp_.status().dma_full = 1;
    }
}

void RSPDMA::process_transfer(u32 cycles)
{
    if (request_queue_.empty()) {
        to_transfer_ = 0.0f;
        return;
    }
    auto& request = request_queue_.front();
    to_transfer_ += cycles * DMA_BYTES_PER_CYCLE_FP;

    while (to_transfer_ >= 1 && request.length > 0) {
        to_transfer_ -= 1;
        if (request.is_read) {
            u8 data = rdram_.read_memory<u8>(request.rdram_address);
            if (request.is_imem) {
                rsp_.write_imem(request.sp_address, data);
            } else {
                rsp_.write_dmem(request.sp_address, data);
            }
        } else {
            u8 data = 0;
            if (request.is_imem) {
                data = rsp_.read_imem(request.sp_address);
            } else {
                data = rsp_.read_dmem(request.sp_address);
            }

            rdram_.write_memory<u8>(request.rdram_address, data);
        }

        request.sp_address = (request.sp_address + 1) & 0xFFF;  // wrap within 4KB
        request.rdram_address += 1;
        request.length -= 1;
        if (request.length == 0) {
            if (request.count == 0) {
                request_queue_.pop();
                if (request_queue_.empty()) {
                    rsp_.status().dma_busy = 0;
                } else {
                    rsp_.status().dma_full = 0;
                }
                break;
            }
            request.count -= 1;
            request.length = request.start_length;
            request.rdram_address += request.skip;
        }
    }

    // Ostatak ostaje za sledeći poziv - ne rekurzija
}

} // namespace n64::rcp