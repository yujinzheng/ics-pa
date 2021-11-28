#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <regex.h>
#include <memory/vaddr.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
WP *new_wp();
WP *get_head();
void free_wp(WP *wp);

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

// 判断参数个数是否超标，同时返回获取到的参数
static  char** check_cmd_args(int arg_num, char *args, char *split, int need_check, char **result) {
    int count = 0;
    do {
        char *next_arg;
        if (count == 0) {
            next_arg = strtok(args, split);
        } else {
            next_arg = strtok(NULL, split);
        }
        result[count] = next_arg;
        count++;
        if (next_arg == NULL) {
            printf("args miss: %d arg\n", count);
            result[arg_num] = split;
            return result;
        }
    } while (count < arg_num);

    if (need_check == 1) {
        char *last_arg = strtok(NULL, split);
        if (last_arg != NULL) {
            printf("too many args\n");
            result[arg_num] = split;
        }
    }

    return result;
}

// 执行正则表达式的比较
static int regx_check(const char *pattern, char *arg) {
    int flag = REG_EXTENDED;
    regmatch_t pmatch[1];
    const size_t nmatch = 1;
    regex_t reg;

    // 编译正则表达式
    regcomp(&reg, pattern, flag);

    // 执行正则表达式和缓存的比较
    int status = regexec(&reg, arg, nmatch, pmatch, 0);
    regfree(&reg);
    if (status == REG_NOMATCH) {
        printf("invalid arg: %s\n", arg);
        return 0;
    }
    return 1;
}

// 继续运行程序
static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

// 退出程序
static int cmd_q(char *args) {
  return -1;
}

// 单步执行
static int cmd_si(char *args) {
    int steps;
    if (args == NULL) {
        steps = 1;
    } else {
        char *p_arr[2];
        char *split = " ";
        p_arr[1] = 0;
        check_cmd_args(1, args, split, 1, p_arr);
        if (p_arr[1] == split) {
            return 0;
        }
        char *arg = p_arr[0];

        // 用于判断参数的每一位是否都是数字
        // 这里也可以用正则表达式来优化
        int step_result = regx_check("[0-9]+", arg);
        if (step_result != 1) {
            return 0;
        }

        steps = (int)strtol(arg, NULL, 10);
    }
    cpu_exec(steps);
    return 0;
}

// 打印程序状态
static int cmd_info(char *args) {
    if (args == NULL) {
        printf("please input args!\n");
        return 0;
    } else {
        char *p_arr[2];
        char *split = " ";
        p_arr[1] = 0;
        check_cmd_args(1, args, split, 1, p_arr);
        if (p_arr[1] == split) {
            return 0;
        }

        char *arg = p_arr[0];
        WP *head = get_head();
        switch (*arg) {
            // info r表示打印寄存器
            case 'r': isa_reg_display(); break;
                // info w表示打印监视点
            case 'w':
                if (head == NULL || head->next == NULL) {
                    printf("No watchpoint!\n");
                    return 0;
                }
                printf("Number\t\tExpression\t\tOld value(DEC)\t\tOld value(HEX)\n");
                WP *temp_wp = head->next;
                while (temp_wp != NULL) {
                    printf("%d\t\t%s\t\t\t%d\t\t\t0x%x\n", temp_wp->NO, temp_wp->expression, temp_wp->value, temp_wp->value);
                    temp_wp = temp_wp->next;
                }
                break;
            default: printf("invalid args!\n"); break;
        }
    }
    return 0;
}

// 扫描内存信息
static int cmd_x(char *args) {
    if (args == NULL) {
        printf("please input args!\n");
        return 0;
    } else {
        char *p_arr[3];
        char *split = " ";
        p_arr[2] = 0;
        check_cmd_args(2, args, split, 1, p_arr);
        if (p_arr[2] == split) {
            return 0;
        }

        // 检查是否缺少参数
        char *first_arg = p_arr[0];
        char *second_arg = p_arr[1];

        // 检查需要扫描的内存数，如果输入不为数字，则报错处理
        int step_result = regx_check("[0-9]+", first_arg);
        if (step_result != 1) {
            return 0;
        }
        int steps = (int)strtol(first_arg, NULL, 10);

        // 检查输入的表达式的格式
        const char *hex_pattern = "^0x[0-9a-f]{8}";
        const char *pc_pattern = "^\\$pc";
        int hex_result = regx_check(hex_pattern, second_arg);
        int reg_result = regx_check(pc_pattern, second_arg);
        uint32_t addr = cpu.pc;

        if (hex_result == 1) {
            addr = (uint32_t)strtol(second_arg, &second_arg, 16);
        }

        // 如果两个正则都没有匹配上，说明无法正常获得地址
        if (hex_pattern ==0 && reg_result == 0) {
            printf("Can not get address of arg: %s", second_arg);
            return 0;
        }

        // 准备开始打印扫描结果了，先打印表头
        printf("%s\t\t%s\t\t%s\n", "addr", "16进制", "10进制");

        for (int idx = 0; idx < steps; ++idx) {
            // 第一列打印地址
            printf("0x%08x:\t", addr + 4 * idx);
            // 第二列打印十六进制的值
            word_t addr_value = vaddr_read(addr + 4 * idx, 4);
            printf("0x%08x\t", addr_value);
            // 第三列打印十进制的值
            printf("%d\n", addr_value);
        }
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
        printf("the express %s result is: %d\t0x%x\n", args, result, result);
    }
    return 0;
}

// 设置监视点
static int cmd_w(char *args) {
    if (args == NULL) {
        printf("args miss: please input expression\n");
        return 0;
    } else {
        bool success = true;
        word_t result = expr(args, &success);
        if (success == false) {
            printf("Can not get result of watch point express %s\n", args);
            return 0;
        }
        WP *head = new_wp();
        if (head == NULL) {
            return 0;
        }
        head->next->value = result;
        memcpy(head->next->expression, args, 128);
        printf("Set watchpoint #%d\n", head->next->NO);
        printf("Expression is: %s\n", head->next->expression);
        printf("Old value = %u\n", head->next->value);
    }
    return 0;
}

// 删除监视点
static int cmd_d(char *args) {
    if (args == NULL) {
        printf("please input args!\n");
        return 0;
    } else {
        char *p_arr[2];
        char *split = " ";
        p_arr[1] = 0;
        check_cmd_args(1, args, split, 1, p_arr);
        if (p_arr[1] == split) {
            return 0;
        }
        char *arg = p_arr[0];
        int step_result = regx_check("[0-9]+", arg);
        if (step_result != 1) {
            return 0;
        }
        int no = (int)strtol(arg, NULL, 10);
        WP *head = get_head();
        if (head == NULL) {
            Log("Can't delete watchpoint %d, head is NULL", no);
            return 0;
        }
        WP *temp_wp = head->next;
        while (temp_wp != NULL) {
            if (temp_wp->NO == no) {
                free_wp(temp_wp);
                printf("Success delete #%d\n", no);
                return 0;
            }
            temp_wp = temp_wp->next;
        }
        if (temp_wp == NULL) {
            Log("Can't delete #%d, maybe it is free", no);
        }
    }
    return 0;
}

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
  {"w", "Find the value of the expression", cmd_w},
  {"d", "Find the value of the expression", cmd_d},

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
