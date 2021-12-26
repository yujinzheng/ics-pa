#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static char trans_tab[16] = "0123456789abcdef";

/**
 * int类型的数字转字符串
 * @param result 转换结果
 * @param num 需要转换的数字
 * @param base 进制
 * @param u_flag 是否为无符号转换
 * @return
 */
char *my_itoa(char *result, int num, int base, int u_flag) {
    if (base > 16 || base < 2) {
        return NULL;
    }

    int p = 0;
    // 中间变量
    unsigned int unum;

    // 只有u_flag为0的十进制才需要考虑负数
    if (base == 10 && num < 0 && u_flag == 0) {
        unum = (unsigned int)-num;
    } else {
        unum = (unsigned int)num;
    }

    do {
        result[p++] = trans_tab[unum%(unsigned int)base];
        unum /= base;
    } while (unum);
    result[p] = '\0';

    if (base == 10 && num < 0 && u_flag == 0) {
        result[p++] = '-';
    }

    char temp;
    for (int idx = 0; idx < p - idx; idx++) {
        temp = result[idx];
        result[idx] = result[p - idx - 1];
        result[p - idx - 1] = temp;
    }
    return result;
}

/**
 * long类型的数字转字符串
 * @param result 转换结果
 * @param num 需要转换的数字
 * @param base 进制
 * @param u_flag 是否为无符号转换
 * @return
 */
char *my_itoa_l(char *result, long num, int base, int u_flag) {
    if (base > 16 || base < 2) {
        return NULL;
    }

    int p = 0;
    // 中间变量
    unsigned long unum;

    // 只有u_flag为0的十进制才需要考虑负数
    if (base == 10 && num < 0 && u_flag == 0) {
        unum = (unsigned long)-num;
    } else {
        unum = (unsigned long)num;
    }

    do {
        result[p++] = trans_tab[unum%(unsigned int)base];
        unum /= base;
    } while (unum);
    result[p] = '\0';

    if (base == 10 && num < 0 && u_flag == 0) {
        result[p++] = '-';
    }

    char temp;
    for (int idx = 0; idx < p - idx; idx++) {
        temp = result[idx];
        result[idx] = result[p - idx - 1];
        result[p - idx - 1] = temp;
    }
    return result;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    char *out_p;
    unsigned int count = 0;
    if (out != NULL && *out != '\0') {
        memset(out, '\0', strlen(out));
    }
    for (out_p = out; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            if (out == NULL) {
                putch(*fmt);
            } else {
                *out_p++ = *fmt;
            }
            count++;
            continue;
        }
        fmt++;
        unsigned int width = 0;

        // %%，当做一个普通的%处理
        if (*fmt == '%') {
            if (out == NULL) {
                putch(*fmt);
            } else {
                *out_p++ = *fmt;
            }
            count++;
            continue;
        }

        // 检查在%之后是否有长度数据，最多支持长度数据最大位99
        while (*fmt >= '0' && *fmt <= '9') {
            if (width < 100) {
                width = width * 10 + *fmt - '0';
            }
            fmt++;
        }

        //
        int is_long = 0;
        if (*fmt == 'l') {
            is_long = 1;
            fmt++;
        }

        int base = -1;
        int u_flag = 0;
        char *ss;
        unsigned int buf_len = 0;

        switch (*fmt) {
            case 'u':
                base = 10;
                u_flag = 1;
                break;
            case 'd':
                base = 10;
                u_flag = 0;
                break;
            case 'x':
                base = 16;
                break;
            case 'p':
                base = 16;
                if (out == NULL) {
                    putch('0');
                    putch('x');
                } else {
                    *out_p++ = '0';
                    *out_p++ = 'x';
                }
                break;
            case 'o':
                base = 8;
                break;
            case 'b':
                base = 2;
                break;
            case 's':
                ss = va_arg(ap, char *);
                buf_len = strlen(ss);
                while (width > buf_len) {
                    if (out == NULL) {
                        putch(' ');
                    } else {
                        *out_p++ = ' ';
                    }
                    count++;
                    width--;
                }
                while (*ss != '\0') {
                    if (out == NULL) {
                        putch(*ss++);
                    } else {
                        *out_p++ = *ss++;
                    }
                    count++;
                }
                break;
            default:
                break;
        }
        if (base > 0) {
            char tmp_buf[256];
            memset(tmp_buf, '\0', sizeof(tmp_buf));
            if (is_long) {
                long dd = 0;
                dd = (long)va_arg(ap, long);
                if (my_itoa_l(tmp_buf, dd, base, u_flag) == NULL) {
                    return -1;
                }
            } else {
                int dd = 0;
                dd = (int)va_arg(ap, int);
                if (my_itoa(tmp_buf, dd, base, u_flag) == NULL) {
                    return -1;
                }
            }
            buf_len = strlen(tmp_buf);
            if (out == NULL) {
                while (buf_len < width) {
                    putch('0');
                    count++;
                    width--;
                }
                for (int idx = 0; idx < buf_len; idx++) {
                    putch(tmp_buf[idx]);
                    count++;
                }
            } else {
                if (buf_len < width) {
                    memset(out_p, '0', width - buf_len);
                    out_p = out_p + width - buf_len;
                    memcpy(out_p, tmp_buf, buf_len);
                    out_p = out_p + buf_len;
                    count += width;
                } else {
                    memcpy(out_p, tmp_buf, buf_len);
                    out_p = out_p + buf_len;
                    count += buf_len;
                }
            }
        }
    }
    return count;
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int result = vsprintf(NULL, fmt, ap);
    va_end(ap);
    return result;
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int result = vsprintf(out, fmt, ap);
    va_end(ap);
    return result;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    panic("Not implemented");
}

#endif
