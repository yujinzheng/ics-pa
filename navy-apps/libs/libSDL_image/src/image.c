#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"
#include <stdio.h>

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface *IMG_Load(const char *file_path) {
    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        assert(0);
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    char *buf = (char *)malloc(size);
    fseek(fp, 0, SEEK_SET);
    fread(buf, 1, size, fp);
    SDL_Surface *result = STBIMG_LoadFromMemory(buf, size);
    free(buf);
    fclose(fp);
    return result;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
