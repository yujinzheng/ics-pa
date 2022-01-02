#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>
#include <assert.h>

void CallbackHelper();

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
    printf("=======SDL_AddTimer\n");
    assert(0);
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
    printf("=======SDL_RemoveTimer\n");
    assert(0);
  return 1;
}

uint32_t SDL_GetTicks() {
    CallbackHelper();
    return NDL_GetTicks();
}

void SDL_Delay(uint32_t ms) {
//    assert(0);
}
