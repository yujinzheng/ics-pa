#include <common.h>
#include <fs.h>
#include "syscall.h"

extern char _end;
int gettimeofday(intptr_t tv, intptr_t tz);

/**
 * 处理SYS_write事件，将write传入的数据输出到串口
 *
 * @param c 上下文信息
 */
void sys_write(Context *c) {
    printf("=====Sys write fd: %d=====\n", c->GPR2);
    int fd = c->GPR2;
    char *buf = (char *)c->GPR3;
    int count = c->GPR4;
    int num = fs_write(fd, buf, count);
    c->GPRx = num;
}

void sys_read(Context *c) {
    int fd = c->GPR2;
    char *buf = (char *)c->GPR3;
    int count = c->GPR4;
    int num = fs_read(fd, buf, count);
//    printf("=====Sys read fd: %d, return: %d=====\n", c->GPR2, num);
    c->GPRx = num;
}

/**
 * 处理SYS_brk事件
 *
 * @param c 上下文信息
 */
void sys_brk(Context *c) {
    printf("=====Sys brk: %x, %x=====\n", c->GPR2);
    c->GPRx = 0;
}

void sys_open(Context *c) {
    printf("=====Sys open file: %s=====\n", (char *)(c->GPR2));
    c->GPRx = fs_open((char *)(c->GPR2), c->GPR3, c->GPR4);
}

void sys_close(Context *c) {
//    printf("=====Sys close fd: %d=====\n", c->GPR2);
    c->GPRx = fs_close(c->GPR2);
}

void sys_lseek(Context *c) {
//    printf("=====Sys lseek fd: %d=====\n", c->GPR2);
    c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
}

void sys_gettimeofday(Context *c) {
    gettimeofday(c->GPR2, c->GPR3);
}

void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;

    switch (a[0]) {
        case SYS_exit:
            halt(c->GPR1);
            break;
        case SYS_yield:
            yield();
            break;
        case SYS_write:
            sys_write(c);
            break;
        case SYS_brk:
            sys_brk(c);
            break;
        case SYS_open:
            sys_open(c);
            break;
        case SYS_read:
            sys_read(c);
            break;
        case SYS_close:
            sys_close(c);
            break;
        case SYS_lseek:
            sys_lseek(c);
            break;
        case SYS_gettimeofday:
            sys_gettimeofday(c);
            break;
        default:
            panic("Unhandled syscall ID = %d", a[0]);
    }
}
