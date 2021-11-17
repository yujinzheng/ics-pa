#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

typedef struct watchpoint {
    int NO;
    struct watchpoint *next;

    /* TODO: Add more members if necessary */

    int value;
    uint32_t addr;
    char *exp;

} WP;

word_t expr(char *e, bool *success);

WP *new_wp();

WP *get_head();

void free_wp(WP *wp);

#endif
