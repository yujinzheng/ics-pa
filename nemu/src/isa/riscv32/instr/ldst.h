def_EHelper(lb) {
    int value = vaddr_read(*dsrc1 + id_src2->imm, 1);
    value = (value << 24) >> 24;
    rtl_li(s, ddest, value);
}

def_EHelper(lh) {
    int value = vaddr_read(*dsrc1 + id_src2->imm, 2);
    value = (value << 16) >> 16;
    rtl_li(s, ddest, value);
}

def_EHelper(lw) {
    rtl_lm(s, ddest, dsrc1, id_src2->imm, 4);
}

def_EHelper(lbu) {
    rtl_lm(s, ddest, dsrc1, id_src2->imm, 1);
}

def_EHelper(lhu) {
    rtl_lm(s, ddest, dsrc1, id_src2->imm, 2);
}

def_EHelper(sb) {
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_sm(s, ddest, dsrc1, id_src2->simm, 1);
}

def_EHelper(sh) {
    id_src2->simm = id_src2->imm;
    id_src2->simm = (id_src2->simm << 20) >> 20;
    rtl_sm(s, ddest, dsrc1, id_src2->simm, 2);
}

def_EHelper(sw) {
    rtl_sm(s, ddest, dsrc1, id_src2->imm, 4);
}
