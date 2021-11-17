#include "sdb.h"

#define NR_WP 32

//typedef struct watchpoint {
//    int NO;
//    struct watchpoint *next;
//
//    /* TODO: Add more members if necessary */
//
//    int value;
//    uint32_t addr;
//    char *exp;
//} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

// 从free_链表中返回一个空闲的监视点
WP *new_wp() {
    if (free_ == NULL) {
        printf("Can not find a free WP");
        assert(0);
    }

    WP *new_wp = free_;
    if (free_->next != NULL) {
        free_ = new_wp->next;
    }
    else if (free_->next == NULL) {
        free_ = NULL;
    }

    return new_wp;
}

WP *get_head() {
    return head;
}

// 释放一个监视点到free_链表中
void free_wp(WP *wp) {
    if (free_ == NULL) {
        free_ = wp;
        return;
    }

    if (free_->NO > wp->NO) {
        wp->next = free_;
        return;
    }

    WP *prev_wp = free_;

    // 沿着链表向下遍历，会出现两种情况
    // 情况一、pre_wp->next->NO > wp->NO，这种场景wp放在pre_wp之后，同时wp->next要指向pre_wp->next
    // 情况二、pre_wp->next == NULL，这种场景下将wp放在链表末尾
    while (prev_wp->next != NULL && prev_wp->next->NO < wp->NO);

    if (prev_wp->next == NULL) {
        prev_wp->next = wp;
        return;
    }

    wp->next = prev_wp->next;
    prev_wp->next = wp;
}
