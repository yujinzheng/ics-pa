#include <am.h>
#include <NDL.h>
#include <unistd.h>
#include <klib-macros.h>
#include <sys/time.h>

//int NDL_Init(uint32_t flags);
//void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h);
//void NDL_OpenCanvas(int *w, int *h);
//uint32_t NDL_GetTicks();
//int NDL_PollEvent(char *buf, int len);

void __am_timer_init();
void __am_gpu_init();
void __am_audio_init();
void __am_input_keybrd(AM_INPUT_KEYBRD_T *);
void __am_timer_rtc(AM_TIMER_RTC_T *);
void __am_timer_uptime(AM_TIMER_UPTIME_T *);
void __am_gpu_config(AM_GPU_CONFIG_T *);
void __am_gpu_status(AM_GPU_STATUS_T *);
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *);
void __am_audio_config(AM_AUDIO_CONFIG_T *);
void __am_audio_ctrl(AM_AUDIO_CTRL_T *);
void __am_audio_status(AM_AUDIO_STATUS_T *);
void __am_audio_play(AM_AUDIO_PLAY_T *);

static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) { cfg->present = true; cfg->has_rtc = true; }
static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true;  }
static void __am_uart_config(AM_UART_CONFIG_T *cfg)   { cfg->present = false; }

uint32_t start_time_sec;
uint32_t start_time_usec;
//uint32_t w, h;

//void __am_gpu_init() {
//    NDL_OpenCanvas(&w, &h);
//}

//void __am_timer_init() {
//    struct timeval current_time;
//    gettimeofday(&current_time, NULL);
//    start_time_sec = current_time.tv_sec;
//    start_time_usec = current_time.tv_usec;
//}

//void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
//    struct timeval current_time;
//    gettimeofday(&current_time, NULL);
//    uptime->us = (current_time.tv_sec - start_time_sec) * 1000000 + (current_time.tv_usec - start_time_usec);
//}
//
//void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
//    *cfg = (AM_GPU_CONFIG_T) {
//            .present = true, .has_accel = false,
//            .width = w, .height = h,
//            .vmemsz = 0
//    };
//}

//void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
//    char buf[64];
//    int event_result = NDL_PollEvent(buf, sizeof(buf));
//    if (event_result == 0) {
//        kbd->keycode = 0;
//        return;
//    }
//    int key_down, key_code = 0;
//    sscanf(buf, "%d %d", &key_down, &key_code);
//    kbd->keydown = key_down;
//    kbd->keycode = key_code;
//    printf("__am_input_keybrd\t keydown: %d, keycode: %d\n", kbd->keydown, kbd->keycode);
//}

//void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *fbdraw) {
////    printf("=====================__am_gpu_fbdraw, x: %d, y: %d, w: %d, h: %d\n", fbdraw->x, fbdraw->y, fbdraw->w, fbdraw->h);
//    NDL_DrawRect(fbdraw->pixels, fbdraw->x, fbdraw->y, fbdraw->w, fbdraw->h);
//}

//void __am_timer_rtc(AM_TIMER_RTC_T *test) {
//    printf("=====================__am_timer_rtc\n");
//}

//void __am_gpu_status(AM_GPU_STATUS_T *test) {
//    printf("=====================__am_gpu_status\n");
//}
void __am_audio_config(AM_AUDIO_CONFIG_T *test) {
    printf("=====================__am_audio_config\n");
}
void __am_audio_ctrl(AM_AUDIO_CTRL_T *test) {
    printf("=====================__am_audio_ctrl\n");
}
void __am_audio_status(AM_AUDIO_STATUS_T *test) {
    printf("=====================__am_audio_status\n");
}
void __am_audio_play(AM_AUDIO_PLAY_T *test) {
    printf("=====================__am_audio_play\n");
}

typedef void (*handler_t)(void *buf);

static void *lut[128] = {
        [AM_TIMER_CONFIG] = __am_timer_config,
        [AM_TIMER_RTC   ] = __am_timer_rtc,
        [AM_TIMER_UPTIME] = __am_timer_uptime,
        [AM_INPUT_CONFIG] = __am_input_config,
        [AM_INPUT_KEYBRD] = __am_input_keybrd,
        [AM_GPU_CONFIG  ] = __am_gpu_config,
        [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw,
        [AM_GPU_STATUS  ] = __am_gpu_status,
        [AM_UART_CONFIG ] = __am_uart_config,
        [AM_AUDIO_CONFIG] = __am_audio_config,
        [AM_AUDIO_CTRL  ] = __am_audio_ctrl,
        [AM_AUDIO_STATUS] = __am_audio_status,
        [AM_AUDIO_PLAY  ] = __am_audio_play,
};

static void fail(void *buf) { panic("access nonexist register"); }

bool ioe_init() {
//    printf("=========ioe_init\n");
    for (int i = 0; i < LENGTH(lut); i++) {
        if (!lut[i]) {
            lut[i] = fail;
        }
    }
    NDL_Init(1);
    __am_gpu_init();
    __am_timer_init();
    return true;
}

void ioe_read(int reg, void *buf) { ((handler_t) lut[reg])(buf); }

void ioe_write(int reg, void *buf) { ((handler_t) lut[reg])(buf); }
