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
//uint64_t last_play_time;

/**
 * 音频的回调函数
 *
 * @param userdata 没有使用，回调函数固定值
 * @param stream 即将用于填充音频数据的流的指针
 * @param len 音频流指针的长度（以bytes计算）
 */
static void audio_play(void *userdata, uint8_t *stream, int len) {
//    if (get_time() - last_play_time < 40000) {
//        memset(stream, 0, len);
//        return;
//    }
    volatile uint32_t start = audio_base[5];
    volatile uint32_t end = audio_base[6];
    uint32_t *tmp_stream = (uint32_t *)stream;
    uint32_t sb_size = audio_base[3];
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
    uint32_t point = start;
    while (b < nread) {
        if (point + 4 < sb_size) {
            *tmp_stream++ = mmio_read(CONFIG_SB_ADDR + point, 4);
            stream += 4;
            point += 4;
            b += 4;
        } else {
            uint32_t need_len = 4;
            while (need_len > 0) {
                *stream++ = mmio_read(CONFIG_SB_ADDR + point, 1);
                point++;
                b++;
                need_len--;
                if (point == sb_size) {
                    point = 0;
                }
            }
            tmp_stream++;
        }
    }

    // 移动start坐标，当超过边界时需要从零开始
    if ((start + nread) < sb_size) {
        start += nread;
    } else {
        start = start + nread - sb_size;
    }

    // 防止白噪音
    if (len > nread) {
        memset(stream + nread, 0, len - nread);
    }

    // end在初始化一次后就没有再更新了，经过本次读取，如果start等于end，就认为数据空了
    if (start == end) {
        audio_base[7] = 1;
    } else {
        audio_base[7] = 0;
    }
    audio_base[5] = start;
}

static void init_SDL() {
    audio_base[3] = CONFIG_SB_SIZE;
    SDL_AudioSpec s = {};
    s.freq = (int)audio_base[0];
    s.format = AUDIO_S16SYS;
    s.channels = audio_base[1];
    s.samples = audio_base[2];
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
    if (is_write) {
        if (offset == 16 && audio_base[4] == 1) {
            init_SDL();
        }
    }
}

void init_audio() {
    uint32_t space_size = sizeof(uint32_t) * nr_reg;
    audio_base = (uint32_t *) new_space(space_size);
    audio_base[5] = 0;
    audio_base[6] = 0;
    audio_base[7] = 1;
#ifdef CONFIG_HAS_PORT_IO
    add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
    add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif
//    last_play_time = get_time();
    sbuf = (uint8_t *) new_space(CONFIG_SB_SIZE);
    add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
