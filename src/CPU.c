#include "CPU.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void init_cpu(Chip8 *chip8) {
    chip8->pc = 0x200;        // Начальный адрес программы
    chip8->I = 0;
    chip8->sp = 0;

    // Сначала обычный режим
    chip8->width = 64;
    chip8->height = 32;
    chip8->superChipmode = 0;

    // Очистка памяти, экранного буфера, клавиатуры
    for (int i = 0; i < 4096; i++) chip8->memory[i] = 0;
    for (int i = 0; i < 64 * 32; i++) chip8->screen[i] = 0;
    for (int i = 0; i < 16; i++) chip8->V[i] = chip8->keys[i] = chip8->stack[i] = 0;

    // Таймеры
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    // Загрузка шрифтов (0-F) в память
    uint8_t fontset[80] = {
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
        0xF0, 0x80, 0xF0, 0x80, 0x80,  // F
    };
    memcpy(chip8->memory + 0x50, fontset, 80 * sizeof(uint8_t));
}

void loadGame(Chip8 *chip8, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Cant open: %s\n", filename);
        exit(1);
    }
    // Чтение данных в память с адреса 0x200
    fread(&chip8->memory[0x200], sizeof(uint16_t), 4096 - 0x200, file);
    fclose(file);
}

void emulateCycle(Chip8 *chip8) {
    // ШАГ 1: Считывание команды
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1]; // Считывание команды (2 байта)
    chip8->pc += 2;

    // ШАГ 2: Распознавание и выполнение команды
    // Системные команды
    if (opcode == 0x00E0) { // Очистка экрана
        for (int i = 0; i < 64 * 32; i++) chip8->screen[i] = 0;
        chip8->drawFlag = 1;
    } else if (opcode == 0x00EE) { // Возврат из подпрограммы
        chip8->sp--;
        chip8->pc = chip8->stack[chip8->sp];
    }
    // Переходы и вызовы (по адресу NNN)
    switch (opcode & 0xF000) {
        case 0x1000: //0x1NNN Переход на адрес NNN
            chip8->pc = opcode & 0x0FFF;
        break;
        case  0x2000: // Вызов подпрограммы по адресу NN
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = opcode & 0x0FFF;
        break;
        case 0xB000: //0x0NNN Переход на адрес NNN + V0
            chip8->pc = (opcode & 0x0FFF) + chip8->V[0];
        break;
    }
    // Условные переходы
    switch (opcode & 0xF000) {
        case 0x3000: //0x3XNN Пропуск следующей инструкции, если Vx == NN
            if (chip8->V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) chip8->pc += 2;
        break;
        case 0x4000: //0x4XNN Пропуск следующей инструкции, если Vx != NN
            if (chip8->V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) chip8->pc += 2;
        break;
        case 0x5000: //0x5XY0 Пропуск следующей инструкции, если Vx == Vy
            if (chip8->V[(opcode & 0x0F00) >> 8] == chip8->V[(opcode & 0x00F0) >> 4]) chip8->pc += 2;
        break;
        case 0x9000: //0x9XY0 Пропуск следующей инструкции, если Vx != Vy
            if (chip8->V[(opcode & 0x0F00) >> 8] != chip8->V[(opcode & 0x00F0) >> 4]) chip8->pc += 2;
        break;
    }
    // Арифметика и логика
    switch (opcode & 0xF00F) { // 0x8XY0 (firs and fourth bits) of the opcode
        case 0x8000: //0x8XY0 Загрузка значения регистра Vy в регистр Vx
            chip8->V[(opcode & 0x0F00) >> 8] = chip8->V[(opcode & 0x00F0) >> 4];
        break;
        case 0x8001: //0x8XY1 Логическое ИЛИ регистров Vx и Vy, результат в Vx
            chip8->V[(opcode & 0x0F00) >> 8] |= chip8->V[(opcode & 0x00F0) >> 4];
        break;
        case 0x8002: //0x8XY2 Логическое И регистров Vx и Vy, результат в Vx
            chip8->V[(opcode & 0x0F00) >> 8] &= chip8->V[(opcode & 0x00F0) >> 4];
        break;
        case 0x8003: //0x8XY3 Логическое исключающее ИЛИ регистров Vx и Vy, результат в Vx
            chip8->V[(opcode & 0x0F00) >> 8] ^= chip8->V[(opcode & 0x00F0) >> 4];
        break;
        case 0x8004: //0x8XY4 Добавление регистра Vy к регистру Vx, результат в Vx. Устанавливает флаг переноса
            chip8->V[(opcode & 0x0F00) >> 8] += chip8->V[(opcode & 0x00F0) >> 4];
            chip8->V[0xF] = (chip8->V[(opcode & 0x0F00) >> 8] < chip8->V[(opcode & 0x00F0) >> 4]) ? 1 : 0;
        break;
        case 0x8005: //0x8XY5 Вычитание регистра Vy из регистра Vx, результат в Vx. Устанавливает флаг заема
            chip8->V[0xF] = (chip8->V[(opcode & 0x0F00) >> 8] > chip8->V[(opcode & 0x00F0) >> 4]) ? 1 : 0;
            chip8->V[(opcode & 0x0F00) >> 8] -= chip8->V[(opcode & 0x00F0) >> 4];
        break;
        case 0x8006: //0x8XY6 Сдвиг значения регистра Vx вправо на 1 бит. Младший бит помещается в регистр VF
            chip8->V[0xF] = chip8->V[(opcode & 0x0F00) >> 8] & 0x1;
            chip8->V[(opcode & 0x0F00) >> 8] >>= 1;
        break;
        case 0x8007: //0x8XY7 Вычитание регистра Vx из регистра Vy, результат в Vx. Устанавливает флаг заема
            chip8->V[0xF] = (chip8->V[(opcode & 0x00F0) >> 4] > chip8->V[(opcode & 0x0F00) >> 8]) ? 1 : 0;
            chip8->V[(opcode & 0x0F00) >> 8] = chip8->V[(opcode & 0x00F0) >> 4] - chip8->V[(opcode & 0x0F00) >> 8];
        break;
        case 0x800E: //0x8XYE Сдвиг значения регистра Vx влево на 1 бит. Старший бит помещается в регистр VF
            chip8->V[0xF] = chip8->V[(opcode & 0x0F00) >> 8] >> 7;
            chip8->V[(opcode & 0x0F00) >> 8] <<= 1;
        break;
    }
    // Управление памятью
    if ((opcode & 0xF000) == 0x6000) { //0x6XNN Загрузка значения NN в регистр Vx
        chip8->V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
    } else if ((opcode & 0xF000) == 0x7000) { //0x7XNN Добавление значения NN к регистру Vx
        chip8->V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
    } else if ((opcode & 0xF000) == 0xA000) {
        chip8->I = opcode & 0x0FFF;
    }
    switch (opcode & 0xF0FF) {
        case 0xF01E : //0xFX1E Добавление значения регистра Vx к регистру I
            chip8->I += chip8->V[(opcode & 0x0F00) >> 8];
        break;
        case 0xF029: //0xFX29 Загрузка в регистр I адреса спрайта символа, код которого хранится в Vx
            chip8->I = chip8->V[(opcode & 0x0F00) >> 8] * 5;
        break;
        case 0xF033: //0xFX33 Запись в память по адресу I десятичного представления значения регистра Vx
            chip8->memory[chip8->I] = chip8->V[(opcode & 0x0F00) >> 8] / 100;
            chip8->memory[chip8->I + 1] = (chip8->V[(opcode & 0x0F00) >> 8] / 10) % 10;
            chip8->memory[chip8->I + 2] = chip8->V[(opcode & 0x0F00) >> 8] % 10;
        break;
        case 0xF055: //0xFX55 Сохранение значений регистров V0-Vx в память, начиная с адреса I
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                chip8->memory[chip8->I + i] = chip8->V[i];
            }
        break;
        case 0xF065: //0xFX65 Загрузка значений из памяти, начиная с адреса I, в регистры V0-Vx
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                chip8->V[i] = chip8->memory[chip8->I + i];
            }
        break;
    }
    // Графика и случайные числа
    if ((opcode & 0xF000) == 0xA000) { //0xCXNN Загрузка случайного числа в регистр Vx
        srand(time(NULL));
        chip8->V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
    } else if ((opcode & 0xF000) == 0xD000) { //0xDXYN Отображение спрайта на экране (V[X], V[Y]) размером 8xN пикселей
        uint8_t x = chip8->V[(opcode & 0x0F00) >> 8] % chip8->width;
        uint8_t y = chip8->V[(opcode & 0x00F0) >> 4] % chip8->height ;
        uint8_t height = opcode & 0x000F;
        chip8->V[0xF] = 0;
        for (int yline = 0; yline < height; yline++) { // Цикл по строкам спрайта
            // Считывание байта спрайта
            uint8_t pixel = chip8->memory[chip8->I + yline];
            for (int xline = 0; xline < 8; xline++) { // Цикл по пикселям строки
                if ((pixel & (0x80 >> xline)) != 0) {
                    if (chip8->screen[(x + xline + ((y + yline) * 64))] == 1) {
                        chip8->V[0xF] = 1; // коллизии
                    }
                    chip8->screen[x + xline + ((y + yline) * 64)] ^= 1; // XOR
                }
            }
        }
        chip8->drawFlag = 1;
    }
    // Клавиатура
    switch (opcode & 0xF0FF) {
        case 0xE09E: //0xEX9E Пропуск следующей инструкции, если клавиша с кодом Vx нажата
            if (chip8->keys[chip8->V[(opcode & 0x0F00) >> 8]] != 0) chip8->pc += 2;
        break;
        case 0xE0A1: //0xEXA1 Пропуск следующей инструкции, если клавиша с кодом Vx не нажата
            if (chip8->keys[chip8->V[(opcode & 0x0F00) >> 8]] == 0) chip8->pc += 2;
        break;
        case 0xF00A: //0xFX0A Ожидание нажатия клавиши, запись ее кода в Vx
            int keyPress = 0;
            for (int i = 0; i < 16; i++) {
                if (chip8->keys[i] != 0) {
                    chip8->V[(opcode & 0x0F00) >> 8] = i;
                    keyPress = 1;
                }
            }
            if (!keyPress) chip8->pc -= 2;
        break;
    }
    // Таймеры и звук
    switch (opcode& 0xF0FF) {
        case 0xF007: //0xFX07 Загрузка значения таймера задержки в регистр Vx
            chip8->V[(opcode & 0x0F00) >> 8] = chip8->delay_timer;
        break;
        case 0xF015: //0xFX15 Загрузка значения регистра Vx в таймер задержки
            chip8->delay_timer = chip8->V[(opcode & 0x0F00) >> 8];
        break;
        case 0xF018: //0xFX18 Загрузка значения регистра Vx в таймер звука
            chip8->sound_timer = chip8->V[(opcode & 0x0F00) >> 8];
        break;

    }

    // ШАГ 3: Обновление таймеров
    if (chip8->delay_timer > 0) chip8->delay_timer--;
    if (chip8->sound_timer > 0) chip8->sound_timer--;

}