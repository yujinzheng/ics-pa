#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>
#include <device/mmio.h>
#include <unistd.h>

enum {
    reg_freq = 16000,
    reg_channels = 1,
    reg_samples = 1024,
    reg_sbuf_size = CONFIG_SB_ADDR,
    reg_init = 0,
    reg_count = 0,
    nr_reg = 6
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

/**
 * 音频的回调函数
 *
 * @param userdata 没有使用，回调函数固定值
 * @param stream 即将用于填充音频数据的流的指针
 * @param len 音频流指针的长度（以bytes计算）
 */
static void audio_play(void *userdata, uint8_t *stream, int len) {
    int count = audio_base[5];
    int nread = len;
    if (count < len) {
        nread = count;
    }

    int b = 0;
    while (b < nread) {
        *stream++ = mmio_read(CONFIG_SB_ADDR + b, 1);
        b++;
    }

    if (count > nread) {
        int mmio_remain = 0;
        while (mmio_remain < count - nread) {
            uint8_t data = mmio_read(CONFIG_SB_ADDR + nread + mmio_remain, 1);
            mmio_write(CONFIG_SB_ADDR + mmio_remain, 1, data);
            mmio_remain++;
        }
    }
    count -= nread;
    audio_base[5] = count;

    if (len > nread) {
        memset(stream + nread, 0, len - nread);
    }
}

static void init_SDL() {
    SDL_AudioSpec s = {};
    s.freq = reg_freq;
    s.format = AUDIO_S16SYS;
    s.channels = reg_channels;
    s.samples = reg_samples;
    s.callback = audio_play;
    s.userdata = NULL;
    audio_base[5] = reg_count;
    int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (ret == 0) {
        SDL_OpenAudio(&s, NULL);
        SDL_PauseAudio(0);
        audio_base[4] = reg_init;
    }
}

// CPU访问了mmio映射的时候提供的内存地址，也就是访问了划分给Audio的内存段
static void audio_io_handler(uint32_t offset, int len, bool is_write) {
}

void init_audio() {
    uint32_t space_size = sizeof(uint32_t) * nr_reg;
    audio_base = (uint32_t *) new_space(space_size);
    audio_base[0] = reg_freq;
    audio_base[1] = reg_channels;
    audio_base[2] = reg_samples;
    audio_base[3] = CONFIG_SB_SIZE;
    audio_base[4] = reg_init;
#ifdef CONFIG_HAS_PORT_IO
    add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
    add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif
    init_SDL();
    sbuf = (uint8_t *) new_space(CONFIG_SB_SIZE);
    add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
