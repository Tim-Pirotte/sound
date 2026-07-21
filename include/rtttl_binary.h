#ifndef RTTTL_BINARY_H
#define RTTTL_BINARY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float frequency_hz;
    float duration_ms;
} Note;

typedef struct {
    const uint16_t *song;
    size_t         pos;
    uint16_t       bpm;
} BinaryParser;

bool init_parser(BinaryParser *parser, const uint16_t song[]);
bool get_next_note(BinaryParser *parser, Note *out);

#endif
