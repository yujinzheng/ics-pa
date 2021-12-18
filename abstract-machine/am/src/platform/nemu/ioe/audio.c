#include <am.h>
#include <nemu.h>
#include <klib.h>
#include <unistd.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

/**
 * 音频写入函数，将音频数据写入到MMIO的流缓冲区中
 *
 * @param buf MMIO空间中的流缓冲区
 * @param len 流缓冲区的长度
 */
static void audio_write(uint8_t *buf, int len) {
    int buf_size = *(volatile uint32_t *)AUDIO_SBUF_SIZE_ADDR;
    int remain = len;
    while (remain > 0) {
        int count = *(volatile uint32_t *)AUDIO_COUNT_ADDR;
        int free = buf_size - count;
        int put_len = remain < free ? remain : free;
        if (put_len == 0) {
            continue;
        }
        uint8_t *start = (uint8_t *)(AUDIO_SBUF_ADDR + count);
        memcpy(start, buf + len - remain, put_len);
        remain -= put_len;
        *(volatile uint32_t *)AUDIO_COUNT_ADDR += put_len;
    }
}

void __am_audio_init() {
//    uint32_t freq = *(volatile uint32_t *)AUDIO_FREQ_ADDR;
//    uint32_t channels = *(volatile uint32_t *)AUDIO_CHANNELS_ADDR;
//    uint32_t samples = *(volatile uint32_t *)AUDIO_SAMPLES_ADDR;
//    SDL_AudioSpec s = {};
//    s.freq = ctrl->freq;
//    s.format = AUDIO_S16SYS;
//    s.channels = ctrl->channels;
//    s.samples = ctrl->samples;
//    s.callback = audio_play;
//    s.userdata = NULL;
//    int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
//    if (ret == 0) {
//        SDL_OpenAudio(&s, NULL);
//        SDL_PauseAudio(0);
//    }
//    int fds[2];
//    int ret = pipe2(fds, O_NONBLOCK);
//    assert(ret == 0);
//    rfd = fds[0];
//    wfd = fds[1];
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
    cfg->present = true;
    uint32_t buf_size = *(volatile uint32_t *)AUDIO_SBUF_SIZE_ADDR;
    cfg->bufsize = buf_size;
}

// 应该是在初始化的时候调用一次
// TODO 尚未找准时机
void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
//    uint16_t init = *(volatile uint16_t *)AUDIO_INIT_ADDR;
//
//    if (init == 1) {
//        __am_audio_init();
//    }
//    SDL_AudioSpec s = {};
//    s.freq = ctrl->freq;
//    s.format = AUDIO_S16SYS;
//    s.channels = ctrl->channels;
//    s.samples = ctrl->samples;
//    s.callback = audio_play;
//    s.userdata = NULL;
//
//    count = 0;
//    int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
//    if (ret == 0) {
//        SDL_OpenAudio(&s, NULL);
//        SDL_PauseAudio(0);
//    }
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
    uint32_t init = *(volatile uint32_t *)AUDIO_INIT_ADDR;
    if (init) {
        *(volatile uint32_t *)AUDIO_COUNT_ADDR = 0;
        __am_audio_init();
    }
    stat->count = *(volatile uint32_t *)AUDIO_COUNT_ADDR;
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
    int len = ctl->buf.end - ctl->buf.start;
    audio_write(ctl->buf.start, len);
}
