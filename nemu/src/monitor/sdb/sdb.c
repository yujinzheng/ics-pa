#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <regex.h>
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}


// 继续运行命令
static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


// 让程序单步执行N条指令后暂停执行
static int cmd_si(char *args) {
    int steps;
    if (args == NULL) {
        steps = 1;
    } else {
        char *arg = strtok(args, " ");
        char *next_arg = strtok(NULL, " ");

        // 判断是否有多个参数
        if (next_arg != NULL) {
            printf("too many args\n");
            return 0;
        } else {
            for (int start = 0; start < strlen(arg); start++) {
                if (!isdigit(*(arg+start))) {
                    printf("the arg is invalid\n");
                    return 0;
                }
            }
            steps = atoi(arg);
        }
    }
    cpu_exec(steps);
    return 0;
}


// 打印寄存器状态或监视点信息
static int cmd_info(char *args) {
    if (args == NULL) {
        printf("please input args!\n");
        return 0;
    } else {
        char *arg = strtok(args, " ");
        char *next_arg = strtok(NULL, " ");

        // 判断是否有多个参数
        if (next_arg != NULL) {
            printf("too many args\n");
            return 0;
        } else {
            switch (*arg) {
                case 'r':isa_reg_display(); break;
                case 'w': printf("w"); break;
                default: printf("invalid args!\n"); break;
            }
        }
    }
    return 0;
}


// 求出表达式的值，并将结果作为起始内存地址，以十六进制形式输出连续的N个4字节
static int cmd_x(char *args) {
    if (args == NULL) {
        printf("please input args!\n");
        return 0;
    } else {

        // 检查是否缺少参数
        char *first_arg = strtok(args, " ");
        if (first_arg == NULL) {
            printf("args miss: first arg\n");
            return 0;
        }
        char *second_arg = strtok(NULL, " ");
        if (second_arg == NULL) {
            printf("args miss: second arg\n");
            return 0;
        }

        char *third_arg = strtok(NULL, " ");
        if (third_arg != NULL) {
            printf("too many args\n");
            return 0;
        }

        // 检查需要扫描的内存数，如果输入不为数字，则报错处理
        for (int start = 0; start < strlen(first_arg); start++) {
            if (!isdigit(*(first_arg+start))) {
                printf("the arg is invalid: %s\n", first_arg);
                return 0;
            }
        }
        int steps = strtol(first_arg, &first_arg, 10);

        // 检查输入的表达式的格式
        const char * pattern = "^0x[0-9a-f]{8}";
        int flag = REG_EXTENDED;
        regmatch_t pmatch[1];
        const size_t nmatch = 1;
        regex_t reg;

        // 编译正则表达式
        regcomp(&reg, pattern, flag);

        // 执行正则表达式和缓存的比较
        int status = regexec(&reg, second_arg, nmatch, pmatch, 0);
        regfree(&reg);
        if (status == REG_NOMATCH) {
            printf("invalid address: %s\n", second_arg);
            return 0;
        }

        printf("%s:\t", second_arg);
        uint32_t addr = (uint32_t)strtol(second_arg, NULL, 16);
        for (int idx = 0; idx < steps; ++idx) {
            word_t addr_value = vaddr_read(addr + 4 * idx, 4);
            printf("0x%08x\t", addr_value);
        }
        printf("\n");
    }
    return 0;
}


// 求出表达式的值
static int cmd_p(char *args) {
    if (args == NULL) {
        printf("args miss: please input expression\n");
        return 0;
    } else {
        bool success = true;
        int result = expr(args, &success);
        if (success == false) {
            printf("Can not get result of express %s\n", args);
            return 0;
        }
        printf("the express %s result is: %d\n", args, result);
    }
    return 0;
}

// 设置监视点，当表达式 的值发生变化时，暂停程序的执行
static int cmd_w(char *args) {
    WP *wp = new_wp();
    wp->exp = args;
    bool success = true;
    int result = expr(args, &success);
    if (success == false) {
        printf("The calculation data is incorrect, exp is: %s\n", args);
        return 0;
    }
    wp->value = result;
    return 0;
}


// 退出
static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}


// 帮助
static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si", "Step Run", cmd_si},
  {"info", "Display informations about reg state or monitor info", cmd_info},
  {"x", "Scans a specified number of memory from a specified location", cmd_x},
  {"p", "Find the value of the expression", cmd_p},
  {"w", "Set watchpoint", cmd_w}

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
