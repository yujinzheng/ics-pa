#include <am.h>
#include <nemu.h>
#include <klib.h>
#include <unistd.h>

#define AUDIO_FREQ_ADDR       (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR   (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR    (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR  (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR       (AUDIO_ADDR + 0x10)
#define AUDIO_START_ADDR      (AUDIO_ADDR + 0x14)
#define AUDIO_END_ADDR        (AUDIO_ADDR + 0x18)
#define AUDIO_SBUF_EMPTY_ADDR (AUDIO_ADDR + 0x1C)

/**
 * 音频写入函数，将音频数据写入到MMIO的流缓冲区中
 *
 * @param buf MMIO空间中的流缓冲区
 * @param len 流缓冲区的长度
 */
static void audio_write(uint8_t *buf, int len) {
    if (len == 0) {
        return;
    }

    uint32_t buf_size = *(uint32_t *)AUDIO_SBUF_SIZE_ADDR;
    uint32_t start;
    uint32_t end;
    uint32_t sbuf_empty;

    uint32_t remain = len;
    while (remain > 0) {
        start = *(volatile uint32_t *)AUDIO_START_ADDR;
        end = *(volatile uint32_t *)AUDIO_END_ADDR;
        sbuf_empty = *(uint32_t *)AUDIO_SBUF_EMPTY_ADDR;
        // 先排除满数据的情况
        if (start == end && sbuf_empty == 0) {
            continue;
        }
        // 当start < end，或者sbuf为空时，获取数据的顺序都是正常的
        if (start < end || (start == end && sbuf_empty == 1)) {
            // 如果end到末尾的距离比剩余的小，那么先让数据一直填充到末尾
            if (buf_size - end < remain) {
                if (end == buf_size) {
                    *(volatile uint32_t *)AUDIO_END_ADDR = 0;
                    *(volatile uint32_t *) AUDIO_SBUF_EMPTY_ADDR = 0;
                    continue;
                }
                memcpy((uint8_t * )(AUDIO_SBUF_ADDR + end), buf + len - remain, buf_size - end);
                remain = remain - (buf_size - end);
                *(volatile uint32_t *) AUDIO_END_ADDR = 0;
                *(volatile uint32_t *) AUDIO_SBUF_EMPTY_ADDR = 0;
                continue;
            }
        } else {
            // 这个逻辑分支处理的是start > end，空闲空间从end到start
            if (start - end < remain) {
                memcpy((uint8_t * )(AUDIO_SBUF_ADDR + end), buf + len - remain, start - end);
                remain = remain - (start - end);
                *(volatile uint32_t *) AUDIO_END_ADDR = start;
                *(volatile uint32_t *) AUDIO_SBUF_EMPTY_ADDR = 0;
                continue;
            }
        }
        // 之前都是判断剩余空间小于remain，这里剩余空间大于remain，所以处理逻辑归一了
        // 值得注意的是，这里为啥没有考虑超出上界的问题呢？因为只有start < end的场景可能超出上界
        // 在第一个逻辑分支中已经处理过了
        memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf + len - remain, remain);
        end += remain;
        *(volatile uint32_t *) AUDIO_END_ADDR = end;
        *(volatile uint32_t *) AUDIO_SBUF_EMPTY_ADDR = 0;
        break;
    }



//        if (buf_size - end < remain) {
//            memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf, buf_size - end);
//            remain = remain - (buf_size - end);
//            end = 0;
//        } else {
//            if (sb_left < remain) {
//                memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf, len);
//            }
//
//            *(volatile uint32_t *)AUDIO_ENT_ADDR += len;
//        }
//    }
//
//    sb_left = start < end ? buf_size - (end - start) - 1: buf_size - end + start - 1;
//
//    if (len < sb_left) {
//        if (buf_size - end < len) {
//            memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf, buf_size - end);
//            memcpy((uint8_t *)(AUDIO_SBUF_ADDR), buf + buf_size - end, len - (buf_size - end));
//            *(volatile uint32_t *)AUDIO_ENT_ADDR = len - (buf_size - end);
//        } else {
//            memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf, len);
//            *(volatile uint32_t *)AUDIO_ENT_ADDR += len;
//        }
//    } else {
//        int remain = len;
//        while (remain > 0) {
//            sb_left = start < end ? buf_size - (end - start) - 1: buf_size - end + start - 1;
//            if (sb_left == 0) {
//                continue;
//            }
//            memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf + len - remain, count);
//            start = *(volatile uint32_t *)AUDIO_START_ADDR;
//            end = end + count < buf_size ? end + count : count - (buf_size - end);
//        }
//    }
//
//
//
//
//    if (end < start) {
//        if (len < start - end) {
//            memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf, len);
//            *(volatile uint32_t *)AUDIO_ENT_ADDR += len;
//        } else {
//            int remain = len;
//            while (remain > 0) {
//                int count;
//                count = start < end ? : end - start - 1 : end + buf_size - start - 1;
//                if (count == 0) {
//                    continue;
//                }
//                memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf + len - remain, count);
//                start = *(volatile uint32_t *)AUDIO_START_ADDR;
//                end = end + count < buf_size ? end + count : count - (buf_size - end);
//            }
//        }
//    } else {
//        if (len < end - start) {
//            memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf, len);
//            *(volatile uint32_t *)AUDIO_START_ADDR += len;
//        } else {
//            int remain = len;
//            while (remain > 0) {
//                int count;
//                count = start < end ? : end - start - 1 : end + buf_size - start - 1;
//                if (count == 0) {
//                    continue;
//                }
//                memcpy((uint8_t *)(AUDIO_SBUF_ADDR + start), buf + len - remain, count);
//                start = *(volatile uint32_t *)AUDIO_START_ADDR;
//                end = end + count < buf_size ? end + count : count - (buf_size - end);
//            }
//        }
//    }


//    int buf_size = *(volatile uint32_t *)AUDIO_SBUF_SIZE_ADDR;
//    int remain = len;
//    while (remain > 0) {
//        int count = *(volatile uint32_t *)AUDIO_COUNT_ADDR;
//        int free = buf_size - count;
//        int put_len = remain < free ? remain : free;
//        if (put_len == 0) {
//            continue;
//        }
//        uint8_t *start = (uint8_t *)(AUDIO_SBUF_ADDR + count);
//        memcpy(start, buf + len - remain, put_len);
//        remain -= put_len;
//        *(volatile uint32_t *)AUDIO_COUNT_ADDR += put_len;
//    }
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
        *(volatile uint32_t *)AUDIO_START_ADDR = 0;
        *(volatile uint32_t *)AUDIO_END_ADDR = 0;
        __am_audio_init();
    }
    int start = *(volatile uint32_t *)AUDIO_START_ADDR;
    int end = *(volatile uint32_t *)AUDIO_END_ADDR;
    int sbuf_empty = *(volatile uint32_t *)AUDIO_SBUF_EMPTY_ADDR;
    uint32_t buf_size = *(volatile uint32_t *)AUDIO_SBUF_SIZE_ADDR;
    if (sbuf_empty == 0) {
        stat->count = start < end ? end - start : buf_size - start + end;
    } else {
        stat->count = start <= end ? end - start : buf_size - start + end;
    }
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
    int len = ctl->buf.end - ctl->buf.start;
    audio_write(ctl->buf.start, len);
}
