#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
int  open( const  char  *pathname,  int  flags, mode_t mode);

uint32_t NDL_GetTicks() {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return (current_time.tv_sec * 1000) + (current_time.tv_usec / 1000);
}

int NDL_PollEvent(char *buf, int len) {
    int fp = open("/dev/events", 0, 0);
    return read(fp, buf,len);
}

/**
 * 打开一张(*w)*(*h)的画布，如果*w与*h均为0，则将全屏幕设置为画布，并将*w和*h分别设置为系统屏幕的大小
 *
 * @param w 宽度
 * @param h 高度
 */
void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  } else {
      int fbctl = 4;
      char buf[64];
      read(fbctl, buf, sizeof(buf) - 1);
      sscanf(buf, "%d %d", &screen_w, &screen_h);
      if (*w == 0 && *h == 0) {
          *w = screen_w;
          *h = screen_h;
      }
      assert(*w <= screen_w);
      assert(*h <= screen_h);
  }
}

/**
 * 向画布(x, y)坐标处绘制w*h的矩形图像，并将该绘制区域同步到屏幕上
 * 画像像素按行优先方式存储在pixels中，每个像素用32位整数以00RRGGBB的方式描述颜色
 *
 * @param pixels 画像像素
 * @param x x坐标
 * @param y y坐标
 * @param w 矩形图像宽度
 * @param h 矩形图像高度
 */
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
    int fdfb = 5;
    x = (screen_w - w) / 2;
    y = (screen_h - h) / 2;
    lseek(fdfb, y * screen_w + x, SEEK_SET);
    write(fdfb, pixels, h * screen_w + w);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
    printf("=======NDL_OpenAudio\n");
    assert(0);
}

void NDL_CloseAudio() {
    printf("=======NDL_CloseAudio\n");
    assert(0);
}

int NDL_PlayAudio(void *buf, int len) {
    printf("=======NDL_PlayAudio\n");
    assert(0);
  return 0;
}

int NDL_QueryAudio() {
    printf("=======NDL_QueryAudio\n");
    assert(0);
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
