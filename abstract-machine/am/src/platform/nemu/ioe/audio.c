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
    volatile uint32_t start;
    volatile uint32_t end;
    volatile uint32_t sbuf_empty;

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
                memcpy((uint8_t * )(AUDIO_SBUF_ADDR + end), buf + len - remain, buf_size - end);
                remain = remain - (buf_size - end);
                *(volatile uint32_t *) AUDIO_END_ADDR = 0;
                *(volatile uint32_t *) AUDIO_SBUF_EMPTY_ADDR = 0;
//                printf("111+++++start: %u, end: %u, remain: %u\n", start, end, remain);
                continue;
            }
        } else {
            // 这个逻辑分支处理的是start > end，空闲空间从end到start
            if (start - end < remain) {
                memcpy((uint8_t * )(AUDIO_SBUF_ADDR + end), buf + len - remain, start - end);
                remain = remain - (start - end);
                *(volatile uint32_t *) AUDIO_END_ADDR = start;
                *(volatile uint32_t *) AUDIO_SBUF_EMPTY_ADDR = 0;
//                printf("222+++++start: %u, end: %u, remain: %u\n", start, end, remain);
                continue;
            }
        }
        // 之前都是判断剩余空间小于remain，这里剩余空间大于remain，所以处理逻辑归一了
        // 值得注意的是，这里为啥没有考虑超出上界的问题呢？因为只有start < end的场景可能超出上界
        // 在第一个逻辑分支中已经处理过了
        memcpy((uint8_t *)(AUDIO_SBUF_ADDR + end), buf + len - remain, remain);
        end += remain;
        if (end == buf_size) {
            end = 0;
        }
        *(volatile uint32_t *) AUDIO_END_ADDR = end;
        *(volatile uint32_t *) AUDIO_SBUF_EMPTY_ADDR = 0;
//        printf("333+++++start: %u, end: %u, remain: %u\n", start, end, remain);
        break;
    }
}

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
    cfg->present = true;
    cfg->bufsize = *(uint32_t *)AUDIO_SBUF_SIZE_ADDR;
}

// 在开始播放之前，程序需要先注册配置信息
void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
    *(volatile uint32_t *)AUDIO_FREQ_ADDR = ctrl->freq;
    *(volatile uint32_t *)AUDIO_CHANNELS_ADDR = ctrl->channels;
    *(volatile uint32_t *)AUDIO_SAMPLES_ADDR = ctrl->samples;
    *(volatile uint32_t *)AUDIO_INIT_ADDR = 1;
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
    volatile uint32_t init = *(volatile uint32_t *)AUDIO_INIT_ADDR;
    if (init) {
        *(volatile uint32_t *)AUDIO_START_ADDR = 0;
        *(volatile uint32_t *)AUDIO_END_ADDR = 0;
        __am_audio_init();
    }
    volatile int start = *(volatile uint32_t *)AUDIO_START_ADDR;
    volatile int end = *(volatile uint32_t *)AUDIO_END_ADDR;
    volatile int sbuf_empty = *(volatile uint32_t *)AUDIO_SBUF_EMPTY_ADDR;
    uint32_t buf_size = *(uint32_t *)AUDIO_SBUF_SIZE_ADDR;
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
