#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    assert(NULL != s);
    int i = 0;
    while (*s != '\0') {
        s++;
        i++;
    }
    return i;
}

char *strcpy(char *dst, const char *src) {
    assert(NULL != dst);
    assert(NULL != src);
    char *p = dst;
    while ((*dst++ = *src++) != '\0');
    return p;
}

char *strncpy(char *dst, const char *src, size_t n) {
    assert(NULL != dst);
    assert(NULL != src);
    size_t size = strlen(src) < n ? strlen(src) : n;
    if (size != n) {
        memset(dst + size, '\0', n - size);
    }
    return memcpy(dst, src, size);
}

char *strcat(char *dst, const char *src) {
    assert(NULL != dst);
    assert(NULL != src);

    char *ret = dst;
    while (*dst != '\0') {
        dst++;
    }
    while ((*dst++ = *src++) != '\0');
    return ret;
}

int strcmp(const char *s1, const char *s2) {
    assert(NULL != s1);
    assert(NULL != s2);
    while (*s1 == *s2) {
        if (*s1 == '\0') {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }
    return (*s1 - *s2);
}

int strncmp(const char *s1, const char *s2, size_t n) {
    assert(NULL != s1);
    assert(NULL != s2);
    assert(n > 0);

    while (n-- > 0 && *s1 == *s2) {
        if (*s1 == '\0') {
            return (*s1 - *s2);
        }
        s1++;
        s2++;
    }

    return (*s1 - *s2);
}

void *memset(void *s, int c, size_t n) {
    assert(NULL != s);
    assert(n > 0);
    unsigned char *p = (unsigned char *)s;
    while (n > 0) {
        *p = (unsigned char) c;
        p++;
        n--;
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
    assert(NULL != dst);
    assert(NULL != src);
    assert(n > 0);
    void *ret = dst;

    if (dst <= src || (char *)dst >= ((char *)src + n)) {
        while (n--) {
            *(char *)dst = *(char *)src;
            dst = (char *)dst + 1;
            src = (char *)src + 1;
        }
    } else {
        dst = (char *)dst + n - 1;
        src = (char *)src + n - 1;
        while (n--) {
            *(char *)dst = *(char *)src;
            dst = (char *)dst - 1;
            src = (char *)src - 1;
        }
    }
    return ret;
}

void *memcpy(void *out, const void *in, size_t n) {
    assert(NULL != out);
    assert(NULL != in);
    assert(n > 0);
    void *ret = out;

    // 判断没有内存重叠，从低地址开始复制
    if (out <= in || (char *)out >= ((char *)in + n)) {
        while (n--) {
            *(char *)out = *(char *)in;
            out = (char *)out + 1;
            in = (char *)in + 1;
        }
    } else {
        // 有内存重叠，从高地址开始复制
        in = (char *)in + n - 1;
        out = (char *)out + n - 1;
        while (n--) {
            *(char *)out = *(char *)in;
            out = (char *)out - 1;
            in = (char *)in - 1;
        }
    }
    return ret;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    assert(NULL != s1);
    assert(NULL != s2);
    assert(n > 0);

    while (--n > 0 && (*(char *)s1 == *(char *)s2)) {
        if (*(char *)s1 == '\0') {
            return (*(unsigned char *)s1 - *(unsigned char *)s2);
        }
        s1 = (char *)s1 + 1;
        s2 = (char *)s2 + 1;
    }

    return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

#endif
