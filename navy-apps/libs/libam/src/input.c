#include <am.h>
#include <klib.h>

#define KEYDOWN_MASK 0x8000
#define KEYCODE_MASK 0x0fff

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    uint32_t key_value = *(volatile uint32_t *)KBD_ADDR;
    kbd->keydown = (key_value & KEYDOWN_MASK) >> 15;
    kbd->keycode = (key_value & KEYCODE_MASK);
}
