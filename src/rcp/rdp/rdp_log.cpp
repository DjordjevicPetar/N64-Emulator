#include "rdp_log.hpp"
#include <cstdlib>

#ifdef RDP_LOG

int rdp_log_level = 2;
int rdp_pixel_budget = 16;

namespace {
struct RdpLogInit {
    RdpLogInit() {
        if (const char* env = std::getenv("RDP_LOG_LEVEL"))
            rdp_log_level = std::atoi(env);
        if (const char* env = std::getenv("RDP_PIXEL_BUDGET"))
            rdp_pixel_budget = std::atoi(env);
    }
} rdp_log_init_;
}

#endif
