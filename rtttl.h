#ifndef RTTTL_H
#define RTTTL_H

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

char peek(RTTTLParser *parser);
void advance(RTTTLParser *parser);
void advance_number(RTTTLParser *parser);

bool is_valid_duration(uint8_t duration);
bool is_valid_octave(uint8_t octave);

bool parse_control_pair(RTTTLParser *parser);

bool parse_u8(const char *str, uint8_t *result);
bool parse_u16(const char *str, uint16_t *result);

#endif
