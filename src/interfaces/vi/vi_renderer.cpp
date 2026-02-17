#include "vi_renderer.hpp"
#include "vi.hpp"
#include "../../memory/rdram.hpp"

namespace n64::interfaces {

VIRenderer::VIRenderer(VI* vi, memory::RDRAM& rdram)
    : vi_(vi)
    , rdram_(rdram)
    , window_(nullptr)
    , renderer_(nullptr)
    , texture_(nullptr)
    , texture_width_(0)
    , texture_height_(0)
{
    SDL_Init(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("N64 Emulator", 1280, 960, 0);  // 2x scale
    renderer_ = SDL_CreateRenderer(window_, nullptr);
}

VIRenderer::~VIRenderer() {
    if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool VIRenderer::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
    }
    return true;
}

void VIRenderer::render_frame() {
    // TODO: Implement x_scale/y_scale (2.10 fixed point) for proper framebuffer scaling
    // TODO: Implement interlaced rendering (odd/even fields based on v_current bit 0)
    // TODO: Implement VI filters (gamma, gamma_dither, divot, anti-aliasing, dedither)
    
    u32 origin = vi_->origin().origin;
    u32 width = vi_->width().width;
    u32 type = vi_->ctrl().type;

    // TODO: Handle blank screen for test_mode and kill_we flags
    if (type == 0 || width == 0) {
        // Blank screen
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        SDL_RenderPresent(renderer_);
        return;
    }

    // Calculate height: use 3/4 of width for 4:3 aspect ratio
    // TODO: Calculate from v_video register (v_end - v_start) / 2
    u32 height = width * 3 / 4;

    // Recreate texture if dimensions changed
    if (width != texture_width_ || height != texture_height_) {
        if (texture_) SDL_DestroyTexture(texture_);
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, 
                                      SDL_TEXTUREACCESS_STREAMING, width, height);
        pixel_buffer_.resize(width * height, 0);
        texture_width_ = width;
        texture_height_ = height;
    }

    // TODO: Add RDRAM bounds checking to prevent out-of-range reads
    // Fill pixel buffer from RDRAM
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            u8 red = 0;
            u8 green = 0;
            u8 blue = 0;
            u8 alpha = 255;

            if (type == 3) {
                // 32-bit RGBA 8888
                u32 addr = origin + (y * width + x) * 4;
                u32 pixel = rdram_.read_memory<u32>(addr);
                red = (pixel >> 24) & 0xFF;
                green = (pixel >> 16) & 0xFF;
                blue = (pixel >> 8) & 0xFF;
                alpha = pixel & 0xFF;
            } else if (type == 2) {
                // 16-bit RGBA 5551
                u32 addr = origin + (y * width + x) * 2;
                u16 pixel = rdram_.read_memory<u16>(addr);
                red = ((pixel >> 11) & 0x1F) << 3;
                green = ((pixel >> 6) & 0x1F) << 3;
                blue = ((pixel >> 1) & 0x1F) << 3;
                alpha = (pixel & 0x1) ? 255 : 0;
            }
            // NOTE: type=1 is reserved/undefined on real hardware

            // Pack as RGBA8888 for SDL
            pixel_buffer_[y * width + x] = (static_cast<u32>(red) << 24) |
                                            (static_cast<u32>(green) << 16) |
                                            (static_cast<u32>(blue) << 8) |
                                            static_cast<u32>(alpha);
        }
    }

    // Update texture with pixel buffer (stride = width * 4 bytes per pixel)
    SDL_UpdateTexture(texture_, nullptr, pixel_buffer_.data(), width * sizeof(u32));
    
    // Render - SDL stretches texture to fill the window
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

} // namespace n64::interfaces
