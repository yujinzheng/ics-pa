#include <NDL.h>

int SDL_Init(uint32_t flags) {
  return NDL_Init(flags);
}

void SDL_Quit() {
  NDL_Quit();
}

char *SDL_GetError() {
    printf("=======SDL_GetError\n");
    assert(0);
  return "Navy does not support SDL_GetError()";
}

int SDL_SetError(const char* fmt, ...) {
    printf("=======SDL_SetError\n");
    assert(0);
  return -1;
}

int SDL_ShowCursor(int toggle) {
    printf("=======SDL_ShowCursor\n");
    assert(0);
  return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon) {
    printf("=======SDL_WM_SetCaption\n");
    assert(0);
}
