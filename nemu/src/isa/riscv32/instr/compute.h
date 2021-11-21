def_EHelper(lui) {
  rtl_li(s, ddest, id_src1->imm);
}

// addi的功能：立即数加法，将立即数加入到rs1中，结果写入rd
def_EHelper(addi) {
    bool success = true;
    id_src1->imm = isa_reg_str2val(id_src1->str, &success);
    if (success == false) {
        printf("Can not get value from reg %s\n", id_src1->str);
        assert(0);
    }
    id_src1->imm += id_src2->imm;
    rtl_li(s, ddest, id_src1->imm);
}

// auipc的功能：20位的立即数左移12位，低12位补零，将得到的32位数与pc的值相加，最后写回寄存器rd中
def_EHelper(auipc) {
    id_src1->imm += s->pc;
    rtl_li(s, ddest, id_src1->imm);
}


// jal的功能：将下一条指令的地址存储在rd中，然后将PC设置为rs1+offset
def_EHelper(jal) {
    rtl_li(s, ddest, s->snpc);

    // 由于立即数需要进行符号扩展，而在赋值立即数时，其类型为unsigned int
    // 所以这里需要先将其转换成int类型，进行符号扩展后再赋值
    int val = id_src1->imm;
    val = (val << 11) >> 11;
    rtl_j(s, s->pc + val);
    Log("before: %#x, after: %#x, iddest: %#x", s->pc, s->dnpc, *ddest);
}

// jalr的功能：将下一条指令的地址存储在rd中，然后将PC设置为rs1+立即数
def_EHelper(jalr) {
    rtl_li(s, ddest, s->snpc);
    bool success = true;
    id_src1->imm = isa_reg_str2val(id_src1->str, &success);
    if (success == false) {
        printf("Can not get value from reg %s\n", id_src1->str);
        assert(0);
    }
    rtl_j(s, id_src1->imm + id_src2->imm);
}