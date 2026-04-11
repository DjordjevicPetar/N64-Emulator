#include "rsp_dma.hpp"
#include "rsp.hpp"
#include "rsp_registers.hpp"
#include "../../memory/rdram.hpp"
#include <cstdio>

namespace n64::rcp {

RSPDMA::RSPDMA(RSP& rsp, memory::RDRAM& rdram)
    : rsp_(rsp)
    , rdram_(rdram)
{
}

RSPDMA::~RSPDMA()
{
}

void RSPDMA::add_request(DMARequest request)
{
    static u32 dma_log_count = 0;
    if (dma_log_count < 20) {
        fprintf(stderr, "[RSP DMA] %s %s: SP=0x%03X RDRAM=0x%06X len=%u count=%u skip=%u\n",
                request.is_read ? "RDRAM->SP" : "SP->RDRAM",
                request.is_imem ? "IMEM" : "DMEM",
                request.sp_address, request.rdram_address,
                request.length, request.count, request.skip);
        dma_log_count++;
    }

    while (request.length > 0) {
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

        request.sp_address = (request.sp_address + 1) & 0xFFF;
        request.rdram_address += 1;
        request.length -= 1;

        if (request.length == 0 && request.count > 0) {
            request.count -= 1;
            request.length = request.start_length;
            request.rdram_address += request.skip;
        }
    }

    rsp_.on_dma_complete(
        request.sp_address, request.rdram_address,
        request.is_imem, request.skip
    );

    rsp_.status().dma_busy = 0;
    rsp_.status().dma_full = 0;
}

void RSPDMA::process_transfer(u32 /* cycles */)
{
}

} // namespace n64::rcp