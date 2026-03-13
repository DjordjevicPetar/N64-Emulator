#pragma once
#include <cstdio>

#ifdef RDP_LOG

extern int rdp_log_level;
extern int rdp_pixel_budget;

#define RDP_LOG_CMD(fmt, ...)   do { if (rdp_log_level >= 1) fprintf(stderr, "[RDP:CMD] " fmt "\n", ##__VA_ARGS__); } while(0)
#define RDP_LOG_STATE(fmt, ...) do { if (rdp_log_level >= 2) fprintf(stderr, "[RDP:STATE] " fmt "\n", ##__VA_ARGS__); } while(0)
#define RDP_LOG_PRIM(fmt, ...)  do { if (rdp_log_level >= 3) fprintf(stderr, "[RDP:PRIM] " fmt "\n", ##__VA_ARGS__); } while(0)
#define RDP_LOG_PIXEL(counter, fmt, ...) do { if (rdp_log_level >= 4 && (counter)++ < rdp_pixel_budget) fprintf(stderr, "[RDP:PIX] " fmt "\n", ##__VA_ARGS__); } while(0)

#else

#define RDP_LOG_CMD(...)   ((void)0)
#define RDP_LOG_STATE(...) ((void)0)
#define RDP_LOG_PRIM(...)  ((void)0)
#define RDP_LOG_PIXEL(counter, ...) ((void)0)

#endif
