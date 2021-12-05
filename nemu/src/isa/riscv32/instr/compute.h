static word_t zero_null = 0;

def_EHelper(lui) {
    rtl_li(s, ddest, id_src1->imm);
}

def_EHelper(auipc) {
    rtl_li(s, ddest, s->pc + id_src1->imm);
}

def_EHelper(jal) {
    rtl_li(s, ddest, s->snpc);
    id_src1->simm = id_src1->imm;
    id_src1->simm = (id_src1->simm << 11) >> 11;
    rtl_j(s, s->pc + id_src1->simm);
}

def_EHelper(jalr) {
    rtl_li(s, ddest, s->snpc);
    id_src2->imm = (id_src2->imm << 12) >> 12;
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    rtl_j(s, *x_rs1 + id_src2->imm);
}

def_EHelper(beq) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    if (*x_rs1 == *x_rs2) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 20) >> 20;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

def_EHelper(bne) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    if (*x_rs1 != *x_rs2) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 20) >> 20;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

def_EHelper(blt) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    id_src1->simm = *x_rs1;
    id_src2->simm = *x_rs2;
    if (id_src1->simm < id_src2->simm) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 20) >> 20;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

def_EHelper(bge) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    id_src1->simm = *x_rs1;
    id_src2->simm = *x_rs2;
    if (id_src1->simm >= id_src2->simm) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 20) >> 20;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

def_EHelper(bltu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    if (*x_rs1 < *x_rs2) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 20) >> 20;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

def_EHelper(bgeu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    if (*x_rs1 >= *x_rs2) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 20) >> 20;
        rtl_j(s, s->pc + id_dest->simm);
    }
}


def_EHelper(addi) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_addi(s, ddest, x_rs1, id_src2->simm);
}

def_EHelper(slti) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src1->simm = *x_rs1;
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_li(s, ddest, id_src1->simm < id_src2->simm ? 1 : 0);
}

def_EHelper(sltiu) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src1->imm = *x_rs1;
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    id_src2->imm = id_src2->simm;
    rtl_li(s, ddest, id_src1->imm < id_src2->imm ? 1 : 0);
}

def_EHelper(xori) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_xori(s, ddest, x_rs1, id_src2->simm);
}

def_EHelper(ori) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_ori(s, ddest, x_rs1, id_src2->simm);
}

def_EHelper(andi) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_andi(s, ddest, x_rs1, id_src2->simm);
}

def_EHelper(slli) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    rtl_slli(s, ddest, x_rs1, id_src2->imm);
}

def_EHelper(srli) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    rtl_srli(s, ddest, x_rs1, id_src2->imm);
}

def_EHelper(srai) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    rtl_srai(s, ddest, x_rs1, id_src2->imm);
}

def_EHelper(add) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_add(s, ddest, x_rs1, x_rs2);
}

def_EHelper(sub) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_sub(s, ddest, x_rs1, x_rs2);
}

def_EHelper(sll) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_sll(s, ddest, x_rs1, x_rs2);
}

def_EHelper(slt) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    id_src1->simm = *x_rs1;
    id_src2->simm = *x_rs2;
    rtl_li(s, ddest, id_src1->simm < id_src2->simm ? 1 : 0);
}

def_EHelper(sltu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_li(s, ddest, *x_rs1 < *x_rs2 ? 1 : 0);
}

def_EHelper(xor) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_xor(s, ddest, x_rs1, x_rs2);
}

def_EHelper(srl) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_srl(s, ddest, x_rs1, x_rs2);
}
def_EHelper(sra) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_sra(s, ddest, x_rs1, x_rs2);
}

def_EHelper(or) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_or(s, ddest, x_rs1, x_rs2);
}

def_EHelper(and) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_and(s, ddest, x_rs1, x_rs2);
}

def_EHelper(mul) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_li(s, ddest, *x_rs1 * *x_rs2);
}

def_EHelper(mulh) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    id_src1->simm = *x_rs1;
    long src1_val_l = id_src1->simm;
    src1_val_l = (src1_val_l << 32) >> 32;

    id_src2->simm = *x_rs2;
    long src2_val_l = id_src2->simm;
    src2_val_l = (src2_val_l << 32) >> 32;

    rtl_li(s, ddest, (src1_val_l * src2_val_l) >> 32);
}

def_EHelper(div) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_li(s, ddest, *x_rs1 / *x_rs2);
}

def_EHelper(rem) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_li(s, ddest, *x_rs1 % *x_rs2);
}