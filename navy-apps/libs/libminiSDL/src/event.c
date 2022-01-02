#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <stdio.h>

#define keyname(k) #k,

static const char *keyname[] = {
        "NONE",
        _KEYS(keyname)
};

# define SDL_NUM_SCANCODES sizeof(keyname)/sizeof(keyname[0])

typedef struct SDL_Keyboard SDL_Keyboard;
struct SDL_Keyboard {
    uint8_t keystate[SDL_NUM_SCANCODES];
};

static SDL_Keyboard SDL_keyboard;

int SDL_PushEvent(SDL_Event *ev) {
    printf("=======SDL_PushEvent\n");
    assert(0);
    return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
    char buf[64];
    int event_result = NDL_PollEvent(buf, sizeof(buf));
    if (event_result == 0) {
        return event_result;
    }
    int key_down, key_code = 0;
    sscanf(buf, "%d %d", &key_down, &key_code);
    if (key_down == 0) {
        ev->type = SDL_KEYUP;
    } else {
//        printf("key_down: %d, key_code: %d\n", key_down, key_code);
        ev->type = SDL_KEYDOWN;
    }
    ev->key.keysym.sym = key_code;
    SDL_keyboard.keystate[key_code] = ev->type;
    CallbackHelper();
    return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
    char buf[64];
    int event_result;
    while (1) {
        CallbackHelper();
        event_result = NDL_PollEvent(buf, sizeof(buf));
        if (event_result == 0) {
            continue;
        }
        int key_down, key_code = 0;
        sscanf(buf, "%d %d", &key_down, &key_code);
        if (key_down == 0) {
            event->type = SDL_KEYUP;
        } else {
//        printf("key_down: %d, key_code: %d\n", key_down, key_code);
            event->type = SDL_KEYDOWN;
        }
        event->key.keysym.sym = key_code;
        SDL_keyboard.keystate[key_code] = event->type;
        return key_code;
    }
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
    printf("=======SDL_PeepEvents\n");
    assert(0);
    return 0;
}

/**
 * 获取键盘状态
 *
 * @param numkeys 该数组是SDLK*_符号的索引，值1表示被按下，0表示没有被按下
 * @return 返回一个指向uint8类型的数组头部的指针，数组的每个元素都对应记录了某个按键是否被按下的标志
 *         返回的指针应该直接指向内部SDL阵列
 */
uint8_t *SDL_GetKeyState(int *numkeys) {
    SDL_Keyboard *keyBorad = &SDL_keyboard;
    if (numkeys != (int *)0) {
        *numkeys = SDL_NUM_SCANCODES;
    }
    CallbackHelper();
    return keyBorad->keystate;
}
