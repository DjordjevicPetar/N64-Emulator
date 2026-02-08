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
    // TODO: Make pixel buffer size dynamic based on VI_WIDTH and calculated height
    , pixel_buffer_(640 * 480, 0)
{
    SDL_Init(SDL_INIT_VIDEO);
    // TODO: Make window size configurable or calculate from VI h_video/v_video registers
    window_ = SDL_CreateWindow("N64 Emulator", 1280, 960, 0);  // 2x scale
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    // TODO: Make texture size dynamic based on actual video window dimensions
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, 
                                  SDL_TEXTUREACCESS_STREAMING, 640, 480);
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
    // TODO: Implement horizontal scaling using x_scale register (2.10 fixed point)
    // TODO: Implement vertical scaling using y_scale register (2.10 fixed point)
    // TODO: Use h_video (h_start, h_end) and v_video (v_start, v_end) for active video window
    // TODO: Implement interlaced rendering — render odd/even fields based on v_current.field
    // TODO: Implement gamma correction when gamma_enable is set
    // TODO: Implement gamma dither when gamma_dither_enable is set
    // TODO: Implement divot filter when divot_enable is set
    // TODO: Implement anti-aliasing based on aa_mode (4 modes)
    // TODO: Implement dedither filter when dedither_filter_enable is set
    
    u32 origin = vi_->origin().origin;
    u32 width = vi_->width().width;
    u32 type = vi_->ctrl().type;

    // TODO: Handle blank screen conditions more thoroughly (test_mode, kill_we, etc.)
    if (type == 0 || width == 0) {
        // Blank screen
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        SDL_RenderPresent(renderer_);
        return;
    }

    // TODO: Calculate height from v_video register (v_video.v_end - v_video.v_start)
    u32 height = 480;
    
    // TODO: Remove hardcoded limit; handle large widths properly
    // Make sure width doesn't exceed buffer
    if (width > 640) width = 640;

    // TODO: Add bounds checking to prevent reading beyond RDRAM limits
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
            // TODO: Verify pixel format type 1 behavior (reserved? blank?)

            // Pack as RGBA8888 for SDL
            pixel_buffer_[y * 640 + x] = (static_cast<u32>(red) << 24) |
                                          (static_cast<u32>(green) << 16) |
                                          (static_cast<u32>(blue) << 8) |
                                          static_cast<u32>(alpha);
        }
    }

    // TODO: Calculate pitch dynamically based on actual buffer width
    // Update texture with pixel buffer
    SDL_UpdateTexture(texture_, nullptr, pixel_buffer_.data(), 640 * sizeof(u32));
    
    // Render
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

} // namespace n64::interfaces