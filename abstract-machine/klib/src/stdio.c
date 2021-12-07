#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static char trans_tab[16] = "0123456789abcdef";

// 数字转字符串，最大只支持16进制的转换
int my_itoa(char *result, int num, int base) {
    char tmp[36];
    if (base > 16) {
        return 1;
    }
    int p = 0;
    int tmp_num = num < 0 ? 0 - num : num;
    while (tmp_num > 0) {
        tmp[p++] = trans_tab[tmp_num % base];
        tmp_num = (tmp_num - tmp_num % base) / base;
    }
    if (num < 0) {
        tmp[p++] = '-';
    }
    for (int idx = 0; idx < p; idx++) {
        result[idx] = tmp[p- 1 - idx];
    }
    result[p] = '\0';
    return 0;
}

int parse_num(char *buf, int dd, int base) {
    char tmp[36];

    if (my_itoa(tmp, dd, base) != 0) {
        return 1;
    }

    int p = 0;
    while (tmp[p] != '\0') {
        buf[p] = tmp[p];
        p++;
    }
    buf[p] = '\0';
    return 0;
}

int printf(const char *fmt, ...) {
    panic("Not implemented");
//    val_list ap;
//    int ret = -1;
//    va_start(ap, fmt);
//    ret = vprintf(fmt, ap);
//    va_end(ap);
//    return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    panic("Not implemented");
//    char *p;
//    char tmp[256];
//    val_list p_next_args = ap;
//
//    for (p = out; *fmt; fmt++) {
//        if (*fmt != '%') {
//            *p++ = *fmt;
//            continue;
//        }
//        fmt++;
//
//        switch (*fmt) {
//            case 'x':
//                itoa(tmp, *((int *)p_next_arg));
//                strcpy(p, tmp);
//                p_next_args += 4;
//                p += strlen(tmp);
//                break;
//            case 's':
//                break;
//            default:
//                break;
//        }
//    }
//    return (p - out);
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    char *out_p;
    char tmp_buf[256];
    if (*out != '\0') {
        memset(out, '\0', strlen(out));
    }
    memset(tmp_buf, '\0', sizeof(tmp_buf));
    va_start(ap, fmt);
    for (out_p = out; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            *out_p++ = *fmt;
            continue;
        }
        fmt++;
        unsigned int width = 0;

        // 检查在%之后是否有长度数据，最多支持长度数据最大位99
        while (*fmt >= '0' && *fmt <= '9') {
            if (width < 100) {
                width = width * 10 + *fmt - '0';
            }
            fmt++;
        }

        int dd;
        char *ss;
        int base = -1;
        unsigned int buf_len = 0;
        switch (*fmt) {
            // %%，当做一个普通的%处理
            case '%':
                *out_p++ = *fmt;
                break;
            case 'd':
                base = 10;
            case 'x':
                if (base < 0) {
                    base = 16;
                }
                dd = (int)va_arg(ap, int);
                if (parse_num(tmp_buf, dd, base) != 0) {
                    return -1;
                }
                buf_len = strlen(tmp_buf);
                if (buf_len < width) {
                    memset(out_p, '0', width - buf_len);
                    out_p = out_p + width - buf_len;
                    memcpy(out_p, tmp_buf, buf_len);
                    out_p = out_p + buf_len;
                } else {
                    memcpy(out_p, tmp_buf, buf_len);
                    out_p = out_p + buf_len;
                }
                break;
            case 's':
                ss = va_arg(ap, char *);
                buf_len = strlen(ss);
                while (width > buf_len) {
                    *out_p++ = ' ';
                    width--;
                }
                while (*ss != '\0') {
                    *out_p++ = *ss++;
                }
                break;
            default:
                break;
        }
    }
    va_end(ap);
    return (out_p - out);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    panic("Not implemented");
}

#endif
