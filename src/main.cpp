#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "chip8.h"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 640;

const uint32_t onColor = 0xFFFFFFFF;
const uint32_t offColor = 0x00000000;

void *LoadEntireBinaryFile(const char *fname, int *size);
void CopyDisplayToTexture(CHIP8 &vm, SDL_Texture *tex);
int KeyMap(Uint32 sdlKey);

int main(int argc, char *argv[])
{
    //
    // Load ROM and Initialize CHIP-8
    //

    if (argc != 2)
    {
        fprintf(stderr, "Usage:\n%s <rom_file>\n", argv[0]);
        exit(1);
    }

    const char *romFile = argv[1];
    int romSize = 0;
    uint8_t *romData = (uint8_t *)LoadEntireBinaryFile(romFile, &romSize);
    if (romData == NULL)
    {
        fprintf(stderr, "ERROR: Failed to load ROM\n");
        exit(1);
    }

    CHIP8 vm;
    vm.LoadROM(romData, romSize);

    //
    // Setup SDL
    //

    SDL_Window *window;
    SDL_Renderer *renderer;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Texture *c8Screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, CHIP8::DISPLAY_WIDTH, CHIP8::DISPLAY_HEIGHT);
    if (c8Screen == NULL)
    {
        printf("Screen texture could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    //
    // Main loop
    //

    int lastFrameTime = SDL_GetTicks();
    float dt = 0.0f;

    SDL_Event e;
    bool quit = false;
    while (!quit)
    {
        /* Delta Time */
        int thisFrameTime = SDL_GetTicks();
        dt = (thisFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = thisFrameTime;

        /* Input Events */
        while (SDL_PollEvent(&e) != 0)
        {
            // User requests quit
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN)
            {
                int key = KeyMap(e.key.keysym.scancode);
                if (key != -1 && !e.key.repeat)
                    vm.KeyPressed(key);
            }
            else if (e.type == SDL_KEYUP)
            {
                int key = KeyMap(e.key.keysym.scancode);
                if (key != -1)
                    vm.KeyReleased(key);
            }
        }

        /* Update */
        vm.Advance(dt);

        /* Render */
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        // Copy screen from vm to c8Screen texture
        CopyDisplayToTexture(vm, c8Screen);

        // Render the screen to the window
        SDL_Rect dstRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_Rect srcRect = {0, 0, 64, 32};
        SDL_RenderCopy(renderer, c8Screen, &srcRect, &dstRect);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    // Destroy window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void *LoadEntireBinaryFile(const char *fname, int *size)
{
    FILE *fin;

    fin = fopen(fname, "rb");
    if (fin == NULL)
    {
        perror(fname);
        *size = 0;
        return NULL;
    }

    fseek(fin, 0, SEEK_END);
    *size = ftell(fin);
    rewind(fin);

    void *data = malloc(*size);
    fread(data, 1, *size, fin);

    fclose(fin);

    return data;
}

void CopyDisplayToTexture(CHIP8 &vm, SDL_Texture *tex)
{
    uint8_t *pixels = NULL;
    int pitch = 0;
    if (SDL_LockTexture(tex, NULL, (void **)&pixels, &pitch))
    {
        printf("Unable to lock texture! %s\n", SDL_GetError());
        return;
    }

    for (int y = 0; y < CHIP8::DISPLAY_HEIGHT; ++y)
    {
        for (int x = 0; x < CHIP8::DISPLAY_WIDTH; ++x)
        {
            uint32_t *pixel = (uint32_t *)(pixels + (y * pitch) + (x * 4));
            *pixel = vm.GetPixel(x, y) ? onColor : offColor;
        }
    }

    SDL_UnlockTexture(tex);
}

int KeyMap(Uint32 sdlKey)
{
    switch (sdlKey)
    {
    case SDL_SCANCODE_KP_PERIOD:
        return 0;
    case SDL_SCANCODE_KP_7:
        return 1;
    case SDL_SCANCODE_KP_8:
        return 2;
    case SDL_SCANCODE_KP_9:
        return 3;
    case SDL_SCANCODE_KP_4:
        return 4;
    case SDL_SCANCODE_KP_5:
        return 5;
    case SDL_SCANCODE_KP_6:
        return 6;
    case SDL_SCANCODE_KP_1:
        return 7;
    case SDL_SCANCODE_KP_2:
        return 8;
    case SDL_SCANCODE_KP_3:
        return 9;
    case SDL_SCANCODE_KP_0:
        return 10;
    case SDL_SCANCODE_KP_ENTER:
        return 11;
    case SDL_SCANCODE_KP_DIVIDE:
        return 12;
    case SDL_SCANCODE_KP_MULTIPLY:
        return 13;
    case SDL_SCANCODE_KP_MINUS:
        return 14;
    case SDL_SCANCODE_KP_PLUS:
        return 15;
    default:
        return -1;
    }
}