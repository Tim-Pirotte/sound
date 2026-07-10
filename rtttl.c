#include "pico/stdlib.h"

#include "rtttl.h"

static const int VALID_DURATIONS[] = {1, 2, 4, 8, 16, 32};
static const int VALID_DURATION_COUNT = sizeof(VALID_DURATIONS) / sizeof(VALID_DURATIONS[0]);

static const int MIN_OCTAVE = 4;
static const int MAX_OCTAVE = 7;

bool init_parser(RTTTLParser *parser, const char *song) {

}

char peek(const char *str, int *pos) {
    hard_assert(str != NULL);
    hard_assert(pos != NULL);

    while (str[*pos] == ' ') {
        (*pos)++;
    }

    return str[(*pos)];
}

void advance(const char *str, int *pos) {
    hard_assert(str != NULL);
    hard_assert(pos != NULL);

    if (peek(str, pos) != '\0') (*pos)++;
}

void advance_number(const char *str, int *pos) {
    hard_assert(str != NULL);
    hard_assert(pos != NULL);

    for (char c = peek(str, pos); c != '\0' && '0' <= c && c <= '9'; c = peek(str, pos)) {
        advance(str, pos);
    };
}

bool is_valid_duration(int duration) {
    for (size_t i = 0; i < VALID_DURATION_COUNT; i++) {
        if (VALID_DURATIONS[i] == duration)
            return true;
    }

    printf("Error: duration=%d but should be in [", duration);

    for (int i = 0; i < VALID_DURATION_COUNT; i++) {
        printf("%d%s", VALID_DURATIONS[i], (i < VALID_DURATION_COUNT - 1) ? ", " : "]\n");
    }

    return false;
}

bool is_valid_octave(int octave) {
    if (!(MIN_OCTAVE <= octave && octave <= MAX_OCTAVE)) {
        printf(
            "Error: octave=%d but should be between (inclusive) %d and %d\n",
            octave,
            MIN_OCTAVE,
            MAX_OCTAVE
        );

        return false;
    }

    return true;
}

bool parse_control_pair(const char *str, int *pos, Settings *settings) {
    hard_assert(str != NULL);
    hard_assert(pos != NULL);
    hard_assert(settings != NULL);

    char c = peek(str, pos);

    if (c == '\0') {
        printf("Error: expected a control pair but got the end of the file\n");

        return false;
    }

    advance(str, pos);

    if (peek(str, pos) == '=') {
        advance(str, pos);
    }

    switch (c) {
        case 'o':
            int octave = atoi(&str[*pos]);

            if (!is_valid_octave(octave)) {
                return false;
            }

            settings->default_octave = octave;

            break;
        case 'd':
            int duration = atoi(&str[*pos]);

            if (!is_valid_duration(duration)) {
                return false;
            }

            settings->default_duration = duration;

            break;
        case 'b':
            int bpm = atoi(&str[*pos]);

            if (bpm <= 0) {
                printf("Error: BPM should be larger than 0\n");

                return false;
            }

            settings->bpm = bpm;

            break;
    }

    int pos_before = *pos;

    advance_number(str, pos);

    if (*pos == pos_before) {
        printf("Error: expected a numerical value in a control pair\n");

        return false;
    }

    return true;
}

bool get_next_note(RTTTLParser *parser, Note *out) {

}
