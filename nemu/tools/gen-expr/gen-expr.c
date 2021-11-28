#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static int is_total_expr = 0;
static int expr_num_count = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
        "#include <stdio.h>\n"
        "int main() { "
        "  unsigned result = %s; "
        "  printf(\"%%u\", result); "
        "  return 0; "
        "}";

static void gen_rand_op() {
    switch (rand() % 4) {
        case 0:
            strcat(buf, "+");
            break;
        case 1:
            strcat(buf, "-");
            break;
        case 2:
            strcat(buf, "*");
            break;
        default:
            strcat(buf, "/");
            break;
    }
}

static void gen_rand_expr() {
    char expr[32] = {};
    switch (rand() % 3) {
        case 0:
            gen_rand_expr();
            if (expr_num_count == 0) {
                return;
            }
            gen_rand_op();
            is_total_expr = 0;
            gen_rand_expr();
            if (expr_num_count == 0) {
                return;
            }
            break;
        case 1:
            if (is_total_expr == 1) {
                if (expr_num_count == 0) {
                    return;
                }
                gen_rand_op();
            }
            strcat(buf, "(");
            is_total_expr = 0;
            gen_rand_expr();
            strcat(buf, ")");
            is_total_expr = 1;
            if (expr_num_count == 0) {
                return;
            }
            break;
        default:
            if (is_total_expr == 1) {
                if (expr_num_count == 0) {
                    return;
                }
                gen_rand_op();
            }
            sprintf(expr, "%d", rand() % 100);
            strcat(buf, expr);
            is_total_expr = 1;
            expr_num_count -= 1;
            if (expr_num_count == 0) {
                return;
            }
            break;
    }
}

int main(int argc, char *argv[]) {
    int seed = time(0);
    srand(seed);
    int loop = 1;
    if (argc > 1) {
        sscanf(argv[1], "%d", &loop);
    }
    int i;
    for (i = 0; i < loop; i++) {
        expr_num_count = rand() % 10 + 1;
        memset(buf, '\0', 65536);
        is_total_expr = 0;
        gen_rand_expr();

//        printf("exp is %s\n", buf);

        sprintf(code_buf, code_format, buf);

        FILE *fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        int ret = system("gcc /tmp/.code.c -Wall -Werror -o /tmp/.expr");
        if (ret != 0) continue;

        fp = popen("/tmp/.expr", "r");
        assert(fp != NULL);

        int result;
        fscanf(fp, "%d", &result);
        pclose(fp);

        printf("%u %s\n", result, buf);
    }
//    这段代码可以放到sdb.c文件中，用于测试表达式求值的功能
//    FILE *fp = fopen("/home/yu/learn/ics2021/nemu/tools/gen-expr/input.txt", "r");
//    assert(fp != NULL);
//    char lines[32768];
//    while (fgets(lines, 32768, fp) != NULL) {
//        unsigned len = strlen(lines);
//        lines[len - 1] = '\0';
//        char *result_str = strtok(lines, " ");
//        char *expr_str = strtok(NULL, " ");
//        unsigned result = strtol(result_str, NULL, 10);
//        bool success_check = true;
//        word_t result_check = expr(expr_str, &success_check);
//        printf("==========the expr is %s\n", expr_str);
//        assert(success_check == true);
//        assert(result_check == result);
//    }
//    fclose(fp);
    return 0;
}
