#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>
#include "../../utils/common_utils.h"

enum {
    TK_NOTYPE = 256,
    TK_EQ = 1,
    TK_NEQ = 2,
    TK_AND = 3,
    TK_HEX = 4,
    TK_REG = 5,
    TK_DEC = 6,
    TK_DEREF = 7,
    TK_NEG = 8,
};

static struct rule {
    const char *regex;
    int token_type;
} rules[] = {
        {" +",            TK_NOTYPE},   // spaces
        {"\\+",           '+'},         // plus
        {"==",            TK_EQ},       // equal
        {"\\-",           '-'},         // minus
        {"\\*",           '*'},         // multiply
        {"/",             '/'},         // divide
        {"\\(",           '('},         // left bracket
        {"\\)",           ')'},         // right bracket
        {"!=",            TK_NEQ},      // not equal
        {"&&",            TK_AND},      // logical and
        {"0x[0-9a-f]+", TK_HEX},      // hexadecimal
        {"\\$[0-9a-z]+",  TK_REG},      // register
        {"[0-9]+",        TK_DEC},      // decimal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}

typedef struct token {
    int type;
    char str[32];
    int priority;
} Token;

#define TOKEN_MAX 65536

static Token tokens[TOKEN_MAX] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    // 清空数组
    memset(tokens, '\0', sizeof(Token) * TOKEN_MAX);

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len, substr_start);

                position += substr_len;

                Token temp_token;
                // 先将字符作为字符串存入到token中
                snprintf(temp_token.str, substr_len + 1, "%s", substr_start);
                switch (rules[i].token_type) {
                    // 如果是空格，则放弃匹配
                    case TK_NOTYPE:
                        break;

                        // 特殊场景一、判断是否是指针的解引用
                        // 解引用的判断条件是：1、*号在第一位；2、*号前面是操作符
                    case '*':
                        temp_token.type = rules[i].token_type;
                        if (nr_token == 0 || (tokens[nr_token - 1].type != TK_HEX &&
                                              tokens[nr_token - 1].type != TK_DEC &&
                                              tokens[nr_token - 1].type != ')')) {
                            temp_token.type = TK_DEREF;
                        }
                        tokens[nr_token] = temp_token;
                        nr_token++;
                        break;

                        // 特殊场景二、判断是否是负号
                        // 负号的判断条件是：1、-号在第一位；2、-号前面是操作符
                    case '-':
                        temp_token.type = rules[i].token_type;
                        if (nr_token == 0 || (tokens[nr_token - 1].type != TK_HEX &&
                                              tokens[nr_token - 1].type != TK_DEC &&
                                              tokens[nr_token - 1].type != ')')) {
                            temp_token.type = TK_NEG;
                        }
                        tokens[nr_token] = temp_token;
                        nr_token++;
                        break;

                        // 特殊场景三、十进制数需要判断长度是否超标
                        // TODO 思考：其他数是否需要考虑长度问题
                    case TK_DEC:
                        // 判断十进制数的长度是否超标
                        if (sizeof(temp_token.str) < substr_len) {
                            Log("the decimal's length %d is over %lu at position %d\n%s\n%*.s^\n",
                                substr_len, strlen(temp_token.str), position, e, position, "");
                            return false;
                        }

                        temp_token.type = rules[i].token_type;
                        tokens[nr_token] = temp_token;
                        nr_token++;
                        break;

                        // 其余场景没有需要特殊处理的地方，所以照常将数据存入到tokens中就行了
                    default:
                        temp_token.type = rules[i].token_type;
                        tokens[nr_token] = temp_token;
                        nr_token++;
                        break;
                }

                break;
            }
        }

        if (i == NR_REGEX) {
            Log("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }

    return true;
}

// 对表达式进行判断，只有当表达式的左右两边刚好被一对括号包围时，返回true
// 如果表达式左右不是一对括号，或者中间有不符合表达式的括号，则返回false
bool check_parentheses(int p, int q, bool *success) {
    *success = true;
    int left_bracket_num = 0;
    // 最简单的场景，只要表达式没有被左右括号包围，那么就一定是false
    if (tokens[p].type != '(' || tokens[q].type != ')') {
        return false;
    }
    int point = p;
    while (point <= q) {
        if (tokens[point].type == '(') {
            left_bracket_num++;
        } else if (tokens[point].type == ')') {
            // 如果在扫描到右括号时，左括号数量已经被抵消，说明括号不配对
            if (left_bracket_num == 0) {
                *success = false;
                Log("The exp is invalid, brackets are malformed: %d", point);
                return false;
            }
            left_bracket_num--;
        }
        point++;

        // 当左括号数量被抵消时，需要检查一下point是否等于q
        // 如果point不等于q，说明最左边的括号在中间就已经被匹配了，左右两边的括号不是一对
        if (left_bracket_num == 0) {
            if (point < q) {
                Log("Left and right parentheses do not match: %d", point);
                return false;
            }
        }
    }

    // 在扫描完成后，堆栈不为空，说明左括号比右括号多，有不是一对的括号，表达式不合法
    if (left_bracket_num != 0) {
        *success = false;
        Log("The exp is invalid, brackets are malformed: %d", point);
        return false;
    }
    Log("Brackets are matched, left %d, right %d", p, q);
    return true;
}

word_t compute_two_value(int val1, int val2, int op, bool *success) {
    switch (tokens[op].str[0]) {
        case '+':
            return val1 + val2;
        case '-':
            return val1 - val2;
        case '*':
            return val1 * val2;
        case '/':
            if (val2 == 0) {
                *success = false;
                Log("The divisor cannot be 0");
                return 0;
            }
            return val1 / val2;
        case '!':
            if (tokens[op].str[1] == '=') {
                return val1 != val2;
            } else {
                *success = false;
                Log("Unknown symbol: %s", tokens[op].str);
                return 0;
            }
        case '&':
            if (tokens[op].str[1] == '&') {
                return val1 && val2;
            } else {
                *success = false;
                Log("Unknown symbol: %s", tokens[op].str);
                return 0;
            }
        case '=':
            if (tokens[op].str[1] == '=') {
                return val1 == val2;
            } else {
                *success = false;
                Log("Unknown symbol: %s", tokens[op].str);
                return 0;
            }
        default:
            *success = false;
            Log("Unknown symbol: %c", tokens[op].str[0]);
            return 0;
    }
}

// 跳过一对闭合括号
int skip_bracket(int position, int q, bool *success) {
    *success = true;
    int left_bracket_num = 0;
    if (tokens[position].type == '(') {
        left_bracket_num++;
        position++;

        while (position <= q) {
            if (tokens[position].type == ')') {
                // 如果在扫描到右括号时，左括号数量已经被抵消，说明括号不配对
                if (left_bracket_num == 0) {
                    *success = false;
                    Log("The exp is invalid, brackets are malformed: %d", position);
                    return 0;
                }
                left_bracket_num--;
            } else if (tokens[position].type == '(') {
                left_bracket_num++;
            }
            position++;

            // 如果经过上面的操作后，左括号数量被抵消，说明完成一对闭合括号的配对，跳出循环
            // 之所以要在position++之后再跳出，是因为position必须指向下一个数
            if (left_bracket_num == 0) {
                break;
            }
        }
        // 如果发现扫描完表达式后，堆栈没有清空，说明表达式无效
        if (left_bracket_num != 0) {
            *success = false;
            Log("The exp is invalid, brackets are malformed");
            return 0;
        }
    }
    Log("Try to skip bracket, the position is %d", position);
    return position;
}

word_t eval(int p, int q, bool *success) {
    *success = true;
    if (p > q) {
        *success = false;
        Log("Invalid express");
        return 0;
    } else if (p == q) {
        // 这种情况下应该只有一个数，因此这个数需要返回
        return strtol(tokens[p].str, NULL, 10);
    } else if (check_parentheses(p, q, success) == true) {
        if (*success == false) {
            return 0;
        }
        return eval(p + 1, q - 1, success);
    } else {
        // 找到表达式的主运算符
        // 主运算符满足如下几个条件：
        // 1、一定不在括号内，因为整个表达式必然没有被包含在括号里面（由前面的括号检查来保证）
        // 2、一定是不在括号内的优先级最低的操作符（priority越小，优先级越高）
        // 3、如果有多个满足条件1、2的操作符，那么选取最后一个操作符
        int op_priority = 0;
        int op = p;
        int position = p;
        while (position <= q) {
            // 如果操作符在括号内部，直接过滤掉
            position = skip_bracket(position, q, success);
            if (*success == false) {
                return 0;
            }
            // 考虑表达式最后一位是括号，这个时候返回的position会比q大1
            if (position == q + 1) {
                continue;
            }
            if (tokens[position].priority == 0) {
                position++;
                continue;
            }

            if (tokens[position].priority >= op_priority) {
                op = position;
                op_priority = tokens[op].priority;
                Log("Find a operator: %s, the position is %d", tokens[op].str, position);
            }
            position++;
        }

        if (op_priority <= 0) {
            *success = false;
            Log("Unable to find a suitable operator, the exp is invalid");
            return 0;
        }

        int val1 = eval(p, op - 1, success);
        if (success == false) {
            Log("Can not get val1 from express, the p is: %d, q is %d", p, op - 1);
            return 0;
        }
        int val2 = eval(op + 1, q, success);
        if (success == false) {
            Log("Can not get val2 from express, the p is: %d, q is %d", op + 1, q);
            return 0;
        }
        Log("Start to compute value, val1 = %d, val2 = %d, operator is %s, start is %d, end is %d", val1, val2,
            tokens[op].str, p, q);
        return compute_two_value(val1, val2, op, success);
    }
}

// 对tokens进行扫描并格式化，得到一个消去了负号和解引用负号的数组
void scan_tokens(Token *exp_tokens, bool *success) {
    int length = 0;
    *success = true;
    for (int idx = 0; idx < TOKEN_MAX; idx++) {
        if (tokens[idx].str[0] == '\0') {
            exp_tokens[length] = tokens[idx];
            return;
        }

        // 检查到如果是负号，这时有三种情况
        // 情况一、后面是常数或十六进制数，那么直接就设置为负数
        // 情况二、后面是解引用，那么需要再向后扫描到地址位置，然后再将结果计算出来
        if (tokens[idx].type == TK_NEG) {
            int offset;
            if (tokens[idx + 1].type == TK_DEC) {
                // 如果是十进制的数，则只需要向后offset 1位
                offset = 1;
                int token_num = (int) strtol(tokens[idx + offset].str, NULL, 10);
                sprintf(tokens[idx + offset].str, "%d", 0 - token_num);
            } else if (tokens[idx + 1].type == TK_HEX) {
                // 如果是十六进制的数，则只需要向后offset 1位
                offset = 1;
                int token_num = (int) strtol(tokens[idx + offset].str, NULL, 16);
                sprintf(tokens[idx + offset].str, "%d", 0 - token_num);
            } else if (tokens[idx + 1].type == TK_DEREF && tokens[idx + 2].type == TK_HEX) {
                // 如果后面接的是解引用符号，则只需要向后offset 1位
                offset = 2;
                vaddr_t addr = (int) strtol(tokens[idx + offset].str, NULL, 16);
                int token_num = vaddr_read(addr, 4);
                sprintf(tokens[idx + offset].str, "%d", 0 - token_num);
            } else {
                Log("Invalid exp, can not parse negative number, position is %d", idx);
                *success = false;
                return;
            }

            // 这里的索引需要加一个偏置数，因为在转换token的时候，负号和解引用负号不应该算在长度里面
            // 相当于有的位置是无效的，idx为了跟上数据，所以需要偏置一下
            idx += offset;
        } else if (tokens[idx].type == TK_DEREF) {
            // 对解引用进行判断，解引用只有一种场景，那就是下一个数必须为十六进制的数，否则就报错
            int offset = 1;
            if (tokens[idx + offset].type == TK_HEX) {
                vaddr_t addr = (int) strtol(tokens[idx + offset].str, NULL, 16);
                int token_num = vaddr_read(addr, 4);
                sprintf(tokens[idx + offset].str, "%d", token_num);
            } else {
                Log("Invalid exp, can not parse dereference address, position is %d", idx);
                *success = false;
                return;
            }
            idx += offset;
        } else if (tokens[idx].type == TK_REG) {
            // 如果是获取寄存器的值，也将其进行解析
            char *name = tokens[idx].str;
            // 在寄存器那边不要做特殊适配，在调用函数之前就先把非$0寄存器前的$去掉
            if (strcmp(name, "$0") != 0) {
                remove_one_char(name, '$');
            }
            int token_num = isa_reg_str2val(name, success);
            if (*success == false) {
                return;
            }
            sprintf(tokens[idx].str, "%d", token_num);
        }
        exp_tokens[length] = tokens[idx];
        length++;
    }
}

word_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    // 对tokens进行一次扫描，将负号、解引用负号都消去，只保留数字结果
    Token exp_tokens[TOKEN_MAX];

    scan_tokens(exp_tokens, success);
    if (*success == false) {
        return 0;
    }

    // tokens是静态变量，在规整之后，还是要将数据写入到tokens中
    int length = 0;
    while (exp_tokens[length].str[0] != '\0') {
        tokens[length] = exp_tokens[length];
        if (tokens[length].type == '+' || tokens[length].type == '-') {
            tokens[length].priority = 4;
        } else if (tokens[length].type == '*' || tokens[length].type == '/') {
            tokens[length].priority = 3;
        } else if (tokens[length].type == TK_EQ || tokens[length].type == TK_NEQ) {
            tokens[length].priority = 7;
        } else if (tokens[length].type == TK_AND) {
            tokens[length].priority = 11;
        } else {
            // 其余情况优先级都设置为0
            tokens[length].priority = 0;
        }
        length++;
    }

    tokens[length].str[0] = '\0';

    return (word_t)eval(0, length - 1, success);
}
