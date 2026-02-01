#pragma once

#include <queue>
#include "../../utils/types.hpp"

namespace n64::memory {
    class RDRAM;  // Forward declaration
}

namespace n64::rcp {

class RSP;  // Forward declaration

constexpr float DMA_BYTES_PER_CYCLE_FP = 3.7f;

struct DMARequest {
    bool is_read;       // true = RDRAM to SP, false = SP to RDRAM
    bool is_imem;       // true = IMEM, false = DMEM
    u32 sp_address;     // Address in DMEM/IMEM
    u32 rdram_address;  // Address in RDRAM
    u32 start_length;   // Starting transfer length (bytes)
    u32 length;         // Transfer length (bytes)
    u32 count;          // Number of rows
    u32 skip;           // Skip between rows in RDRAM
    DMARequest(bool is_read, bool is_imem, u32 sp_address, u32 rdram_address, u32 start_length, u32 count, u32 skip)
        : is_read(is_read)
        , is_imem(is_imem)
        , sp_address(sp_address)
        , rdram_address(rdram_address)
        , start_length(start_length)
        , length(start_length)
        , count(count)
        , skip(skip) {}
};

class RSPDMA {
public:
    RSPDMA(RSP& rsp, memory::RDRAM& rdram);
    ~RSPDMA();

    void add_request(DMARequest request);
    void process_transfer(u32 cycles);

private:
    RSP& rsp_;
    memory::RDRAM& rdram_;
    std::queue<DMARequest> request_queue_;
    float to_transfer_ = 0.0f;
};

} // namespace n64::rcp