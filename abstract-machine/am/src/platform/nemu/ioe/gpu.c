#include <am.h>
#include <nemu.h>
#include <klib.h>

#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0x00000000

#define SYNC_ADDR (VGACTL_ADDR + 4)
/* 按照amdev.h中定义变量的顺序分配mmio中的内存
 * GPU_CONFIG: VGACTL_ADDR
 * GPU_STATUS: VGACTL_ADDR + 8
 * GPU_FBDRAW: VGACTL_ADDR + 16
 * GPU_MEMCPY: VGACTL_ADDR + 24
 * GPU_RENDER: VGACTL_ADDR + 32
 * */
#define CONFIG_ADDR (VGACTL_ADDR + 0)
#define FBDRAW_ADDR (VGACTL_ADDR + 8)
#define MEMCPY_ADDR (VGACTL_ADDR + 12)
#define RENDER_ADDR (VGACTL_ADDR + 16)

unsigned int width;
unsigned int height;

void __am_gpu_init() {
//    int i;
//    int w = 800;  // TODO: get the correct width
//    int h = 600;  // TODO: get the correct height
//    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
//    for (i = 0; i < w * h; i ++) fb[i] = i;
//    outl(SYNC_ADDR, 1);
}

// 获取显示控制器信息
// 根据nemu模块中的代码，GPU_CONFIG的高16位为宽度，低16位为高度
void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    uint32_t config = *(volatile uint32_t *)CONFIG_ADDR;
    width = ((config & 0xffff0000) >> 16);
    height = (config & 0xffff);
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
    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
    int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
    uint32_t *pixels = ctl->pixels;
    int scan_width = w < (width - x) ? w : (width - x);
    int cp_bytes = sizeof(uint32_t) * scan_width;
    for (int p = 0; p < h && p + y < height; p++) {
        memcpy(&fb[(y + p) * width + x], pixels, cp_bytes);
        pixels += w;
    }
    if (ctl->sync) {
        outl(SYNC_ADDR, 1);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}
