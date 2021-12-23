#include <isa.h>
#include <rtl/rtl.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
    isa_reg_set_csr(epc, cpu.pc);
    isa_reg_set_csr(MCAUSE, NO);
    return isa_reg_get_csr(MTVEC);
}

word_t isa_query_intr() {
    return INTR_EMPTY;
}
