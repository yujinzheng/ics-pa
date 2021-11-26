// lui的功能：20位立即数左移12位，并将低12位置零，写入到rd中
def_EHelper(lui) {
    rtl_li(s, ddest, id_src1->imm);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: lui\timm: %#x, iddest: %#x", id_src1->imm, *ddest);
}

// addi的功能：立即数加法，将立即数加入到rs1中，结果写入rd
def_EHelper(addi) {
    id_src1->imm += id_src2->imm;
    rtl_li(s, ddest, id_src1->imm);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: addi\tsrc1: %#x, imm: %#x, iddest: %#x", id_src1->imm - id_src2->imm, id_src2->imm, *ddest);
}

// slti的功能：比较rs1和有符号扩展的立即数，比较时都视为有符号数，如果rs1更小，向rd写入1，否则写入0
def_EHelper(slti) {

    // 需要先将立即数进行符号位扩展，所以新建一个有符号数int来进行转换
    int val = id_src2->imm;
    val = (val << 20) >> 20;

    // 将rs1中的数强制转为有符号数，然后再对二者进行对比，所以这里直接比较就行了
    int result = (int) id_src1->imm < val ? 1 : 0;
    rtl_li(s, ddest, result);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: slti\tsrc1: %#x, imm: %#x, result: %d, iddest: %#x", id_src1->imm, val, result, *ddest);
}

// sltiu的功能：比较rs1和有符号扩展的立即数，比较时视为无符号数，如果rs1更小，向rd写入1，否则写入0
def_EHelper(sltiu) {

    // 需要先将立即数进行符号位扩展，所以新建一个有符号数int来进行转换
    int val = id_src2->imm;
    val = (val << 20) >> 20;

    // 在比较时将符号扩展后的立即数视为无符号数，由于内存中存的值本身就是无符号数，在比较时会将val也视为无符号数
    // 所以这里直接比较就行了
    int result = id_src1->imm < val ? 1 : 0;
    rtl_li(s, ddest, result);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: sltiu\tsrc1: %#x, imm: %#x, result: %d, iddest: %#x", id_src1->imm, val, result, *ddest);
}

// xori的功能：rs1和有符号扩展的立即数按位异或，结果写入rd中
def_EHelper(xori) {

    // 需要先将立即数进行符号位扩展，所以新建一个有符号数int来进行转换
    int val = id_src2->imm;
    val = (val << 20) >> 20;
    rtl_li(s, ddest, id_src1->imm ^ val);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: xori\tsrc1: %#x, imm: %#x, result: %#x, iddest: %#x", id_src1->imm, val, id_src1->imm ^ val, *ddest);
}

// ori的功能：rs1和有符号扩展的立即数按位或，结果写入rd中
def_EHelper(ori) {

    // 需要先将立即数进行符号位扩展，所以新建一个有符号数int来进行转换
    int val = id_src2->imm;
    val = (val << 20) >> 20;
    rtl_li(s, ddest, id_src1->imm | val);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: ori\tsrc1: %#x, imm: %#x, result: %#x, iddest: %#x", id_src1->imm, val, id_src1->imm | val, *ddest);
}

// andi的功能：rs1和有符号扩展的立即数按位与，结果写入rd中
def_EHelper(andi) {

    // 需要先将立即数进行符号位扩展，所以新建一个有符号数int来进行转换
    int val = id_src2->imm;
    val = (val << 20) >> 20;
    rtl_li(s, ddest, id_src1->imm & val);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: andi\tsrc1: %#x, imm: %#x, result: %#x, iddest: %#x", id_src1->imm, val, id_src1->imm & val, *ddest);
}

// srai的功能：把寄存器rs1右移shamt位，空位用rs1的最高位填充，结果写入rd
def_EHelper(srai) {

    // 这里的shamt值为立即数的低6位，先对其进行按位与处理，保留低6位的数
    // 对于RV32I而言，最高一位没有作用，所以这里与上31
    id_src2->imm = id_src2->imm & 31;

    // rs1需要进行算数右移，因此先转成int再右移
    id_src1->imm = (int) id_src1->imm >> id_src2->imm;

    rtl_li(s, ddest, id_src1->imm);

    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: srai\tsrc1: %#x, shamt: %#x, iddest: %#x", id_src1->imm, id_src2->imm, *ddest);
}

// auipc的功能：20位的立即数左移12位，低12位补零，将得到的32位数与pc的值相加，最后写回寄存器rd中
def_EHelper(auipc) {
    id_src1->imm += s->pc;
    rtl_li(s, ddest, id_src1->imm);

    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: auipc\tmm: %#x, pc: %#x, iddest: %#x", id_src1->imm - s->pc, s->pc, *ddest);
}


// jal的功能：将下一条指令的地址存储在rd中，然后将PC设置为rs1+offset
def_EHelper(jal) {
    rtl_li(s, ddest, s->snpc);

    // 由于立即数需要进行符号扩展，而在赋值立即数时，其类型为unsigned int
    // 所以这里需要先将其转换成int类型，进行符号扩展后再赋值
    int val = id_src1->imm;
    val = (val << 11) >> 11;
    rtl_j(s, s->pc + val);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: jal\tbefore: %#x, after: %#x, iddest: %#x", s->pc, s->dnpc, *ddest);
}

// jalr的功能：将下一条指令的地址存储在rd中，然后将PC设置为rs1+立即数
def_EHelper(jalr) {
    rtl_li(s, ddest, s->snpc);
    rtl_j(s, id_src1->imm + id_src2->imm);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: jalr\tbefore: %#x, after: %#x, iddest: %#x", s->pc, s->dnpc, *ddest);
}

// add的功能：将寄存器rs1与rs2相加，然后将结果写入到rd中
def_EHelper(add) {
    rtl_li(s, ddest, id_src1->imm + id_src2->imm);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: add\tsrc1: %#x, src2: %#x, iddest: %#x", id_src1->imm, id_src2->imm, *ddest);
}

// sub的功能：寄存器rs1减去rs2，然后将结果写入到rd中
def_EHelper(sub) {
    rtl_li(s, ddest, id_src1->imm - id_src2->imm);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: sub\tsrc1: %#x, src2: %#x, iddest: %#x", id_src1->imm, id_src2->imm, *ddest);
}

// sll的功能：寄存器rs1左移rs2位，空出的填入0，结果写入rd，rv32I中，rs2只取低5位
def_EHelper(sll) {
    id_src2->imm = id_src2->imm & 31;
    id_src1->imm = id_src1->imm << id_src2->imm;
    rtl_li(s, ddest, id_src1->imm);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: sll\tsrc1: %#x, src2: %#x, iddest: %#x", id_src1->imm, id_src2->imm, *ddest);
}

// slt的功能：比较rs1和rs2中的数，比较时视为有符号数，如果rs1更小，向xd中写入1，否则写入0
def_EHelper(slt) {
    int result = (int) id_src1->imm < (int) id_src2->imm ? 1 : 0;
    rtl_li(s, ddest, result);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: sll\tsrc1: %#x, src2: %#x, result: %#x, iddest: %#x", (int) id_src1->imm, (int) id_src2->imm, result,
        *ddest);
}

// sltu的功能：比较rs1和rs2，比较时视为无符号数，如果rs1更小，则向rd中写入1，否则写入0
def_EHelper(sltu) {
    int result = id_src1->imm < id_src2->imm ? 1 : 0;
    rtl_li(s, ddest, result);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: sltu\tsrc1: %#x, src2: %#x, result: %#x, iddest: %#x", id_src1->imm, id_src2->imm, result, *ddest);
}

// xor的功能：rs1与rs2按位异或，结果写入rd
def_EHelper(xor) {
    rtl_li(s, ddest, id_src1->imm ^ id_src2->imm);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: xor\tsrc1: %#x, imm: %#x, result: %#x, iddest: %#x", id_src1->imm, id_src2->imm,
        id_src1->imm & id_src2->imm, *ddest);
}

// beq的功能：若rs1与rs2的值相等，则把pc的值设置为当前值加上符号扩展的偏移offset
def_EHelper(beq) {
    // 需要先将立即数进行符号位扩展，所以新建一个有符号数int来进行转换
    int val = id_dest->imm;
    val = (val << 19) >> 19;

    if (id_src1->imm == id_src2->imm) {
        rtl_j(s, s->pc + val);
    }
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: beq\tsrc1: %#x, src2: %#x, offset: %#x, pc: %#x", id_src1->imm, id_src2->imm, val, s->pc);
}

// bne的功能：若rs1与rs2的值不相等，则把pc的值设置为当前值加上符号扩展的偏移offset
def_EHelper(bne) {
    // 需要先将立即数进行符号位扩展，所以新建一个有符号数int来进行转换
    int val = id_dest->imm;
    val = (val << 19) >> 19;

    if (id_src1->imm != id_src2->imm) {
        rtl_j(s, s->pc + val);
    }
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: bne\tsrc1: %#x, src2: %#x, offset: %#x, pc: %#x", id_src1->imm, id_src2->imm, val, s->pc);
}

// or的功能：rs1与rs2按位或，结果写入rd
def_EHelper(or) {
    rtl_li(s, ddest, id_src1->imm | id_src2->imm);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: or\tsrc1: %#x, src2: %#x, result: %#x, iddest: %#x", id_src1->imm, id_src2->imm,
        id_src1->imm | id_src2->imm, *ddest);
}

// and的功能：rs1与rs2按位与，结果写入rd
def_EHelper(and) {
    rtl_li(s, ddest, id_src1->imm & id_src2->imm);
    Log("before: %#x, after: %#x", s->pc, s->dnpc);
    Log("function: and\tsrc1: %#x, src2: %#x, result: %#x, iddest: %#x", id_src1->imm, id_src2->imm,
        id_src1->imm & id_src2->imm, *ddest);
}