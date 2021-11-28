#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

word_t expr(char *e, bool *success);

typedef struct watchpoint {
    int NO;
    struct watchpoint *next;
    int value;
    char expression[128];
    /* TODO: Add more members if necessary */

} WP;

#endif
