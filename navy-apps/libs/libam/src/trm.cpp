#include <am.h>
#include <klib-macros.h>
#include <unistd.h>

//extern char _heap_start;
int main(const char *args);
//Area heap = RANGE(&_heap_start, PMEM_END);
//Area heap;

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)
#ifndef MAINARGS
#define MAINARGS ""
#endif

static const char mainargs[] = MAINARGS;

void putch(char ch) {
    int fb = 1;
    write(fb, &ch, 1);
}

void halt(int code) {
    _exit(code);

    // should not reach here
    while (1);
}

void _trm_init() {
    printf("=====================_trm_init\n");
    int ret = main(mainargs);
    halt(ret);
}
