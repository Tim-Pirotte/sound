#ifndef COMMANDS_TABLE_H
#define COMMANDS_TABLE_H

#include <stdint.h>

enum {
    COMMANDS_TABLE_SIZE = 16384,
    COMMANDS_N_VALUES = 1463,
    COMMANDS_L = 65536,
    COMMANDS_B = 256,
    COMMANDS_STATE_WIDTH_BYTES = 3,
};

typedef struct {
    uint16_t f;
    uint16_t c;
} commands_frequency_t;

extern const uint16_t COMMANDS_TABLE[COMMANDS_TABLE_SIZE];
extern const commands_frequency_t COMMANDS_FREQUENCIES[COMMANDS_N_VALUES];

#endif
