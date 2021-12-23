#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context *(*user_handler)(Event, Context *) = NULL;

const char *regs[] = {
        "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void print_etrace(Context *c) {
    printf("===================etrace begin===================\n");
    printf("%s\t\t%s\n", "项目", "值");
    printf("%s\t\t0x%08x\n", "mcause", c->mcause);
    printf("%s\t\t0x%08x\n", "mstatus", c->mstatus);
    printf("%s\t\t0x%08x\n", "mepc", c->mepc);
    for (int idx = 0; idx < 32; idx++) {
        printf("%s\t\t0x%08x\n", regs[idx], c->gpr[idx]);
    }
    printf("===================etrace end===================\n\n");
}

Context *__am_irq_handle(Context *c) {
    print_etrace(c);
    if (user_handler) {
        Event ev = {0};
        switch (c->mcause) {
            case -1:
                ev.event = EVENT_YIELD;
                break;
            default:
                ev.event = EVENT_ERROR;
                break;
        }

        c = user_handler(ev, c);
        assert(c != NULL);
    }
    return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context *(*handler)(Event, Context *)) {
    // initialize exception entry
    // 这里是将trap指令存入到mtvec中，也就是存放入口地址操作
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

    // register event handler
    user_handler = handler;

    return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
    return NULL;
}

void yield() {
    asm volatile("li a7, -1; ecall");
}

bool ienabled() {
    return false;
}

void iset(bool enable) {
}
