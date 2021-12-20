#include <am.h>
#include <nemu.h>
#include <klib.h>

#define SCREEN_ADDR (VGACTL_ADDR + 0)
#define SYNC_ADDR (VGACTL_ADDR + 4)

unsigned int width;
unsigned int height;

void __am_gpu_init() {
    uint32_t config = *(volatile uint32_t *)SCREEN_ADDR;
    width = ((config & 0xffff0000) >> 16);
    height = (config & 0xffff);
}

// 获取显示控制器信息
// 根据nemu模块中的代码，GPU_CONFIG的高16位为宽度，低16位为高度
void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    *cfg = (AM_GPU_CONFIG_T) {
            .present = true, .has_accel = false,
            .width = width, .height = height,
            .vmemsz = 0
    };
}

// nemu那边把x、y、显存、屏幕宽度、屏幕高度和是否同步一并传过来
// 在这个函数里面根据是否要同步，将显存信息刷新到屏幕中
// 显存按行优先方式存储了像素点的00RRGGBB信息
// 具体地，在这个函数里，需要从左向右，从上向下扫描显存中的内容，并将其写入到对应的像素点
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
    if (w != 0 && h != 0) {
        uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
        uint32_t *pixels = ctl->pixels;
        int scan_width = w < (width - x) ? w : (width - x);
        int cp_bytes = sizeof(uint32_t) * scan_width;
        for (int p = 0; p < h && p + y < height; p++) {
            memcpy(&fb[(y + p) * width + x], pixels, cp_bytes);
            pixels += w;
        }
    }
    if (ctl->sync) {
        *(volatile uint32_t *)SYNC_ADDR = 1;
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}
