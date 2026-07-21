#ifndef RTTTL_H
#define RTTTL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    float frequency_hz;
    float duration_ms;
} Note;

typedef struct {
    uint8_t  default_octave;
    uint8_t  default_duration;
    uint16_t bpm;
} Settings;

typedef struct {
    const char *song;
    size_t      pos;
    Settings    settings;
} RTTTLParser;

bool init_parser(RTTTLParser *parser, const char *song);
bool get_next_note(RTTTLParser *parser, Note *out);

#endif
