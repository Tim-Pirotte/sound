#ifndef RTTTL_H
#define RTTTL_H

typedef struct {
    float frequency_hz;
    float duration_ms;
} Note;

typedef struct {
    int default_octave;
    int default_duration;
    int bpm;
} Settings;

typedef struct {
    const char *song;
    int pos;
    Settings settings;
} RTTTLParser;

bool init_parser(RTTTLParser *parser, const char *song);
bool get_next_note(RTTTLParser *parser, Note *out);

char peek(RTTTLParser *parser);
void advance(RTTTLParser *parser);
void advance_number(RTTTLParser *parser);

bool is_valid_duration(int duration);
bool is_valid_octave(int octave);

bool parse_control_pair(RTTTLParser *parser);

#endif
