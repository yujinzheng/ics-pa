static word_t zero_null = 0;
#ifdef CONFIG_ITRACE_FUN
static word_t call_depth = 1;
#endif

/**
 * x[rd] = sext(imm[31:12] << 12)
 */
def_EHelper(lui) {
    rtl_li(s, ddest, id_src1->imm);
}

/**
 * x[rd] = pc + sext(imm[31:12] << 12)
 */
def_EHelper(auipc) {
    rtl_li(s, ddest, s->pc + id_src1->imm);
}

/**
 * x[rd] = pc + 4; pc += sext(offset)
 */
def_EHelper(jal) {
    rtl_li(s, ddest, s->snpc);
    id_src1->simm = id_src1->imm;
    id_src1->simm = (id_src1->simm << 11) >> 11;
    rtl_j(s, s->pc + id_src1->simm);
#ifdef CONFIG_ITRACE_FUN
    bool success = true;
    char *fun_and_addr = get_fun_and_addr(s->dnpc, &success);
    if (success == true) {
        ftrace_write("0x%08x:%*scall [%s]\n", s->pc, call_depth," ", fun_and_addr);
        call_depth++;
    }
    free(fun_and_addr);
#endif
}

/**
 * t = pc + 4; pc = (x[rs1] + sext(offset)) & ~1; x[rd] = t
 */
def_EHelper(jalr) {
    vaddr_t t = s->snpc;
    vaddr_t result = (*id_src1->preg + id_src2->simm) & ~1;
    rtl_li(s, ddest, t);
    rtl_j(s, result);
#ifdef CONFIG_ITRACE_FUN
    // 先判断是否是返回，ret是一个伪指令，等价为jalr ra 0($0)
    if (s->isa.instr.i.rs1 == 1 && s->isa.instr.i.rd == 0 && id_src2->imm == 0) {
        bool success = true;
        char *fun_and_addr = get_fun_and_addr(s->dnpc, &success);
        if (success == true) {
            call_depth--;
            ftrace_write("0x%08x:%*sret [%s]\n", s->pc, call_depth," ", fun_and_addr);
        }
        free(fun_and_addr);
    } else {
        bool success = true;
        char *fun_and_addr = get_fun_and_addr(s->dnpc, &success);
        if (success == true) {
            ftrace_write("0x%08x:%*scall [%s]\n", s->pc, call_depth," ", fun_and_addr);
            call_depth++;
        }
        free(fun_and_addr);
    }
#endif
}

/**
 * if (x[rs1] == x[rs2]) pc += setx(offset)
 */
def_EHelper(beq) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    if (*x_rs1 == *x_rs2) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 19) >> 19;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

/**
 * if (x[rs1] != x[rs2]) pc += setx(offset)
 */
def_EHelper(bne) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    if (*x_rs1 != *x_rs2) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 19) >> 19;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

/**
 * if (x[rs1] < x[rs2]) pc += setx(offset)
 */
def_EHelper(blt) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    id_src1->simm = *x_rs1;
    id_src2->simm = *x_rs2;
    if (id_src1->simm < id_src2->simm) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 19) >> 19;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

/**
 * if (x[rs1] >= x[rs2]) pc += setx(offset)
 */
def_EHelper(bge) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    id_src1->simm = *x_rs1;
    id_src2->simm = *x_rs2;
    if (id_src1->simm >= id_src2->simm) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 19) >> 19;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

/**
 * if ((u)x[rs1] < (u)x[rs2]) pc += setx(offset)
 */
def_EHelper(bltu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    if (*x_rs1 < *x_rs2) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 19) >> 19;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

/**
 * if ((u)x[rs1] >= (u)x[rs2]) pc += setx(offset)
 */
def_EHelper(bgeu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    if (*x_rs1 >= *x_rs2) {
        id_dest->simm = id_dest->imm;
        id_dest->simm = (id_dest->simm << 19) >> 19;
        rtl_j(s, s->pc + id_dest->simm);
    }
}

/**
 * x[rx] = x[rs1] + sext(imm)
 */
def_EHelper(addi) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_addi(s, ddest, x_rs1, id_src2->simm);
}

/**
 * x[rx] = x[rs1] < sext(imm) ? 1 : 0
 */
def_EHelper(slti) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src1->simm = *x_rs1;
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_li(s, ddest, id_src1->simm < id_src2->simm ? 1 : 0);
}

/**
 * t = sext(imm); x[rx] = x[rs1] < (u)t ? 1 : 0
 */
def_EHelper(sltiu) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src1->imm = *x_rs1;
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    id_src2->imm = id_src2->simm;
    rtl_li(s, ddest, id_src1->imm < id_src2->imm ? 1 : 0);
}

/**
 * x[rd] = x[rs1] ^ sext(imm)
 */
def_EHelper(xori) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_xori(s, ddest, x_rs1, id_src2->simm);
}

/**
 * x[rd] = x[rs1] | sext(imm)
 */
def_EHelper(ori) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_ori(s, ddest, x_rs1, id_src2->simm);
}

/**
 * x[rd] = x[rs1] & sext(imm)
 */
def_EHelper(andi) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_andi(s, ddest, x_rs1, id_src2->simm);
}

/**
 * x[rd] = x[rs1] << shamt
 */
def_EHelper(slli) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    rtl_slli(s, ddest, x_rs1, id_src2->imm);
}

/**
 * x[rd] = x[rs1] >> (u)x[rs2]
 */
def_EHelper(srli) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    rtl_srli(s, ddest, x_rs1, id_src2->imm);
}

/**
 * x[rd] = x[rs1] >> shamt
 */
def_EHelper(srai) {
    rtlreg_t *x_rs1 = s->isa.instr.i.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.i.rs1);
    rtl_srai(s, ddest, x_rs1, id_src2->imm);
}

/**
 * x[rd] = x[rs1] + x[rs2]
 */
def_EHelper(add) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_add(s, ddest, x_rs1, x_rs2);
}

/**
 * x[rd] = x[rs1] - x[rs2]
 */
def_EHelper(sub) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_sub(s, ddest, x_rs1, x_rs2);
}

/**
 * x[rd] = x[rs1] << x[rs2]
 */
def_EHelper(sll) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_sll(s, ddest, x_rs1, x_rs2);
}

/**
 * x[rd] = x[rs1] < x[rs2] ? 1 : 0
 */
def_EHelper(slt) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    id_src1->simm = *x_rs1;
    id_src2->simm = *x_rs2;
    rtl_li(s, ddest, id_src1->simm < id_src2->simm ? 1 : 0);
}

/**
 * x[rd] = (u)x[rs1] < (u)x[rs2] ? 1 : 0
 */
def_EHelper(sltu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_li(s, ddest, *x_rs1 < *x_rs2 ? 1 : 0);
}

/**
 * x[rd] = x[rs1] ^ x[rs2]
 */
def_EHelper(xor) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_xor(s, ddest, x_rs1, x_rs2);
}

/**
 * x[rd] = x[rs1] >> x[rs2]
 */
def_EHelper(srl) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_srl(s, ddest, x_rs1, x_rs2);
}

/**
 * x[rd] = x[rs1] >> (s)x[rs2]
 */
def_EHelper(sra) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_sra(s, ddest, x_rs1, x_rs2);
}

/**
 * x[rd] = x[rs1] | x[rs2]
 */
def_EHelper(or) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_or(s, ddest, x_rs1, x_rs2);
}

/**
 * x[rd] = x[rs1] & x[rs2]
 */
def_EHelper(and) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_and(s, ddest, x_rs1, x_rs2);
}

/**
 * x[rd] = x[rs1] * x[rs2]
 */
def_EHelper(mul) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_li(s, ddest, *x_rs1 * *x_rs2);
}

/**
 * x[rs1] = (2XLEN)x[rs1];x[rs2] = (2XLEN)x[rs2];x[rd] = x[rs1] * x[rs2]
 */
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

/**
 * x[rs1] = (2XLEN)x[rs1];x[rs2] = (u)(2XLEN)x[rs2];x[rd] = x[rs1] * x[rs2]
 */
def_EHelper(mulhsu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    id_src1->imm = *x_rs1;
    long src1_val_l = id_src1->imm;
    src1_val_l = (src1_val_l << 32) >> 32;

    id_src2->imm = *x_rs2;
    unsigned long src2_val_l = id_src2->imm;
    src2_val_l = (src2_val_l << 32) >> 32;

    rtl_li(s, ddest, (src1_val_l * src2_val_l) >> 32);
}

/**
 * x[rs1] = (u)(2XLEN)x[rs1];x[rs2] = (u)(2XLEN)x[rs2];x[rd] = x[rs1] * x[rs2]
 */
def_EHelper(mulhu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    id_src1->imm = *x_rs1;
    unsigned long src1_val_l = id_src1->imm;
    src1_val_l = (src1_val_l << 32) >> 32;

    id_src2->imm = *x_rs2;
    unsigned long src2_val_l = id_src2->imm;
    src2_val_l = (src2_val_l << 32) >> 32;

    rtl_li(s, ddest, (src1_val_l * src2_val_l) >> 32);
}

/**
 * x[rd] = x[rs1] / x[rs2]
 */
def_EHelper(div) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    int rs1 = *x_rs1;
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    int rs2 = *x_rs2;
    if (rs2 == 0) {
        Log("Illegal division by zero of div");
        assert(0);
    }
    rtl_li(s, ddest, rs1 / rs2);
}

/**
 * x[rd] = (u)x[rs1] / (u)x[rs2]
 */
def_EHelper(divu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    if (*x_rs2 == 0) {
        Log("Illegal division by zero of divu");
        assert(0);
    }
    rtl_li(s, ddest, *x_rs1 / *x_rs2);
}

/**
 * x[rd] = x[rs1] % x[rs2]
 */
def_EHelper(rem) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    int rs1 = *x_rs1;
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    int rs2 = *x_rs2;
    rtl_li(s, ddest, rs1 % rs2);
}

/**
 * x[rd] = (u)x[rs1] % (u)x[rs2]
 */
def_EHelper(remu) {
    rtlreg_t *x_rs1 = s->isa.instr.r.rs1 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs1);
    rtlreg_t *x_rs2 = s->isa.instr.r.rs2 == 0 ? &zero_null : &gpr(s->isa.instr.r.rs2);
    rtl_li(s, ddest, *x_rs1 % *x_rs2);
}