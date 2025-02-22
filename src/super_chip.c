//
// Created by user on 21.01.2025.
//

#include "super_chip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "CPU.h"


/*
    В стадии разработки. Пока не работает.
*/

void init_superChip(Chip8 *chip8) {

    chip8->width = 128;
    chip8->height = 64;
    chip8->superChipmode = 1;

    uint8_t schip_fonts[160] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    memcpy(chip8->memory + 0x100, schip_fonts, sizeof(schip_fonts));

    memset(chip8->screen, 0, 128 * 64 * sizeof(uint8_t));
}

// Поддержка  Super-CHIP
void scrollRight(uint8_t *screen, int width, int height) { // Прокрутка вправо (00FB) на 4 пикселя
    for (int y = 0; y < height; y++) {
        for (int x = width - 1; x >= 4; x--) {
            screen[y * width + x] = screen[y * width + (x - 4)];
        }
        // Очистка 4 левых пикселей в строке
        memset(&screen[y * width], 0, 4);
    }
}

void scrollLeft(uint8_t *screen, int width, int height) { // Прокрутка влево (00FC) на 4 пикселя
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 4; x++) {
            screen[y * width + x] = screen[y * width + (x + 4)];
        }
        // Очистка 4 правых пикселей в строке
        memset(&screen[y * width + (width - 4)], 0, 4);
    }
}

void scrollDown(uint8_t n, uint8_t *screen, int width, int height) {
    // Сдвиг экрана вниз
    memmove(&screen[n * width], &screen[0], (height - n) * width);
    // Очистка верхних n строк
    memset(&screen[0], 0, n * width);
}

void drawSprite(uint8_t *screen, int width, int height, uint8_t *sprite, int x, int y, int spriteHeight) { // Команда отрисовки спрайтов (DXYN) новая
    for (int yline = 0; yline < spriteHeight; yline++) {
        uint8_t pixel = sprite[yline];
        for (int xline = 0; xline < 8; xline++) {
            if ((pixel & (0x80 >> xline)) != 0) {
                if (screen[(x + xline + ((y + yline) * width)) % (width * height)] == 1) {
                    screen[(x + xline + ((y + yline) * width)) % (width * height)] = 0;
                } else {
                    screen[(x + xline + ((y + yline) * width)) % (width * height)] = 1;
                }
            }
        }
    }
}


// 9. Super Chip-48 Instructions

// if ((opcode & 0xFFFF) == 0x00FB) { //0x00FB Прокрутка экрана вправо на 4 пикселя
//     scrollRight(chip8->screen, chip8->width, chip8->height);
//     chip8->drawFlag = 1;
// }
//
// if ((opcode & 0xFFFF) == 0x00FC) { //0x00FC Прокрутка экрана влево на 4 пикселя
//     scrollLeft(chip8->screen, chip8->width, chip8->height);
//     chip8->drawFlag = 1;
// }
//
// if ((opcode & 0xFFF0) == 0x00C0) { //0x00CN Прокрутка экрана вниз на N строк
//     scrollDown(opcode & 0x000F ,chip8->screen, chip8->width, chip8->height);
//     chip8->drawFlag = 1;
// }
//
// // if (opcode == 0x00FD) { //0x00FD Выход из программы
// //     exit(0);
// // }
//
// if ((opcode & 0xFFFF) == 0x00FE) { //0x00FE Отключение режима SuperChip
//     chip8->superChipmode = 0;
//     chip8->width = 64;
//     chip8->height = 32;
// }
//
// if ((opcode & 0xFFFF) == 0x00FF) { //0x00FF Включение режима SuperChip
//     printf("SUPER CHIP ENABLED\n");
//     init_superChip(chip8);
// }