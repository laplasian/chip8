#ifndef CPU_H
#define CPU_H

#include <stdint.h>

typedef struct {
    uint8_t memory[4096];    // Память
    uint8_t V[16];           // Регистры
    uint16_t I;              // Регистр адреса
    uint16_t pc;             // указатель команд
    uint16_t stack[16];      // Стек
    uint8_t sp;              // Указатель стека
    uint8_t delay_timer;     // Таймер задержки
    uint8_t sound_timer;     // Звуковой таймер
    uint8_t screen[128 * 64];// Экран (черно-белый)
    uint8_t keys[16];        // Клавиатура
    int width;               // Ширина экрана
    int height;              // Высота экрана
    int drawFlag;            // Флаг отрисовки
    int superChipmode;       // Флаг SuperChip
} Chip8;

// инициализация регистров, стеков, указателей, загрузка шрифтов в память
void init_cpu(Chip8 *chip8);
// загрузка программы в память начиная с 512 байта
void loadGame(Chip8 *chip8, const char *filename);

/*Цикл интерпретации
Основной цикл эмулятора выполняет следующие действия:
1. Считывает инструкцию (2 байта).
2. Распознает и выполняет команду.
3. Обновляет таймеры.*/
void emulateCycle(Chip8 *chip8);

#endif //CPU_H
