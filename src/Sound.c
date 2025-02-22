//
// Created by user on 21.01.2025.
//

#include "Sound.h"
#include <stdio.h>

#define VIBRATO_SPEED 5
#define VIBRATO_DEPTH 10

// Прямоугольная волна
double square_wave(double phase) {
    return phase < M_PI ? 1.0 : -1.0;
}

void playSound(void* userdata, Uint8* stream, int len)
{
    AudioData* audio_data = (AudioData*)userdata;
    float* buffer = (float*)stream; // Аудио в формате float
    int samples = len / sizeof(float); // Количество float-значений

    double vibrato_speed = VIBRATO_SPEED;  // Скорость вибрато
    double vibrato_depth = VIBRATO_DEPTH;  // Глубина вибрато
    double vibrato;
    double current_frequency;
    double phase_increment;
    float sample;

    for (int i = 0; i < samples; i += 2) { // Для стерео: шаг 2

        vibrato = sin(vibrato_speed * audio_data->phase) * vibrato_depth;
        current_frequency = audio_data->frequency + vibrato;

        // Увеличиваем фазу с учётом текущей частоты
        phase_increment = 2.0 * M_PI * current_frequency / SAMPLE_RATE;
        audio_data->phase += phase_increment;
        if (audio_data->phase >= 2.0 * M_PI) {
            audio_data->phase -= 2.0 * M_PI;
        }

        // Генерируем прямоугольную волну
        sample = (float)(square_wave(audio_data->phase) * 0.5); // 0.5 для ограничения амплитуды
        buffer[i] = sample;     // Левый канал
        buffer[i + 1] = sample; // Правый канал
    }
}


void init_sound(AudioData *audio_data)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf( "Failed to initialize SDL:\n");
        exit(1);
    }
    SDL_AudioSpec desired_spec;
    SDL_AudioSpec obtained_spec;

    SDL_memset(&desired_spec, 0, sizeof(desired_spec));
    desired_spec.freq = SAMPLE_RATE;
    desired_spec.format = AUDIO_S16SYS;
    desired_spec.channels = 2;
    desired_spec.samples = 1024;
    desired_spec.callback = playSound;
    desired_spec.userdata = audio_data;

    if (SDL_OpenAudio(&desired_spec, &obtained_spec) < 0) {
            printf("Failed to open audio:\n");
            exit(1);
        }

    // Ниже отладка на всякий случай
    //SDL_setenv("SDL_AUDIODRIVER", "pulseaudio", 1);

    // int num_drivers = SDL_GetNumAudioDrivers();
    // printf("Available audio drivers:\n");
    // for (int i = 0; i < num_drivers; ++i) {
    //     printf("  %s\n", SDL_GetAudioDriver(i));
    // }



    // printf("Obtained audio spec:\n");
    // printf("  Frequency: %d\n", obtained_spec.freq);
    // printf("  Format: 0x%x\n", obtained_spec.format);
    // printf("  Channels: %d\n", obtained_spec.channels);
    // printf("  Samples: %d\n", obtained_spec.samples);

}