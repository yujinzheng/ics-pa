#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
    int length = sizeof(regs) / sizeof(regs[0]);
    for (int idx = 0; idx < length; ++idx) {
        printf("%s\t0x%08x\n", regs[idx], cpu.gpr[idx]._32);
    }
}

int isa_reg_name2num(const char *s) {
    int length = sizeof(regs) / sizeof(regs[0]);
    if (strcmp(s, regs[0]) == 0) {
        return 0;
    }
    for (int idx = 1; idx < length; ++idx) {
        char temp[4] = {};

        // 除了$0外，如果寄存器的名字前缀是$，则表中的寄存器名字也带一个$
        if (s[0] == '$') {
            temp[0] = '$';
        }
        strcat(temp, regs[idx]);
        if (strcmp(s, temp) == 0) {
            return idx;
        }
    }
    return -1;
}

// 根据寄存器名字获取寄存器的值
word_t isa_reg_str2val(const char *s, bool *success) {
    int num = isa_reg_name2num(s);
    if (num >= 0) {
        *success = true;
        return cpu.gpr[num]._32;
    }
    *success = false;
    return 0;
}
