#ifndef TITLES_TABLE_H
#define TITLES_TABLE_H

#include <stdint.h>

enum {
    TITLES_TABLE_SIZE = 256,
    TITLES_N_VALUES = 126,
    TITLES_L = 256,
    TITLES_B = 256,
    TITLES_STATE_WIDTH_BYTES = 2,
};

typedef struct {
    uint8_t f;
    uint8_t c;
} titles_frequency_t;

extern const uint8_t TITLES_TABLE[TITLES_TABLE_SIZE];
extern const titles_frequency_t TITLES_FREQUENCIES[TITLES_N_VALUES];

#endif
