//
// Created by yu on 2021/11/28.
//
#include "common_utils.h"

void remove_one_char(char *str, char target) {
    int i, j;
    for (i = j = 0; str[i] != '\0'; i++) {
        if (str[i] != target) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}