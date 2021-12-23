def_EHelper(inv) {
  rtl_hostcall(s, HOSTCALL_INV, NULL, NULL, NULL, 0);
}

def_EHelper(ecall) {
    bool success = true;
    word_t No = isa_reg_str2val("a7", &success);
    if (success == false) {
        Log("Can not get value from reg: a7");
        assert(0);
    }
    difftest_skip_ref();
    rtl_j(s, isa_raise_intr(No, MEPC));
}

def_EHelper(mret) {
    word_t csr_t = isa_reg_get_csr(MEPC);
    difftest_skip_ref();
    rtl_j(s, csr_t + 4);
}

def_EHelper(csrrw) {
    word_t csr_value = id_src2->imm;
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    word_t csr_t = isa_reg_get_csr(csr_value);
    isa_reg_set_csr(csr_value, *x_rs1);
    difftest_skip_ref();
    rtl_li(s, ddest, csr_t);
}

def_EHelper(csrrs) {
    word_t csr_value = id_src2->imm;
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    word_t csr_t = isa_reg_get_csr(csr_value);
    isa_reg_set_csr(csr_value, (csr_t | *x_rs1));
    difftest_skip_ref();
    rtl_li(s, ddest, csr_t);
}

def_EHelper(nemu_trap) {
  rtl_hostcall(s, HOSTCALL_EXIT, NULL, &gpr(10), NULL, 0); // gpr(10) is $a0
}
