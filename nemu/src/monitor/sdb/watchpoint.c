#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

// 初始化监控点的链表，返回头结点
void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    }

    head = (WP *)malloc(sizeof(struct watchpoint));
    if (head == NULL) {
        Log("Init wp pool failed, can't create head");
        return;
    }

    free_ = (WP *)malloc(sizeof(struct watchpoint));
    if (free_ == NULL) {
        Log("Init wp pool failed, can't create free");
        return;
    }
    head->next = NULL;
    free_->next = wp_pool;
}

WP *new_wp() {
    if (head == NULL) {
        init_wp_pool();
        if (head == NULL) {
            return NULL;
        }
    }
    if (free_->next == NULL) {
        Log("There is no free wp");
        return NULL;
    }

    // free.next指向新获得的wp的next
    // 新获得的wp.next指向head.next
    // head.next指向新获得的wp
    // 类似于栈的效果，head.next永远指向最新wp，wp按照顺序依次向下指定
    WP *temp_wp = free_->next;
    free_->next = temp_wp->next;
    temp_wp->next = head->next;
    head->next = temp_wp;
    return head;
}

void free_wp(WP *wp) {
    // 这里的操作实际上就是从head指向的链表中将wp去掉
    WP *temp = head->next;
    if (temp == NULL) {
        Log("There is no wp can be free");
        return;
    }
    if (temp->NO == wp->NO) {
        head->next = temp->next;
    } else {
        while (temp->next != NULL) {
            if (temp->next->NO == wp->NO) {
                temp->next = wp->next;
                break;
            }
            temp = temp->next;
        }
    }

    // 被释放的wp一定要修改wp.next
    if (free_->next == NULL) {
        free_->next = wp;
        wp->next = NULL;
        return;
    }

    // 如果还有空闲的wp，则修改free指向的链表，将释放的wp放到这个链表中
    wp->next = free_->next;
    free_->next = wp;
}

WP *get_head() {
    return head;
}