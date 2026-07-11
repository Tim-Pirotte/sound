#ifndef FREQUENCY_LUT_H
#define FREQUENCY_LUT_H

typedef enum {
    P,
    A,
    A_SHARP,
    B,
    C,
    C_SHARP,
    D,
    D_SHARP,
    E,
    F,
    F_SHARP,
    G,
    G_SHARP,

    NOTE_COUNT
} Tone;

#define OCTAVE_SIZE NOTE_COUNT
#define OCTAVE_COUNT 10

extern const float FREQUENCY_LUT[NOTE_COUNT * OCTAVE_COUNT];

#endif
