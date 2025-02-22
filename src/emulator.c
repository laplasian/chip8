#include <stdio.h>
#include "emulator.h"
#include "CPU.h"
#include "SDL2/include/SDL.h"
#include "SDL2_ttf/include/SDL_ttf.h"
#include <dirent.h>
#include "Sound.h"

SDL_Renderer *sdl_initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCALE + 2 * BORDER, SCREEN_HEIGHT * SCALE + 2 * BORDER, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    return renderer;

}

game_sycle* get_list_of_games()
{
    DIR *d;
    struct dirent *dir;
    d = opendir("resources/rom");
    game_sycle* head = malloc(sizeof(game_sycle));
    if (head == NULL) {
        printf("Failed to allocate memory\n");
        exit(1);
    }
    game_sycle* current = head;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (dir->d_name[0] != '.')
            {
                current->game_name = malloc(strlen(dir->d_name) + 1);
                if (current->game_name == NULL) {
                    printf("Failed to allocate memory\n");
                    exit(1);
                }
                strcpy(current->game_name, dir->d_name);
                current->next = malloc(sizeof(game_sycle));
                if (current->next == NULL) {
                    printf("Failed to allocate memory\n");
                    exit(1);
                }
                current = current->next;
            }
        }
        closedir(d);
    }
    game_sycle* current1 = head;
    while (current1->next != current)
    {
        current1 = current1->next;
    }
    current1->next = head;
    return head;
}

void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, int is_selected) {
    SDL_Color color = {255, 255 * (!is_selected), 255 * (!is_selected), 255}; // Белый цвет

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstrect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

char* main_menu(SDL_Renderer *renderer, TTF_Font *font, SDL_Event event, game_sycle* firstgame) {
    int running = 1;

    game_sycle* selected_game = firstgame; // текущая выбранная игра (подчеркивается красным)
    game_sycle* current = firstgame; // текущая первая отображаемая сверху игра

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(1);
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN) {
                    return selected_game->game_name;
                }
                if (event.key.keysym.sym == SDLK_DOWN) { // сложность 1
                    selected_game = selected_game->next; // следующая выбранная игра
                }
                if (event.key.keysym.sym == SDLK_UP) { // сложность O(n)
                    if (current != selected_game)
                    {
                        // проходим круг по списку, пока не дойдем до предыдущей
                        game_sycle* curr = firstgame;
                        while (curr->next != selected_game)
                        {
                            curr = curr->next;
                        }
                        selected_game = curr; // предыдущая выбранная игра
                    }
                }
            }
        }
        // Отрисовка экрана
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        renderText(renderer, font, current->game_name, FONT_SIZE, FONT_SIZE , selected_game == current);
        for (int i = 1; i < (SCREEN_HEIGHT * SCALE - BORDER ) / FONT_SIZE - 1; i++)
        {
            current = current->next;
            renderText(renderer, font, current->game_name, FONT_SIZE, FONT_SIZE + i * 20, selected_game == current);
        }

        // нужно для прокрутки вниз, так как в данный момент current - последняя отрисованная игра
        if (current == selected_game) {
            firstgame = firstgame->next; //
        }
        current = firstgame;

        SDL_RenderPresent(renderer);
        SDL_Delay(100);

    }
    return '\0';
}

// Отрисовка экрана
void chip8_render(Chip8 *chip8, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (chip8->screen[y * 64 + x]) {
                SDL_Rect pixel = { x * SCALE + BORDER, y * SCALE + BORDER, SCALE, SCALE };
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

// Установка значений клавишь
void set_keys(Chip8* chip8, SDL_Event event, uint8_t flag) {
    switch (event.key.keysym.sym) {
        case SDLK_1: chip8->keys[0x1] = flag; break;
        case SDLK_2: chip8->keys[0x2] = flag; break;
        case SDLK_3: chip8->keys[0x3] = flag; break;
        case SDLK_4: chip8->keys[0xC] = flag; break;

        case SDLK_q: chip8->keys[0x4] = flag; break;
        case SDLK_w: chip8->keys[0x5] = flag; break;
        case SDLK_e: chip8->keys[0x6] = flag; break;
        case SDLK_r: chip8->keys[0xD] = flag; break;

        case SDLK_a: chip8->keys[0x7] = flag; break;
        case SDLK_s: chip8->keys[0x8] = flag; break;
        case SDLK_d: chip8->keys[0x9] = flag; break;
        case SDLK_f: chip8->keys[0xE] = flag; break;

        case SDLK_z: chip8->keys[0xA] = flag; break;
        case SDLK_x: chip8->keys[0x0] = flag; break;
        case SDLK_c: chip8->keys[0xB] = flag; break;
        case SDLK_v: chip8->keys[0xF] = flag; break;
    }
}

int run(Chip8* chip8, SDL_Renderer *renderer, SDL_Event event)
{
    int running = 1;
    int local_time = SDL_GetTicks(); // Время  миллесекундах
    int iter_count = 0;

    // Инициализация аудио
    AudioData audio_data = { BASE_FREQUENCY, 0.0 };
    init_sound(&audio_data);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0; // Выход из игры и из программы в целом
            }

            if (event.type == SDL_KEYDOWN) {
                set_keys(chip8, event, 1);
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_CloseAudio(); // Закрытие аудио, так как инициализируем локально при кажном открытии игры
                    return 1; // Возврат в главное меню
                }
            }

            if (event.type == SDL_KEYUP) {
                set_keys(chip8, event, 0);
            }

        }

        // Цикл интерпретации
        iter_count += 1;
        emulateCycle(chip8);

        // Отрисовка экрана
        if (chip8->drawFlag) {
            chip8->drawFlag = 0;
            chip8_render(chip8, renderer);
        }

        // Sound (звучиит лучше с оригинальным FPS = 60)
        if (chip8->sound_timer > 0) {
            audio_data.frequency = 440.0 + chip8->sound_timer * 10.0;
            SDL_PauseAudio(0);
        } else {
            SDL_PauseAudio(1);
        }

        // Delay(Сколько должно пройти - сколько прошло)
        if (1000 / FPS * iter_count - abs((SDL_GetTicks() - local_time)) > 0 && iter_count >= ITER_ACCURACY) {
            SDL_Delay(1000 / FPS * iter_count - abs((SDL_GetTicks() - local_time)));
            iter_count = 0;
            local_time = SDL_GetTicks();
        }
    }
    return 0;
}
