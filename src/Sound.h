//
// Created by user on 21.01.2025.
//

#ifndef SOUND_H
#define SOUND_H

#include <SDL_stdinc.h>
#include "SDL2/include/SDL.h"
#include <math.h>

#define SAMPLE_RATE 44100
#define AMPLITUDE 28000
#define BASE_FREQUENCY 440.0

typedef struct {
    double frequency; // Частота сигнала
    double phase;     // Текущая фаза (чтобы сохранять её между вызовами)
} AudioData;

// Прототипы функции (определенного формата) для генерации звука
void playSound(void* userdata, Uint8* stream, int len);
// Функция инициализации SDL Sound
void init_sound(AudioData *audio_data);

#endif //SOUND_H
