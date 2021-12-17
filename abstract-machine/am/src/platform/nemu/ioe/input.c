#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    uint32_t key_value = *(volatile uint32_t *)KBD_ADDR;
    kbd->keydown = (key_value & 0x8000) >> 15;
    kbd->keycode = (key_value & 0x0fff);
}
