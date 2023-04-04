#include "chip8.h"

#include <SDL2/SDL.h>

struct Platform {
    const char *title;

    unsigned int window_width;
    unsigned int window_height;

    unsigned int texture_width;
    unsigned int texture_height;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} platform;

void initialise_platform(const char *title,
                         unsigned int window_width,
                         unsigned int window_height, 
                         unsigned int texture_width,
                         unsigned int texture_height) {
    platform.title = title;
    platform.window_width = window_width;
    platform.window_height = window_height;
    platform.texture_width = texture_width;
    platform.texture_height = texture_height;

    SDL_Init(SDL_INIT_VIDEO);

    platform.window = SDL_CreateWindow(title, 0, 0, window_width, window_height, SDL_WINDOW_SHOWN);
    platform.renderer = SDL_CreateRenderer(platform.window, -1, SDL_RENDERER_ACCELERATED);
    platform.texture = SDL_CreateTexture(platform.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, texture_width, texture_height);
}

void cleanup_platform() {
    SDL_DestroyTexture(platform.texture);
    SDL_DestroyRenderer(platform.renderer);
    SDL_DestroyWindow(platform.window);

    SDL_Quit();
}

void update(const void *buffer, int pitch) {
    SDL_UpdateTexture(platform.texture, NULL, buffer, pitch);
    SDL_RenderClear(platform.renderer);
    SDL_RenderCopy(platform.renderer, platform.texture, NULL, NULL);
    SDL_RenderPresent(platform.renderer);
}

bool process_input(uint8_t keys) {
    bool quit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;

                    case SDLK_x:
                        state.keys[0] = 1;
                        break;

                    case SDLK_1:
                        state.keys[0x1] = 1;
                        break;

                    case SDLK_2:
                        state.keys[0x2] = 1;
                        break;

                    case SDLK_3:
                        state.keys[0x3] = 1;
                        break;

                    case SDLK_q:
                        state.keys[0x4] = 1;
                        break;

                    case SDLK_w: 
                        state.keys[0x5] = 1;
                        break;

                    case SDLK_e:
                        state.keys[0x6] = 1;
                        break;

                    case SDLK_a:
                        state.keys[0x7] = 1;
                        break;

                    case SDLK_s:
                        state.keys[0x8] = 1;
                        break;

                    case SDLK_d:
                        state.keys[0x9] = 1;
                        break;

                    case SDLK_z:
                        state.keys[0xA] = 1;
                        break;

                    case SDLK_c:
                        state.keys[0xB] = 1;
                        break;

                    case SDLK_4:
                        state.keys[0xC] = 1;
                        break;

                    case SDLK_r:
                        state.keys[0xD] = 1;
                        break;

                    case SDLK_f:
                        state.keys[0xE] = 1;
                        break;

                    case SDLK_v: 
                        state.keys[0xF] = 1;
                        break;

                    default:
                        break;
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;

                    case SDLK_x:
                        state.keys[0] = 0;
                        break;

                    case SDLK_1:
                        state.keys[0x1] = 0;
                        break;

                    case SDLK_2:
                        state.keys[0x2] = 0;
                        break;

                    case SDLK_3:
                        state.keys[0x3] = 0;
                        break;

                    case SDLK_q:
                        state.keys[0x4] = 0;
                        break;

                    case SDLK_w: 
                        state.keys[0x5] = 0;
                        break;

                    case SDLK_e:
                        state.keys[0x6] = 0;
                        break;

                    case SDLK_a:
                        state.keys[0x7] = 0;
                        break;

                    case SDLK_s:
                        state.keys[0x8] = 0;
                        break;

                    case SDLK_d:
                        state.keys[0x9] = 0;
                        break;

                    case SDLK_z:
                        state.keys[0xA] = 0;
                        break;

                    case SDLK_c:
                        state.keys[0xB] = 0;
                        break;

                    case SDLK_4:
                        state.keys[0xC] = 0;
                        break;

                    case SDLK_r:
                        state.keys[0xD] = 0;
                        break;

                    case SDLK_f:
                        state.keys[0xE] = 0;
                        break;

                    case SDLK_v: 
                        state.keys[0xF] = 0;
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        error("Invalid arguments provided to program", true);
    }

    int video_scale = atoi(argv[1]);
    int cycle_delay = atoi(argv[2]);
    const char *filename = argv[3];

    initialise_platform("Chip-8 Interpreter", SCREEN_WIDTH * video_scale, SCREEN_HEIGHT * video_scale, SCREEN_WIDTH, SCREEN_HEIGHT); 
    initialise();
    load_rom(filename);

    int video_pitch = sizeof(state.display[0]) * SCREEN_WIDTH;

    clock_t last_time = clock();
    bool quit = false;

    while (!quit) {
        quit = process_input(state.keypad);

        clock_t current_time = clock();
        clock_t dt = current_time - last_time;

        if (dt > cycle_delay) {
            cycle();
            update();
        }
    }

    return 0;
}
