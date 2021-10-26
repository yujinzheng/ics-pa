#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <ctype.h>

enum {
    TK_NOTYPE = 256, TK_EQ, TK_DECIMAL = -1,

    /* TODO: Add more token types */

};

static struct rule {
    const char *regex;
    int token_type;
} rules[] = {

        /* TODO: Add more rules.
         * Pay attention to the precedence level of different rules.
         */

        {" +",   TK_NOTYPE},    // spaces
        {"\\+",  1},         // plus
        {"==",   TK_EQ},        // equal
        {"\\-",  1},        // reduce
        {"\\*",  2},         // take
        {"/",    2},           // remove
        {"\\(",  3},         // 左括号
        {"\\)",  3},         // 右括号
        {"[0-9]+", TK_DECIMAL},         // 十进制整数
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
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    // 清空数组
    memset(tokens, '\0', sizeof(Token)*31);

    nr_token = 0;

    int is_negative = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len, substr_start);

                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                Token tokenInfo;
                tokenInfo.type = rules[i].token_type;
                switch (rules[i].token_type) {
                    // 如果是空格，则不做处理
                    case TK_NOTYPE:
                        break;
                    // 如果是十进制整数，则将整数的字符串存入tokens中
                    case TK_DECIMAL:
                        if (sizeof(tokenInfo.str) < substr_len) {
                            printf("the decimal's length %d is over %lu at position %d\n%s\n%*.s^\n",
                                   substr_len, strlen(tokenInfo.str), position, e, position, "");
                            return false;
                        }
                        snprintf(tokenInfo.str, substr_len + 1, "%s", substr_start);
                        if (is_negative == 1) {
                            char minus[32] = {"-"};
                            strcat(minus, tokenInfo.str);
                            strcpy(tokenInfo.str, minus);
                            is_negative = 0;
                        }
                        tokens[nr_token] = tokenInfo;
                        nr_token++;
                        break;

                    // 如果是减号，需要判断是否是负数
                    case 1:
                        // 如果负号在首位，或者前一位token是操作符，那么说明后面跟的数应该为负数
                        snprintf(tokenInfo.str, substr_len + 1, "%s", substr_start);
                        if (tokenInfo.str[0] == '-') {
                            if (nr_token == 0 || tokens[nr_token - 1].type > 0) {
                                is_negative = 1;
                                break;
                            }
                        }
                        tokens[nr_token] = tokenInfo;
                        nr_token++;
                        break;
                    // 其余场景，都是直接将操作数放入到tokens中
                    // 因为前面有正则匹配，所以这里就不用再校验数据是否合法了
                    default:
                        snprintf(tokenInfo.str, substr_len + 1, "%s", substr_start);
                        tokens[nr_token] = tokenInfo;
                        nr_token++;
                        break;
                }
                break;
            }
        }
        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }
    return true;
}

/**
 * 检查表达式是否被括号包围
 * @param p
 * @param q
 * @return
 */
bool check_parentheses(int p, int q) {
    // 最简单的情况，检查左右是否被括号包围，如果不满足，直接返回false
    if (tokens[p].str[0] != '(' || tokens[q].str[0] != ')') {
        return false;
    }

    // 检查虽然被左右括号包围，但是括号不属于同一对的情况
    int brackets = 0;
    for (int idx = p + 1; idx < q; idx++) {
        if (tokens[idx].str[0] == '(') {
            brackets++;
        }
        if (tokens[idx].str[0] == ')') {
            brackets--;
        }

        // 从左向右扫描，正常情况下一定是先扫描到左括号，因此，一旦brackets小于0，说明括号一定不配对
        if (brackets < 0) {
            return false;
        }
    }

    // 当所有扫描都完成后，brackets应该为0
    if (brackets != 0) {
        return false;
    }
    return true;
}

word_t eval(int p, int q, bool *success) {
    if (p > q) {
        *success = false;
        return 0;
    } else if (p == q) {
        // 判断除了负号以外的第一个char是否为数字
        // 因为前面有正则约束
        int digit_start = 0;
        if (tokens[p].str[0] == '-') {
            digit_start++;
        }
        if (!isdigit(*(tokens[p].str + digit_start))) {
            *success = false;
            printf("the input is invalid, position: %d\n", p);
            return 0;
        }
        char *ptr;
        return strtol(tokens[p].str, &ptr, 10);
    } else if (check_parentheses(p, q) == true) {
        return eval(p + 1, q - 1, success);
    } else {
        int op = p;
        int op_type = 3;
        int temp_position = p;

        // 括号里面的操作符的权重一定大于括号外的操作符，因此直接过滤掉
        while (temp_position < q) {
            int brackets = 0;
            if (tokens[temp_position].str[0] == '(') {
                brackets++;
                temp_position++;
                while (brackets > 0) {
                    if (tokens[temp_position].str[0] == '(') {
                        brackets++;
                        temp_position++;
                        continue;
                    }
                    if (tokens[temp_position].str[0] == ')') {
                        brackets--;
                        if (brackets < 0) {
                            *success = false;
                            printf("invalid express");
                            return 0;
                        }

                        // 如果左右括号已经完全匹配上了，则直接跳出寻找
                        if (brackets == 0) {
                            break;
                        }
                        temp_position++;
                        continue;
                    }

                    // while退出的保险丝，同时防止下标溢出，当temp_position超出下标时，直接报错
                    if (temp_position > q) {
                        *success = false;
                        printf("Can not find middle operation\n");
                        return 0;
                    }
                    temp_position++;
                }
            }

            // 过滤掉操作数，必须放在括号检查之后，防止最后一个token是操作数
            if (tokens[temp_position].type < 0) {
                temp_position++;
                continue;
            }

            // 如果操作符的优先级小于当前操作符优先级，则更新当前操作符
            if (tokens[temp_position].type < op_type) {
                op = temp_position;
                op_type = tokens[temp_position].type;
            }
            temp_position++;
        }

        int val1 = eval(p, op - 1, success);
        int val2 = eval(op + 1, q, success);

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
                    printf("The divisor cannot be 0\n");
                    return 0;
                }
                return val1 / val2;
            default:
                *success = false;
                printf("Unknown symbol: %c\n", tokens[op].str[0]);
                return 0;
        }
    }
}

word_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }
    /* TODO: Insert codes to evaluate the expression. */
    int length = 0;
    while (tokens[length].str[0] != '\0') {
        length++;
    }
    return eval(0, length - 1, success);
}