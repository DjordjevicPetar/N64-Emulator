#include "ai.hpp"

namespace n64::interfaces {

AI::AI(MI& mi, memory::RDRAM& rdram)
    : mi_(mi)
    , rdram_(rdram)
    , dram_addr_{.raw = 0}
    , length_{.raw = 0}
    , control_{.raw = 0}
    , status_{.raw = 0}
    , dacrate_{.raw = 0}
    , bitrate_{.raw = 0}
{
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_S16LE;
    spec.channels = 2;
    spec.freq = 44100;

    audio_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);

    if (audio_stream_) {
        SDL_ResumeAudioStreamDevice(audio_stream_);
    }
}
AI::~AI() {
    if (audio_stream_) {
        SDL_DestroyAudioStream(audio_stream_);
    }
}

template<typename T>
T AI::read(u32 address) const {
    // TODO: Do smarter
    u32 reg_value = read_register(address & ~3u);
    if constexpr (sizeof(T) == 4 || sizeof(T) == 8) {
        return static_cast<T>(reg_value);
    } else if constexpr (sizeof(T) == 2) {
        u32 shift = (2 - (address & 2)) * 8;
        return static_cast<T>((reg_value >> shift) & 0xFFFF);
    } else {
        u32 shift = (3 - (address & 3)) * 8;
        return static_cast<T>((reg_value >> shift) & 0xFF);
    }
}

template<typename T>
void AI::write(u32 address, T value) {
    write_register(address, value);
}

u32 AI::read_register(u32 address) const {
    switch (address) {
        case AI_REGISTERS_ADDRESS::AI_DRAM_ADDR:
            return get_bytes_remaining();
        case AI_REGISTERS_ADDRESS::AI_LENGTH:
            return get_bytes_remaining();
        case AI_REGISTERS_ADDRESS::AI_CONTROL:
            return get_bytes_remaining();
        case AI_REGISTERS_ADDRESS::AI_STATUS:
            return status_.raw;
        case AI_REGISTERS_ADDRESS::AI_DACRATE:
            return get_bytes_remaining();
        case AI_REGISTERS_ADDRESS::AI_BITRATE:
            return get_bytes_remaining();
        default:
            return 0;
    }
}

void AI::write_register(u32 address, u32 value) {
    switch (address) {
        case AI_REGISTERS_ADDRESS::AI_DRAM_ADDR:
            dram_addr_.raw = value & 0x00FFFFF8;
            break;
        case AI_REGISTERS_ADDRESS::AI_LENGTH: {
            u32 new_length = value & 0x0003FFF8;
            if (new_length == 0 || !control_.dma_enable) break;

            if (!status_.full) {
                request_queue_.push(DMA_Request(dram_addr_.address, new_length));
                status_.busy = 1;
                if (request_queue_.size() == 2) status_.full = 1;
            }

            break;
        }
        case AI_REGISTERS_ADDRESS::AI_CONTROL:
            control_.raw = value & 0x00000001;
            status_.enabled = control_.dma_enable;
            break;
        case AI_REGISTERS_ADDRESS::AI_STATUS:
            mi_.clear_interrupt(MI_INTERRUPT_AI);
            break;
        case AI_REGISTERS_ADDRESS::AI_DACRATE: {
            u32 new_dacrate = value & 0x00003FFF;
            if (new_dacrate == dacrate_.raw) break;
            dacrate_.raw = new_dacrate;
            u32 sample_rate = NTSC_VI_FREQ / (dacrate_.dac_rate + 1);
            cycles_per_sample_ = CPU_FREQ / sample_rate;

            if (audio_stream_) {
                SDL_DestroyAudioStream(audio_stream_);
            }

            SDL_AudioSpec spec;
            spec.format = SDL_AUDIO_S16LE;
            spec.channels = 2;
            spec.freq = static_cast<s32>(sample_rate);

            audio_stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);

            if (audio_stream_) {
                SDL_ResumeAudioStreamDevice(audio_stream_);
            }
            break;
        }
        case AI_REGISTERS_ADDRESS::AI_BITRATE:
            bitrate_.raw = value & 0x0000000F;
            break;
        default:
            return;
    }
}

void AI::process_passed_cycles(u32 cycles) {
    if (cycles_per_sample_ == 0) return;

    cycles_accumulator_ += cycles;
    while (cycles_accumulator_ >= cycles_per_sample_) {
        cycles_accumulator_ -= cycles_per_sample_;

        if (!status_.busy || !control_.dma_enable) {
            continue;
        }

        auto& request = request_queue_.front();

        u16 left = rdram_.read_memory<u16>(request.dram_address);
        u16 right = rdram_.read_memory<u16>(request.dram_address + 2);
        request.dram_address += 4;
        request.remaining -= 4;

        sample_buffer_[sample_buffer_pos_++] = static_cast<s16>(left);
        sample_buffer_[sample_buffer_pos_++] = static_cast<s16>(right);

        if (sample_buffer_pos_ >= SAMPLE_BUFFER_SIZE) {
            if (audio_stream_) {
                SDL_PutAudioStreamData(audio_stream_, sample_buffer_.data(), sample_buffer_pos_ * sizeof(s16));
            }
            sample_buffer_pos_ = 0;
        }
        
        if (request.remaining <= 0) {
            if (sample_buffer_pos_ > 0 && audio_stream_) {
                SDL_PutAudioStreamData(audio_stream_, sample_buffer_.data(), sample_buffer_pos_ * sizeof(s16));
                sample_buffer_pos_ = 0;
            }
            request_queue_.pop();
            status_.full = 0;
            if (request_queue_.empty()) {
                status_.busy = 0;
            } else {
                mi_.set_interrupt(MI_INTERRUPT_AI);
            }
        }
    }
}

template u8 AI::read<u8>(u32) const;
template u16 AI::read<u16>(u32) const;
template u32 AI::read<u32>(u32) const;
template u64 AI::read<u64>(u32) const;
template void AI::write<u8>(u32, u8);
template void AI::write<u16>(u32, u16);
template void AI::write<u32>(u32, u32);
template void AI::write<u64>(u32, u64);

} // namespace n64::interfaces
