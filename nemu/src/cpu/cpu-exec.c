#include <cpu/cpu.h>
#include <cpu/exec.h>
#include <cpu/difftest.h>
#include <isa-all-instr.h>
#include <locale.h>
#include <elf.h>
#include "../monitor/sdb/sdb.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 10

CPU_state cpu = {};
CSR_reg csrReg = {};
uint64_t g_nr_guest_instr = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
const rtlreg_t rzero = 0;
rtlreg_t tmp_reg[4];

void device_update();

void fetch_decode(Decode *s, vaddr_t pc);

#ifdef CONFIG_WATCHPOINT
WP *get_head();
static void debug_hook(vaddr_t pc) {
    // 获取wp中的数据并计算值是否发生变化
    WP *head = get_head();
    if (head == NULL) {
        return;
    }
    WP *temp_wp = head->next;
    while (temp_wp != NULL) {
        char expr_str[128];
        memcpy(expr_str, temp_wp->expression, 128);
        bool success = true;
        word_t result = expr(expr_str, &success);
        if (success == false) {
            printf("The calculation data is incorrect, exp is: %s\n", expr_str);
            return;
        }
        if (result != temp_wp->value) {
            nemu_state.state = NEMU_STOP;
            printf("Hit watchpoint %d at address 0x%08x\n", temp_wp->NO, pc);
            printf("Expression is: %s\n", expr_str);
            printf("old_value = 0x%08x, new_value = 0x%08x\n", temp_wp->value, result);
            temp_wp->value = result;
        }
        temp_wp = temp_wp->next;
    }
}
#endif

#ifdef CONFIG_ITRACE_COND
RingBuffer *ringBuffer = NULL;
int ring_buffer_create(RingBuffer *rbuf) {
    rbuf->buffer = malloc(RING_BUFFER_MAXSIZE);
    rbuf->head = rbuf->tail = 0;
    rbuf->size = RING_BUFFER_MAXSIZE;
    return 0;
}

void ring_buffer_free(RingBuffer *rbuf) {
    free(rbuf->buffer);
    rbuf->head = rbuf->tail = 0;
}

int ring_buffer_write(RingBuffer *rbuf, char *wbuf) {
    if (rbuf->head >= rbuf->size) {
        rbuf->head = 0;
    }
    memcpy(rbuf->buffer + rbuf->head, wbuf, RING_BUFFER_SIZE);
    rbuf->head += RING_BUFFER_SIZE;
    return 0;
}

void ring_buffer_print(RingBuffer *rbuf) {
    unsigned int point = 0;
    char temp_buf[RING_BUFFER_SIZE];
    unsigned int target = rbuf->head;
    if (rbuf->head == 0) {
        target = RING_BUFFER_MAXSIZE + RING_BUFFER_SIZE;
    }
    while (point < RING_BUFFER_MAXSIZE) {
        memcpy(temp_buf, rbuf->buffer + point, RING_BUFFER_SIZE);
        if (point + RING_BUFFER_SIZE == target) {
            printf("--> %s\n", temp_buf);
        } else {
            printf("    %s\n", temp_buf);
        }
        point += RING_BUFFER_SIZE;
    }
}

static void iringbuf(char *asmbuf) {
    if (ringBuffer == NULL) {
        ringBuffer = (RingBuffer *)malloc(sizeof(RingBuffer));
        if (ringBuffer == NULL) {
            Log("Init ringBuffer failed, malloc failed");
        }
        ring_buffer_create(ringBuffer);
    }
    ring_buffer_write(ringBuffer, asmbuf);
}
#endif

#ifdef CONFIG_ITRACE_FUN
char *get_fun_and_addr(vaddr_t pc, bool *success);
Elf32_Sym *sh_sym_tab = NULL;
Elf32_Shdr *sh_sym_dr = NULL;
char *sh_str_tab = NULL;
void init_ftrace_log(const char *log_file);

// 根据当前的地址获取函数名称和地址
// 返回的数据使用完成之后一定要free
char *get_fun_and_addr(vaddr_t addr, bool *success) {
    *success = true;
    int str_offset = -1;
    int fun_addr = -1;
    if (sh_sym_tab == NULL || sh_str_tab == NULL || sh_sym_dr == NULL) {
        printf("Had not init ftrace\n");
        *success = false;
        return NULL;
    }
    for (int idx = 0; idx < sh_sym_dr->sh_size; ++idx) {
        if (ELF32_ST_TYPE(sh_sym_tab[idx].st_info) == STT_FUNC) {
            unsigned int scope = sh_sym_tab[idx].st_value + sh_sym_tab[idx].st_size;
            if (addr >= sh_sym_tab[idx].st_value && addr < scope) {
                str_offset = sh_sym_tab[idx].st_name;
                fun_addr = sh_sym_tab[idx].st_value;
                break;
            }
        }
    }

    char *result = (char *)malloc(strlen(&sh_str_tab[str_offset]) + 11);
    if (result == NULL) {
        printf("Fun name and addr malloc failed\n");
        *success = false;
        return NULL;
    }

    if (str_offset < 0) {
        printf("Can not find correct sym tab, addr is: 0x%08x\n", addr);
        sprintf(result, "???@0x%08x", fun_addr);
    } else {
        sprintf(result, "%s@0x%08x", &sh_str_tab[str_offset], fun_addr);
    }
    return result;
}

int check_elf(FILE *file, Elf32_Ehdr *elf_head) {
    int flag;

    // 将ELF头读取出来
    // 参数1：读取内容存储地址，参数2：读取内容大小，参数3：读取次数，参数4：文件读取引擎，返回值为成功读取的次数
    // 作用：从文件中读取n次数据，每次读取指定内容大小，然后将数据存储在给定的内容中
    flag = (int)fread(elf_head, sizeof(Elf32_Ehdr), 1, file);
    if (1 != flag) {
        printf("Fail to read head table\n");
        return 1;
    }

    // elf_ident中存储的是ELF的魔数和其他信息
    // elf_ident的魔数在第一位，取值为0x7f
    // elf_ident的第二到四位存储的是“ELF”
    if (elf_head->e_ident[0] != 0x7f || elf_head->e_ident[1] != 'E' || elf_head->e_ident[2] != 'L' ||
        elf_head->e_ident[3] != 'F') {
        printf("Not a ELF file\n");
        return 1;
    }
    return 0;
}

// 读取节区头信息
int get_shdr(FILE *file, Elf32_Ehdr *elf_head, Elf32_Shdr *sh_eh_dr) {
    int flag;

    // 设置ELF文件的偏移量（跳过ELF头），准备读取节表区信息
    // 参数1：输入流，参数2：偏移量，参数3：开始添加偏移的位置，返回0则代表成功
    // 将文件流的读取指针偏移到偏移量+开始偏移的位置处
    flag = (int)fseek(file, elf_head->e_shoff, SEEK_SET);
    if (0 != flag) {
        printf("Fail to seed section table\n");
        return 1;
    }

    // 读取节区头信息
    // 节表存储的是每个节区的头信息，读取到节区头后，就可以从里面读取符号表头和字符串表头
    flag = (int)fread(sh_eh_dr, sizeof(Elf32_Shdr) * elf_head->e_shnum, 1, file);
    if (1 != flag) {
        printf("Fail to read section table\n");
        return 1;
    }

    // 节区头读取完毕后，恢复输入流的指针到开头
    rewind(file);
    return 0;
}

// 读取字符串表
int get_str_tab(FILE *file, Elf32_Ehdr *elf_head, Elf32_Shdr *sh_eh_dr) {
    int flag;

    // 将ELF文件的指针偏移到字符串表的位置
    flag = (int)fseek(file, sh_eh_dr[elf_head->e_shstrndx - 1].sh_offset, SEEK_SET);
    if (0 != flag) {
        printf("Offset pointer to str table failed\n");
        return 1;
    }

    // 节表中的第e_shstrndx项是字符串表，准备读取字符串表
    sh_str_tab = (char *)malloc(sh_eh_dr[elf_head->e_shstrndx - 1].sh_size);
    if (sh_str_tab == NULL) {
        printf("String table malloc failed\n");
        return 1;
    }

    // 从ELF文件中读取字符串表信息（ELF文件之前已经设置过offset）
    flag = (int)fread(sh_str_tab, sh_eh_dr[elf_head->e_shstrndx - 1].sh_size, 1, file);
    if (1 != flag) {
        printf("Fail to read string table\n");
        return 1;
    }

    // 文件读取完毕后，恢复输入流的指针到开头
    rewind(file);
    return 0;
}

// 获取字符表信息
int get_sym_tab(FILE *file, Elf32_Ehdr *elf_head, Elf32_Shdr *sh_eh_dr) {
    int flag;

    // 遍历节区头，将sh_type等于2的数据读取出来，这就是字符表头
    sh_sym_dr = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr));
    if (NULL == sh_sym_dr) {
        printf("Sym head malloc failed\n");
        return 1;
    }
    for (int idx = 0; idx < elf_head->e_shnum; idx++) {
        if (sh_eh_dr[idx].sh_type == SHT_SYMTAB) {
            *sh_sym_dr = sh_eh_dr[idx];
            break;
        }
    }

    if (sh_sym_dr == NULL || sh_sym_dr->sh_type != 2) {
        printf("Can not find sym table\n");
        return 1;
    }

    // 根据字符表头读取字符表信息
    sh_sym_tab = (Elf32_Sym *)malloc(sizeof(Elf32_Sym) * sh_sym_dr->sh_size);
    if (NULL == sh_sym_tab) {
        printf("Sym table malloc failed\n");
        return 1;
    }
    // 将ELF文件的指针偏移到字符表的位置
    flag = fseek(file, sh_sym_dr->sh_offset, SEEK_SET);
    if (0 != flag) {
        printf("Offset pointer to sym table failed\n");
        return 1;
    }

    // 从ELF文件中读取字符表信息
    flag = (int)fread(sh_sym_tab, sh_sym_dr->sh_size, 1, file);
    if (1 != flag) {
        printf("Fail to read sym table\n");
        return 1;
    }
    return 0;
}

int parse_elf(FILE *file) {
    FILE *elf_file = file;
    Elf32_Ehdr *elf_head = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (NULL == elf_head) {
        printf("ELF head malloc failed\n");
        return 1;
    }

    // 检查elf文件格式是否正确
    if (check_elf(file, elf_head) == 1) {
        return 1;
    }

    // 获取节区头信息
    Elf32_Shdr *sh_eh_dr = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr) * elf_head->e_shnum);
    if (NULL == sh_eh_dr) {
        printf("Section table malloc failed\n");
        return 1;
    }

    if (get_shdr(elf_file, elf_head, sh_eh_dr) == 1) {
        return 1;
    }

    // 获取字符串表
    if (get_str_tab(elf_file, elf_head, sh_eh_dr) == 1) {
        return 1;
    }

    // 获取字符表
    if (get_sym_tab(elf_file, elf_head, sh_eh_dr) == 1) {
        return 1;
    }

    free(elf_head);
    free(sh_eh_dr);
    return 0;
}

void init_ftrace(const char *elf_file, const char *ftrace_log) {
    init_ftrace_log(ftrace_log);
    if (elf_file != NULL) {
        FILE *fp = fopen(elf_file, "r");
        assert(fp != NULL);
        int a = parse_elf(fp);
        fclose(fp);
        if (a == 1) {
            assert(0);
        }
    }
}

#else
void init_ftrace(const char *elf_file, const char *ftrace_log) { }
#endif

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
    if (ITRACE_COND) log_write("%s\n", _this->logbuf);
    if (ITRACE_COND) iringbuf(_this->logbuf);
#endif

#ifdef CONFIG_WATCHPOINT
    debug_hook(_this->pc);
#endif
    if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
    IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
}

#include <isa-exec.h>

#define FILL_EXEC_TABLE(name) [concat(EXEC_ID_, name)] = concat(exec_, name),
static const void *g_exec_table[TOTAL_INSTR] = {
        MAP(INSTR_LIST, FILL_EXEC_TABLE)
};

static void fetch_decode_exec_updatepc(Decode *s) {
    fetch_decode(s, cpu.pc);
    s->EHelper(s);
    cpu.pc = s->dnpc;
}

static void statistic() {
    IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%ld", "%'ld")
    Log("host time spent = " NUMBERIC_FMT " us", g_timer);
    Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_instr);
    if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " instr/s", g_nr_guest_instr * 1000000 / g_timer);
    else
        Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
    isa_reg_display();
#ifdef CONFIG_ITRACE_COND
    ring_buffer_print(ringBuffer);
    ring_buffer_free(ringBuffer);
#endif
    statistic();
}

void fetch_decode(Decode *s, vaddr_t pc) {
    s->pc = pc;
    s->snpc = pc;
    int idx = isa_fetch_decode(s);
    s->dnpc = s->snpc;
    s->EHelper = g_exec_table[idx];
#ifdef CONFIG_ITRACE
    char *p = s->logbuf;
    p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
    int ilen = s->snpc - s->pc;
    int i;
    uint8_t *instr = (uint8_t *) &s->isa.instr.val;
    for (i = 0; i < ilen; i++) {
        p += snprintf(p, 4, " %02x", instr[i]);
    }
    int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
    int space_len = ilen_max - ilen;
    if (space_len < 0) space_len = 0;
    space_len = space_len * 3 + 1;
    memset(p, ' ', space_len);
    p += space_len;

    void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
    disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
                MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *) &s->isa.instr.val, ilen);
#endif
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
    g_print_step = (n < MAX_INSTR_TO_PRINT);
    switch (nemu_state.state) {
        case NEMU_END:
        case NEMU_ABORT:
            printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
            return;
        default:
            nemu_state.state = NEMU_RUNNING;
    }

    uint64_t timer_start = get_time();

    Decode s;
    for (; n > 0; n--) {
        fetch_decode_exec_updatepc(&s);
        g_nr_guest_instr++;
        trace_and_difftest(&s, cpu.pc);
        if (nemu_state.state != NEMU_RUNNING) break;
        IFDEF(CONFIG_DEVICE, device_update());
    }

    uint64_t timer_end = get_time();
    g_timer += timer_end - timer_start;

    switch (nemu_state.state) {
        case NEMU_RUNNING:
            nemu_state.state = NEMU_STOP;
            break;

        case NEMU_END:
        case NEMU_ABORT:
            Log("nemu: %s at pc = " FMT_WORD,
                (nemu_state.state == NEMU_ABORT ? ASNI_FMT("ABORT", ASNI_FG_RED) :
                 (nemu_state.halt_ret == 0 ? ASNI_FMT("HIT GOOD TRAP", ASNI_FG_GREEN) :
                  ASNI_FMT("HIT BAD TRAP", ASNI_FG_RED))),
                nemu_state.halt_pc);
            // fall through
        case NEMU_QUIT:
#ifdef CONFIG_ITRACE_COND
            ring_buffer_print(ringBuffer);
            ring_buffer_free(ringBuffer);
#endif
            statistic();
    }
}
