#ifndef FREQUENCY_LUT_H
#define FREQUENCY_LUT_H

#include <stdint.h>

typedef enum {
    NOTE_P,
    NOTE_A,
    NOTE_A_SHARP,
    NOTE_B,
    NOTE_C,
    NOTE_C_SHARP,
    NOTE_D,
    NOTE_D_SHARP,
    NOTE_E,
    NOTE_F,
    NOTE_F_SHARP,
    NOTE_G,
    NOTE_G_SHARP,

    NOTE_COUNT
} Tone;

enum {
    OCTAVE_COUNT = 10
};

extern const float FREQUENCY_LUT[NOTE_COUNT * OCTAVE_COUNT];

float get_note(uint8_t octave, Tone tone);

#endif
