#include <common.h>
#include <fs.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static int screen_w = 0, screen_h = 0;

static const char *keyname[256] __attribute__((used)) = {
        [AM_KEY_NONE] = "NONE",
        AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
    char *tmp = (char *) buf;
    int count = 0;
    while (*tmp != '\0' && len > 0) {
        putch(*tmp++);
        len--;
        count++;
    }
    return count;
}

typedef AM_INPUT_KEYBRD_T input_keyboard;

size_t events_read(void *buf, size_t offset, size_t len) {
    input_keyboard inputKeyboard;
    ioe_read(AM_INPUT_KEYBRD, &inputKeyboard);
//    printf("keycode: %d, keydown: %d\n", inputKeyboard.keycode, inputKeyboard.keydown);
    if (inputKeyboard.keycode == 0) {
        return 0;
    }
    memset(buf, '\0', len);
//    printf("=============%s %d %d\n", keyname[inputKeyboard.keycode], inputKeyboard.keydown, inputKeyboard.keycode);
    char *tmp = (char *)buf;
    sprintf(tmp, "%d %d", inputKeyboard.keydown, inputKeyboard.keycode);
//    memcpy(buf, tmp, strlen(tmp));
    return strlen(tmp);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    return sprintf((char *)buf, "%d %d", screen_w, screen_h);
}

int x, y;
void *pixels;
int w, h;
_Bool sync;
struct AM_GPU_FBDRAW_T {
    int x, y;
    void *pixels;
    int w, h;
    _Bool sync;
};

size_t fb_write(const void *buf, size_t offset, size_t len) {
    int x = offset % screen_w;
    int y = offset / screen_w;
    int w = len % screen_w;
    if (w == 0 && len != 0) {
        w = screen_w;
    }
    int h = len / screen_w;
//    printf("x: %d, y: %d, w: %d, h: %d\n", x, y ,w ,h);
    struct AM_GPU_FBDRAW_T gpuFbdraw = {
            .x = x,
            .y = y,
            .pixels = (uint32_t *)buf,
            .w = w,
            .h = h,
            .sync = 1
    };
//    gpu_fbdraw gpuFbdraw;
//    gpuFbdraw.x = offset % screen_w;
//    gpuFbdraw.y = offset / screen_w;
//    *(uint32_t *)gpuFbdraw.pixels = *(uint32_t *)buf;
//    gpuFbdraw.w = len % screen_w - gpuFbdraw.x;
//    gpuFbdraw.h = len / screen_w - gpuFbdraw.y;
//    gpuFbdraw.sync = 1;
    ioe_write(AM_GPU_FBDRAW, &gpuFbdraw);
    return gpuFbdraw.w * gpuFbdraw.h;
}

typedef AM_AUDIO_PLAY_T AUDIO_PLAY;
size_t sb_write(const void *buf, size_t offset, size_t len) {
    void *temp_buf = (void *)buf;
    AUDIO_PLAY audio_play;
    audio_play.buf.start = temp_buf;
    audio_play.buf.end = temp_buf + len;
    ioe_write(AM_AUDIO_PLAY, &audio_play);
    printf("=======sb_write\t start: %08x, end: %08x\n", audio_play.buf.start, audio_play.buf.end);
    return len;
}

typedef AM_AUDIO_STATUS_T AUDIO_STATUS;
size_t sbctl_read(void *buf, size_t offset, size_t len) {
    AUDIO_STATUS audio_status;
    ioe_read(AM_AUDIO_STATUS, &audio_status);
    int sb[1];
    sb[0] = audio_status.count;
    buf = (void *)sb;
    return 1;
}

typedef AM_AUDIO_CTRL_T AUDIO_CTRL;
size_t sbctl_write(const void *buf, size_t offset, size_t len) {
    uint32_t *temp_buf = (uint32_t *)buf;
    AUDIO_CTRL audioCtrl;
    audioCtrl.freq = temp_buf[0];
    audioCtrl.channels = temp_buf[1];
    audioCtrl.samples = temp_buf[2];
    ioe_write(AM_AUDIO_CTRL, &audioCtrl);
    printf("=======sbctl_write\t freq: %d, channels: %d, samples: %d\n", temp_buf[0], temp_buf[1], temp_buf[2]);
    return 3;
}

typedef long time_t;
typedef long suseconds_t;
struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
};

typedef AM_TIMER_UPTIME_T timer_uptime;
int gettimeofday(intptr_t tv, intptr_t tz) {
    timer_uptime uptime;
    ioe_read(AM_TIMER_UPTIME, &uptime);
    struct timeval *time_val = (struct timeval *) tv;
    time_val->tv_sec = uptime.us / 1000000;
    time_val->tv_usec = uptime.us % 1000000;
    return 0;
}

void init_device() {
    Log("Initializing devices...");
    ioe_init();
}

void set_screen_size(int width, int height) {
    screen_w = width;
    screen_h = height;
}
