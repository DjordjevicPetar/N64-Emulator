#pragma once

#include "../../utils/types.hpp"
#include <SDL3/SDL.h>
#include <vector>

namespace n64::memory {
class RDRAM;  // Forward declaration
}

namespace n64::interfaces {

class VI;  // Forward declaration

class VIRenderer {
public:
    VIRenderer(VI* vi, memory::RDRAM& rdram);
    ~VIRenderer();

    void render_frame();
    bool handle_events();  // Returns false if window closed

private:
    VI* vi_;
    memory::RDRAM& rdram_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    std::vector<u32> pixel_buffer_;  // RGBA pixel buffer
};

} // namespace n64::interfaces