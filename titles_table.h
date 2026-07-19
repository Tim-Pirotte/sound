#ifndef TITLES_TABLE_H
#define TITLES_TABLE_H

#include <stdint.h>

#define TITLES_TABLE_SIZE 256
#define TITLES_N_VALUES 126

typedef struct {
    uint8_t f;
    uint8_t c;
} titles_frequency_t;

extern const uint8_t TITLES_TABLE[TITLES_TABLE_SIZE];
extern const titles_frequency_t TITLES_FREQUENCIES[TITLES_N_VALUES];

#endif
