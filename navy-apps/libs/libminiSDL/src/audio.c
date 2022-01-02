#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <stdio.h>

uint32_t current_time;
uint32_t callback_mtime;
uint32_t samples;
int pause_flag = 1;
int callback_flag = 0;
SDL_AudioSpec audioSpec;
void CallbackHelper() {
//    if (pause_flag == 0 && callback_flag == 0 && NDL_GetTicks() - current_time >= callback_mtime) {
//        callback_flag = 1;
//        int free_size = NDL_QueryAudio();
//        int stream_len = samples <= free_size ? samples : free_size;
//        printf("=======CallbackHelper, skip time: %d, flag: %d\n", NDL_GetTicks() - current_time, callback_flag);
//        current_time = NDL_GetTicks();
//        uint8_t *stream = (uint8_t *)malloc(stream_len);
//        audioSpec.callback(NULL, stream, stream_len);
//        NDL_PlayAudio(stream, stream_len);
//        free(stream);
//    }
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
//    NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
//    samples = desired->samples;
//    current_time = NDL_GetTicks();
//    callback_mtime = desired->samples * 1000 / desired->freq;
////    printf("=======SDL_OpenAudio, current_time: %d, callback_mtime: %d\n", current_time, callback_mtime);
//    pause_flag = 0;
//    audioSpec = *desired;
    return 0;
}

void SDL_CloseAudio() {
    printf("=======SDL_CloseAudio\n");
    NDL_CloseAudio();
    pause_flag = 0;
}

void SDL_PauseAudio(int pause_on) {
    printf("=======SDL_PauseAudio\t pause: %d\n", pause_on);
    pause_flag = pause_on;
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
    printf("=======SDL_MixAudio\n");
//    assert(0);
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
    printf("=======SDL_LoadWAV\n");
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
    printf("=======SDL_FreeWAV\n");
//    assert(0);
}

void SDL_LockAudio() {
//    printf("=======SDL_LockAudio\n");
//    assert(0);
}

void SDL_UnlockAudio() {
//    printf("=======SDL_UnlockAudio\n");
//    assert(0);
}
