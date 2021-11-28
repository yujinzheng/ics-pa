#include <isa.h>
#include <memory/vaddr.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
    int length = sizeof(regs) / sizeof(regs[0]);
    for (int idx = 0; idx < length; idx++) {
        printf("%s\t0x%08x\n", regs[idx], cpu.gpr[idx]._32);
    }
}

// 根据寄存器的名字返回寄存器的编号
int isa_reg_name2num(const char *s) {
    int length = sizeof(regs) / sizeof(regs[0]);
    for (int idx = 0; idx < length; ++idx) {
        if (strcmp(s, regs[idx]) == 0) {
            return idx;
        }
    }
    return -1;
}

// 根据寄存器名字获取寄存器的值
word_t isa_reg_str2val(const char *s, bool *success) {
    *success = true;
    if (strcmp(s, "pc") == 0) {
        return vaddr_read(cpu.pc, 4);
    }
    int num = isa_reg_name2num(s);
    if (num >= 0) {
        return cpu.gpr[num]._32;
    }
    *success = false;
    return 0;
}
