#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>
#include <device/mmio.h>
#include <unistd.h>

enum {
    reg_freq = 44100,
    reg_channels = 1,
    reg_samples = 512,
    reg_sbuf_size = CONFIG_SB_ADDR,
    reg_init = 0,
    reg_count = 0,
    nr_reg = 8
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
    uint32_t start = audio_base[5];
    uint32_t end = audio_base[6];
    uint32_t sb_size = (uint32_t)CONFIG_SB_SIZE;
    uint32_t count;
    // 获取count的值时需要判断是否为空
    count = start < end ? end - start : sb_size - start + end;
    if (start == end && audio_base[7] == 1) {
        memset(stream, 0, len);
        return;
    }
    uint32_t nread = len;
    if (count < len) {
        nread = count;
    }
    // 从SB_BUF中读取数据，然后将其写入到音频流中
    // 涉及到正常内存之外的数据读取，不能直接使用地址访问，只能用mmio_read、mmio_write方法
    uint32_t b = 0;
    uint32_t point;
    while (b < nread) {
        // 如果读取的数据超出了边界，则需要从头开始读
        if (start + b < sb_size) {
            point = start + b;
        } else {
            point = start + b - sb_size;
        }
        *stream++ = mmio_read(CONFIG_SB_ADDR + point, 1);
        b++;
    }

    // 移动start坐标，当超过边界时需要从零开始
    if ((sb_size - start) > nread) {
        audio_base[5] += nread;
    } else {
        audio_base[5] = start + nread - sb_size;
    }

    // 防止白噪音
    if (len > nread) {
        memset(stream + nread, 0, len - nread);
    }

    // end在初始化一次后就没有再更新了，经过本次读取，如果start等于end，就认为数据空了
    if (audio_base[5] == audio_base[6]) {
        audio_base[7] = 1;
    } else {
        audio_base[7] = 0;
    }

//    int count = audio_base[5];
//    if (count == 0) {
//        return;
//    }
//    int nread = len;
//    if (count < len) {
//        nread = count;
//    }
//
//    // 从SB_BUF中读取数据，然后将其写入到音频流中
//    // 涉及到正常内存之外的数据读取，不能直接使用地址访问，只能用mmio_read、mmio_write方法
//    int b = 0;
//    while (b < nread) {
//        *stream++ = mmio_read(CONFIG_SB_ADDR + b, 1);
//        b++;
//    }
//
//    if (count > nread) {
//        int mmio_remain = 0;
//        while (mmio_remain < count - nread) {
//            uint32_t data = mmio_read(CONFIG_SB_ADDR + nread + mmio_remain, 4);
//            mmio_write(CONFIG_SB_ADDR + mmio_remain, 4, data);
//            mmio_remain += 4;
//        }
//    }
//    count -= nread;
//    audio_base[5] = count;
//
//    if (len > nread) {
//        memset(stream + nread, 0, len - nread);
//    }
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
    audio_base[5] = 0;
    audio_base[6] = 0;
    audio_base[7] = 1;
#ifdef CONFIG_HAS_PORT_IO
    add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
    add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif
    init_SDL();
    sbuf = (uint8_t *) new_space(CONFIG_SB_SIZE);
    add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
