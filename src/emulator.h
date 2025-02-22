#ifndef EMULATOR_H
#define EMULATOR_H

#include "emulator.h"
#include "CPU.h"
#include "SDL2/include/SDL.h"
#include "SDL2_ttf/include/SDL_ttf.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 10
#define BORDER 10
#define FONT_SIZE 20

#define FPS 120
#define ITER_ACCURACY 3

// Связаный список из названий игр отсортированный по алфавиту
typedef struct game_sycle{
    char* game_name;
    struct game_sycle* next;
} game_sycle;

// Окно начального экрана, с выбором игры (скролинг по списку)
char* main_menu(SDL_Renderer *renderer, TTF_Font *font, SDL_Event event, game_sycle* firstgame);
// Возвращает связанный список из названий игр
game_sycle* get_list_of_games();
// Инициализация SDL
SDL_Renderer *sdl_initialize();
// Цикл игры
int run(Chip8* chip8, SDL_Renderer *renderer, SDL_Event event);

#endif //EMULATOR_H
