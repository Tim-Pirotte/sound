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

char peek(const char *str, int *pos);
void advance(const char *str, int *pos);
void advance_number(const char *str, int *pos);

bool is_valid_duration(int duration);
bool is_valid_octave(int octave);

bool parse_control_pair(const char *str, int *pos, Settings *settings);

bool get_next_note(RTTTLParser *parser, Note *out);

#endif
