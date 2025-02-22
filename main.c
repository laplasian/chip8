#include <stdio.h>
#include <stdlib.h>
#include "src/CPU.h"
#include "src/emulator.h"
#include "src/Sound.h"
#include "src/SDL2_ttf/include/SDL_ttf.h"

char* concat(char* s1, char* s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);

    char* new_s1 = malloc(len1);
    if (new_s1 == NULL) {
        printf("Failed to allocate memory\n");
        exit(1);
    }
    memcpy(new_s1, s1, len1);
    new_s1 = realloc(new_s1, len1 + len2 + 1);
    if (new_s1 == NULL) {
        printf("Failed to allocate memory\n");
        exit(1);
    }
    memcpy(new_s1 + len1, s2, len2 + 1);

    return new_s1;
}

#undef main
int main(void)
{
    game_sycle* firstgame = get_list_of_games();

    Chip8 *chip8 = malloc(sizeof(Chip8));
    if (!chip8) {
        printf("Failed to allocate memory\n");
        return 1;
    }

    SDL_Renderer *renderer = sdl_initialize();
    SDL_Event event;
    TTF_Font *font = TTF_OpenFont("resources/JosefinSans-Regular.ttf", FONT_SIZE);

    int running = 1;

    char* game_name;
    char* game_path = "resources/rom/";

     while (running) {
         game_name = main_menu(renderer, font, event, firstgame);
         init_cpu(chip8);
         loadGame(chip8, concat(game_path, game_name));
         running = run(chip8,renderer,event);
     }

    SDL_Quit();
    TTF_Quit();
    return 0;
}