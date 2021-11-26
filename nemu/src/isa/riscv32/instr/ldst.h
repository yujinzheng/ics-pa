// lb的功能：从地址rs1+imm读取一个字节，经符号位扩展后写入rd
def_EHelper(lb) {
    rtl_lm_sig(s, ddest, dsrc1, id_src2->imm, 1, 24);
}

// lh的功能：从地址rs1+imm读取两个字节，经符号扩展后写入rd
def_EHelper(lh) {
    rtl_lm_sig(s, ddest, dsrc1, id_src2->imm, 1, 16);
}

// lw的功能：从地址rs1+imm读取四个字节，经符号扩展后写入rd
def_EHelper(lw) {
    rtl_lm(s, ddest, dsrc1, id_src2->imm, 4);
}

// lbu的功能：从地址rs1+imm读取一个字节，经零扩展后写入rd
def_EHelper(lbu) {
    rtl_lm(s, ddest, dsrc1, id_src2->imm, 4);
}

// lhu的功能：从地址rs1+imm读取两个字节，经零扩展后写入rd
def_EHelper(lhu) {
    rtl_lm(s, ddest, dsrc1, id_src2->imm, 4);
}

// sb的功能：将rs2的低位1个字节存入内存地址rs1+符号扩展的imm中
def_EHelper(sb) {
    rtl_sm(s, ddest, dsrc1, id_src2->imm, 1);
}

// sh的功能：将rs2的低位2个字节存入内存地址rs1+符号扩展的imm中
def_EHelper(sh) {
    rtl_sm(s, ddest, dsrc1, id_src2->imm, 2);
}

// sw的功能：将rs2的低位四个字节存入内存地址(rs1+imm)中
def_EHelper(sw) {
    rtl_sm(s, ddest, dsrc1, id_src2->imm, 4);
}
