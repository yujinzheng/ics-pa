#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

void CallbackHelper();
/**
 * 快速拷贝一个表面内容到目标表面
 *
 * @param src 源面
 * @param srcrect 源面上的矩形区域，如果为NULL，则代表整个源面都要被拷贝
 * @param dst 目标面
 * @param dstrect 虽然是一个矩形区域指针，但是实际上只用到了左上角的坐标，如果为NULL，则左上角为(0, 0)
 */
void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
    assert(dst && src);
    assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);

    CallbackHelper();
    // 源面获取的是一个矩形区域，因此四个坐标都要获取
    int src_rect_x;
    int src_rect_y;
    int src_rect_h;
    int src_rect_w;
    if (srcrect == NULL) {
        src_rect_x = 0;
        src_rect_y = 0;
        src_rect_w = src->w;
        src_rect_h = src->h;
    } else {
        src_rect_x = srcrect->x;
        src_rect_y = srcrect->y;
        src_rect_w = srcrect->w;
        src_rect_h = srcrect->h;
    }

    // 目标地面只需要考虑左上角的坐标
    int dst_rect_x;
    int dst_rect_y;
    if (dstrect == NULL) {
        dst_rect_x = 0;
        dst_rect_y = 0;
    } else {
        dst_rect_x = dstrect->x;
        dst_rect_y = dstrect->y;
    }

    // 考虑到可能会有溢出，所以先判断获取一下最小的宽度和高度
    int actual_w = src_rect_x + src_rect_w < src->w ? src_rect_w : src->w - src_rect_x;
    int actual_h = src_rect_y + src_rect_h < src->h ? src_rect_h : src->h - src_rect_y;
    actual_w = dst_rect_x + actual_w < dst->w ? actual_w : dst->w - dst_rect_x;
    actual_h = dst_rect_y + actual_h < dst->h ? actual_h : dst->h - dst_rect_y;
    for (int idx = 0; idx < actual_h; idx++) {
        if (dst->format->BitsPerPixel >= 24) {
            memcpy(&dst->pixels[(dst_rect_y + idx) * dst->pitch + dst_rect_x * sizeof(uint32_t)],
                   &src->pixels[(src_rect_y + idx) * src->pitch + src_rect_x * sizeof(uint32_t)],
                   actual_w * sizeof(uint32_t));
        } else {
            // 只要不从调色盘中取值，那么pixels的索引就相对没有变化，只是需要注意不用再乘以sizeof(uint32_t)了
            memcpy(&dst->pixels[(dst_rect_y + idx) * dst->pitch + dst_rect_x],
                   &src->pixels[(src_rect_y + idx) * src->pitch + src_rect_x], actual_w);
        }
    }
}

void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {
    CallbackHelper();
    if (dstrect == NULL) {
        memset(dst->pixels, color, (dst->h * dst->pitch));
        return;
    }
    int x = dstrect->x;
    int y = dstrect->y;
    int dst_w = dst->w;
    int dst_h = dst->h;
    if (x >= dst_w || y >= dst_h) {
        return;
    }
    int w = x + dstrect->w > dst_w ? dst_w - x : dstrect->w;
    int h = y + dstrect->h > dst_h ? dst_h - y : dstrect->h;
    if (dst->format->BitsPerPixel >= 24) {
        for (int idx = 0; idx < h; idx++) {
            memset(&(dst->pixels[(y + idx) * dst->pitch + x * sizeof(uint32_t)]), color, w * sizeof(uint32_t));
        }
    } else {
        for (int idx = 0; idx < h; idx++) {
//            printf("SDL_FillRect: x: %d, y: %d, h: %d\n", x, y, h);
            memset(&(dst->pixels[(y + idx) * dst->pitch + x]), color, w);
        }
    }
}

/**
 * 根据给定的矩形区域，更新屏幕，如果x，y，w，h均为0，则更新整个屏幕
 *
 * @param s 屏幕信息
 * @param x 矩形左上角x坐标
 * @param y 矩形左上角y坐标
 * @param w 矩形宽度
 * @param h 矩形高度
 */
void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {
    CallbackHelper();
//    printf("111SDL_UpdateRect: x: %d, y: %d, w: %d, h: %d\n", x, y, w, h);
    if (x == 0 & y == 0 & w == 0 & h == 0) {
        w = s->w;
        h = s->h;
    }
    if (s->format->BitsPerPixel >= 24) {
        NDL_DrawRect((uint32_t *) s->pixels, x, y, w, h);
    } else {
        w = x + w < s->w ? w : s->w - x;
        h = y + h < s->h ? h : s->h - y;
        SDL_PixelFormat *format = s->format;
        SDL_Palette *palette = format->palette;
        SDL_Color *colors = palette->colors;
        int pixels_num = w * h;
        uint32_t *actual_pixels = (uint32_t *) malloc(pixels_num * sizeof(uint32_t));
//        printf("222SDL_UpdateRect: x: %d, y: %d, w: %d, h: %d, pitch: %d, pixels_num: %d\n", x, y, w, h, s->pitch,
//               pixels_num);
        uint8_t pixel;
        SDL_Color *color;
        for (int ydx = 0; ydx < h; ydx++) {
            for (int xdx = 0; xdx < w; xdx++) {
                pixel = (uint8_t) s->pixels[(y + ydx) * s->pitch + x + xdx];
                color = &format->palette->colors[pixel];
                uint32_t r = (uint32_t) color->r;
                uint32_t g = (uint32_t) color->g;
                uint32_t b = (uint32_t) color->b;
                uint32_t actual_color = (r << 16) | (g << 8) | b & 0xffffff;
                memcpy(&actual_pixels[ydx * w + xdx], &actual_color, sizeof(uint32_t));
//                if (r != 0 & g != 0 & b != 0) {
//                    printf("r: %08x, g: %08x, b: %08x, pixel: %08x, pixels: %08x, color: %08x, actual_color: %08x\n", r,
//                           g, b, pixel, actual_pixels[ydx * w + xdx], *color, actual_color);
//                }
//            printf("actual_pixels[%d]: %08x, colors[%d]: %08x, s->pixels[idx]: %d\n", idx, actual_pixels[idx], color_idx, colors[color_idx], s->pixels[idx]);
            }
        }
//        printf("333SDL_UpdateRect: x: %d, y: %d, w: %d, h: %d, pixels_num: %d\n", x, y, w, h, pixels_num);
        NDL_DrawRect(actual_pixels, x, y, w, h);
        free(actual_pixels);
    }
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask) {
    switch (mask) {
        case 0x000000ff:
            return 0;
        case 0x0000ff00:
            return 8;
        case 0x00ff0000:
            return 16;
        case 0xff000000:
            return 24;
        case 0x00000000:
            return 24; // hack
        default:
            assert(0);
    }
}

SDL_Surface *SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
                                  uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
    assert(depth == 8 || depth == 32);
    SDL_Surface *s = malloc(sizeof(SDL_Surface));
    assert(s);
    s->flags = flags;
    s->format = malloc(sizeof(SDL_PixelFormat));
    assert(s->format);
    if (depth == 8) {
        s->format->palette = malloc(sizeof(SDL_Palette));
        assert(s->format->palette);
        s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
        assert(s->format->palette->colors);
        memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
        s->format->palette->ncolors = 256;
    } else {
        s->format->palette = NULL;
        s->format->Rmask = Rmask;
        s->format->Rshift = maskToShift(Rmask);
        s->format->Rloss = 0;
        s->format->Gmask = Gmask;
        s->format->Gshift = maskToShift(Gmask);
        s->format->Gloss = 0;
        s->format->Bmask = Bmask;
        s->format->Bshift = maskToShift(Bmask);
        s->format->Bloss = 0;
        s->format->Amask = Amask;
        s->format->Ashift = maskToShift(Amask);
        s->format->Aloss = 0;
    }

    s->format->BitsPerPixel = depth;
    s->format->BytesPerPixel = depth / 8;

    s->w = width;
    s->h = height;
    s->pitch = width * depth / 8;
    assert(s->pitch == width * s->format->BytesPerPixel);

    if (!(flags & SDL_PREALLOC)) {
        s->pixels = malloc(s->pitch * height);
        assert(s->pixels);
    }

    return s;
}

SDL_Surface *SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
                                      int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
    SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
                                          Rmask, Gmask, Bmask, Amask);
    assert(pitch == s->pitch);
    s->pixels = pixels;
    return s;
}

void SDL_FreeSurface(SDL_Surface *s) {
    if (s != NULL) {
        if (s->format != NULL) {
            if (s->format->palette != NULL) {
                if (s->format->palette->colors != NULL) free(s->format->palette->colors);
                free(s->format->palette);
            }
            free(s->format);
        }
        if (s->pixels != NULL && !(s->flags & SDL_PREALLOC)) free(s->pixels);
        free(s);
    }
}

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags) {
    if (flags & SDL_HWSURFACE) NDL_OpenCanvas(&width, &height);
    return SDL_CreateRGBSurface(flags, width, height, bpp,
                                DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
    assert(src && dst);
    assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
    assert(dst->format->BitsPerPixel == 8);

    int x = (srcrect == NULL ? 0 : srcrect->x);
    int y = (srcrect == NULL ? 0 : srcrect->y);
    int w = (srcrect == NULL ? src->w : srcrect->w);
    int h = (srcrect == NULL ? src->h : srcrect->h);

//    printf("x: %d, y:%d, w:%d, h: %d, dstrect->w: %d, dstrect->h: %d\n", x, y, w, h, dstrect->w, dstrect->h);

    assert(dstrect);
    if (w == dstrect->w && h == dstrect->h) {
        /* The source rectangle and the destination rectangle
         * are of the same size. If that is the case, there
         * is no need to stretch, just copy. */
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        SDL_BlitSurface(src, &rect, dst, dstrect);
    } else {
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.w = (w < dstrect->w ? w : dstrect->w);
        rect.h = (h < dstrect->h ? h : dstrect->h);
        SDL_Rect dect = rect;
        SDL_BlitSurface(src, &rect, dst, &dect);
    }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
    assert(s);
    assert(s->format);
    assert(s->format->palette);
    assert(firstcolor == 0);

    s->format->palette->ncolors = ncolors;
    memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

    if (s->flags & SDL_HWSURFACE) {
        assert(ncolors == 256);
        for (int i = 0; i < ncolors; i++) {
            uint8_t r = colors[i].r;
            uint8_t g = colors[i].g;
            uint8_t b = colors[i].b;
        }
        SDL_UpdateRect(s, 0, 0, 0, 0);
    }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {
    int i;
    uint8_t(*pdst)[4] = dst;
    uint8_t(*psrc)[4] = src;
    union {
        uint8_t val8[4];
        uint32_t val32;
    } tmp;
    int first = len & ~0xf;
    for (i = 0; i < first; i += 16) {
#define macro(i) \
    tmp.val32 = *((uint32_t *)psrc[i]); \
    *((uint32_t *)pdst[i]) = tmp.val32; \
    pdst[i][0] = tmp.val8[2]; \
    pdst[i][2] = tmp.val8[0];

        macro(i + 0);
        macro(i + 1);
        macro(i + 2);
        macro(i + 3);
        macro(i + 4);
        macro(i + 5);
        macro(i + 6);
        macro(i + 7);
        macro(i + 8);
        macro(i + 9);
        macro(i + 10);
        macro(i + 11);
        macro(i + 12);
        macro(i + 13);
        macro(i + 14);
        macro(i + 15);
    }

    for (; i < len; i++) {
        macro(i);
    }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags) {
    assert(src->format->BitsPerPixel == 32);
    assert(src->w * src->format->BytesPerPixel == src->pitch);
    assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

    SDL_Surface *ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
                                            fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

    assert(fmt->Gmask == src->format->Gmask);
    assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
    ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

    return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    assert(fmt->BytesPerPixel == 4);
    uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
    if (fmt->Amask) p |= (a << fmt->Ashift);
    return p;
}

int SDL_LockSurface(SDL_Surface *s) {
    printf("=======SDL_LockSurface\n");
    assert(0);
    return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
    printf("=======SDL_UnlockSurface\n");
    assert(0);
}
