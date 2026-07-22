#ifndef RTTTL_H
#define RTTTL_H

#include <parser.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

extern Parser RTTTL_PARSER;

bool init_parser_rtttl(Parser *p);
bool get_next_note_rtttl(Parser *p, Note *out);

#endif
