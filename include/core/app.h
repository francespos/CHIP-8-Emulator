#ifndef CORE_APP_H
#define CORE_APP_H

#include <SDL2/SDL.h>
#include <core/chip8.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    SDL_AudioDeviceID audioDeviceID;
    uint8_t *wavBuffer;
    uint32_t wavLenght;
} App;

App *AppInit(int windowWidth, int windowHeight);
void AppDestroy(App *app);

void AppLoop(App *app, CHIP8 *chip8);

#endif