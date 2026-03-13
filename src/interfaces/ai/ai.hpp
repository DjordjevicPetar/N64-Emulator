#pragma once

#include "../../utils/types.hpp"
#include "../mi.hpp"
#include "ai_registers.hpp"
#include "../../memory/rdram.hpp"
#include <queue>
#include <array>
#include <SDL3/SDL.h>

namespace n64::interfaces {

struct DMA_Request {
    u32 dram_address;
    u32 remaining;
    DMA_Request(u32 dram_address, u32 remaining)
        : dram_address(dram_address)
        , remaining(remaining) {}
};

constexpr u64 NTSC_VI_FREQ = 48681812;
constexpr u64 CPU_FREQ = 93750000;

class AI {
public:
    AI(MI& mi, memory::RDRAM& rdram);
    ~AI();

    template<typename T>
    [[nodiscard]] T read(u32 address) const;
    template<typename T>
    void write(u32 address, T value);

    [[nodiscard]] u32 read_register(u32 address) const;
    void write_register(u32 address, u32 value);

    void process_passed_cycles(u32 cycles);

private:
    [[nodiscard]] inline u32 get_bytes_remaining() const {return (request_queue_.empty() ? 0 : request_queue_.front().remaining); }
    
    // Dependencies
    MI& mi_;
    memory::RDRAM& rdram_;

    // Registers
    AIDramAddr dram_addr_;
    AILength length_;
    AIControl control_;
    AIStatus status_;
    AIDacRate dacrate_;
    AIBitRate bitrate_;

    std::queue<DMA_Request> request_queue_;

    u64 cycles_accumulator_ = 0;
    u64 cycles_per_sample_ = 0;

    // SDL audio output
    SDL_AudioStream* audio_stream_;
    static constexpr int SAMPLE_BUFFER_SIZE = 128;
    std::array<s16, SAMPLE_BUFFER_SIZE> sample_buffer_{};
    int sample_buffer_pos_ = 0;

};

} // namespace n64::interfaces