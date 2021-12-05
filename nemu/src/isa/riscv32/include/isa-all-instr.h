#include <cpu/decode.h>
#include "../local-include/rtl.h"

#define INSTR_LIST(f) f(lui) f(auipc) f(jal) f(jalr) f(beq) f(bne) f(blt) f(bge) f(bltu) f(bgeu) f(lb) f(lh) f(lw) \
                      f(lbu) f(lhu) f(sb) f(sh) f(sw) f(addi) f(slti) f(sltiu) f(xori) f(ori) f(andi) f(slli) f(srli) \
                      f(srai) f(add) f(sub) f(sll) f(slt) f(sltu) f(xor) f(srl) f(sra) f(or) f(and) f(mul) f(mulh) \
                      f(div) f(rem) f(inv) f(nemu_trap)

def_all_EXEC_ID();

/* f(fence) f(fencei) f(ecall) f(ebreak) f(csrrw) f(csrrs) f(csrrc) f(csrrwi) f(cssrrsi) f(csrrci) */