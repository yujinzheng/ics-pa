#include <common.h>
#include "syscall.h"

extern char _end;
/**
 * 处理SYS_write事件，将write传入的数据输出到串口
 *
 * @param c 上下文信息
 */
void sys_write(Context *c) {
    int fd = c->GPR2;
    char *buf = (char *)c->GPR3;
    int count = c->GPR4;
    int num = 0;
    if (fd == 1 || fd == 2) {
        while (*buf != '\0' && num < count) {
            putch(*buf++);
            num++;
        }
    } else {
        c->GPRx = -1;
        return;
    }
    c->GPRx = num;
}

/**
 * 处理SYS_brk事件
 *
 * @param c 上下文信息
 */
void sys_brk(Context *c) {
    c->GPRx = 0;
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
        default:
            panic("Unhandled syscall ID = %d", a[0]);
    }
}
