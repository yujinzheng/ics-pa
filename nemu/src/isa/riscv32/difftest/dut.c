#include <isa.h>
#include <cpu/difftest.h>
#include <memory/paddr.h>
#include "../local-include/reg.h"

// 判断ref和dut的寄存器的值是否相等
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
//    printf("===================ref pc: %08x, cpu pc: %08x\n", ref_r->pc, cpu.pc);
    if (ref_r->pc != cpu.pc) {
        return false;
    }

    int ref_r_gpr_size = sizeof(ref_r->gpr) / sizeof(ref_r->gpr[0]);

//    printf("===================ref gpr size: %d, cpu gpr size: %ld\n", ref_r_gpr_size, sizeof(cpu.gpr)  / sizeof(ref_r->gpr[0]));
    if (ref_r_gpr_size != sizeof(cpu.gpr)  / sizeof(ref_r->gpr[0])) {
        return false;
    }

    for (int idx = 0; idx < ref_r_gpr_size; idx++) {
//        printf("===================id : %d, ref gpr value: %08x, cpu gpr value: %08x\n", idx, ref_r->gpr[idx]._32, cpu.gpr[idx]._32);
        if (ref_r->gpr[idx]._32 != cpu.gpr[idx]._32) {
            return false;
        }
    }

    return true;
}

void isa_difftest_attach() {
}
