#include <stdio.h>

uint32_t NDL_GetTicks();

int main() {
    uint32_t current_time = NDL_GetTicks();
    while (1) {
        if (NDL_GetTicks() - current_time >= 500) {
            printf("=====NDL_GetTicks: %u, current_time: %u\n", NDL_GetTicks(), current_time);
            current_time = NDL_GetTicks();
        }
    }
    return 0;
}
