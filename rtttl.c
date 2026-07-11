#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "rtttl.h"
#include "frequency_lut.h"

static const uint8_t  DEFAULT_OCTAVE = 6;
static const uint8_t  DEFAULT_DURATION = 4;
static const uint16_t DEFAULT_BPM = 63;

static const uint8_t MAX_NAME_LENGTH = 10;

static const uint32_t MILLISECONDS_PER_MINUTE = 60 * 1000;
static const uint32_t QUARTER_NOTES_PER_WHOLE_NOTE = 4;

static const float DOTTED_NOTE_MULTIPLIER = 1.5f;

static const uint8_t VALID_DURATIONS[] = {1, 2, 4, 8, 16, 32};
static const size_t VALID_DURATION_COUNT = sizeof(VALID_DURATIONS) / sizeof(VALID_DURATIONS[0]);

static const uint8_t MIN_OCTAVE = 4;
static const uint8_t MAX_OCTAVE = 7;

bool init_parser(RTTTLParser *parser, const char *song) {
    parser->song = song;
    parser->pos = 0;
    parser->settings = (Settings){DEFAULT_OCTAVE, DEFAULT_DURATION, DEFAULT_BPM};

    printf("Playing: ");

    for (char c = peek(parser); c != ':'; c = peek(parser)) {
        if (c == '\0') {
            printf("\nError: name should be followed by ':'\n");

            return false;
        }

        if (parser->pos + 1 > MAX_NAME_LENGTH) {
            printf("\nError: name cannot be longer than %d characters\n", MAX_NAME_LENGTH);

            return false;
        }

        putchar(c);
        advance(parser);
    }

    putchar('\n');

    hard_assert(peek(parser) == ':');
    advance(parser);

    for (char c = peek(parser); c != ':'; c = peek(parser)) {
        if (c == '\0') {
            printf("Error: expected the control section to end with ':'\n");

            return false;
        }

        bool ok = parse_control_pair(parser);

        if (!ok) {
            return false;
        }

        c = peek(parser);

        if (c == ',') {
            advance(parser);
        } else if (c != ':') {
            printf("Error: control pairs should contain ',' between them\n");

            return false;
        }
    };

    hard_assert(peek(parser) == ':');
    advance(parser);

    return true;
}

bool get_next_note(RTTTLParser *parser, Note *out) {
    // We keep going until we found a note or the end has been reached
    // <tone-command> := <note> | <control-pair>
    while (true) {
        if (peek(parser) == '\0') return false;

        // TODO what if this is a control sequence
        uint8_t duration;

        if (!parse_u8(&parser->song[parser->pos], &duration)) {
            return false;
        }

        duration = duration ? duration : parser->settings.default_duration;

        if (!is_valid_duration(duration)) {
            return false;
        }

        advance_number(parser);

        Tone note = P;
        bool can_have_sharp = false;

        switch (peek(parser)) {
            case 'A':
                note = A;
                can_have_sharp = true;

                break;
            case 'B':
                note = B;

                break;
            case 'C':
                note = C;
                can_have_sharp = true;

                break;
            case 'D':
                note = D;
                can_have_sharp = true;

                break;
            case 'E':
                note = E;

                break;
            case 'F':
                note = F;
                can_have_sharp = true;

                break;
            case 'G':
                note = G;
                can_have_sharp = true;

                break;
            case 'P':
                note = P;

                break;
            default:
                bool ok = parse_control_pair(parser);

                if (!ok) {
                    return false;
                } else {
                    continue;
                }
        }

        advance(parser);

        if (can_have_sharp && peek(parser) == '#') {
            advance(parser);

            note++;
        }

        hard_assert(parser->settings.bpm != 0);
        hard_assert(duration != 0);

        float ms_per_note = MILLISECONDS_PER_MINUTE * QUARTER_NOTES_PER_WHOLE_NOTE / parser->settings.bpm;
        float duration_ms = ms_per_note / duration;

        if (peek(parser) == '.') {
            advance(parser);

            duration_ms *= DOTTED_NOTE_MULTIPLIER;
        }

        uint8_t octave;

        if (!parse_u8(&parser->song[parser->pos], &octave)) {
            return false;
        }

        octave = octave ? octave : parser->settings.default_octave;

        if (!is_valid_octave(octave)) {
            return false;
        }

        advance_number(parser);

        char c = peek(parser);

        if (c == ',') {
            advance(parser);
        } else if (c != '\0') {
            printf("Error: tone commands should contain ',' between them\n");

            return false;
        }

        float frequency = FREQUENCY_LUT[octave * OCTAVE_SIZE + note];

        out->frequency_hz = frequency;
        out->duration_ms = duration_ms;

        return true;
    }
}

char peek(RTTTLParser *parser) {
    hard_assert(parser != NULL);
    hard_assert(parser->song != NULL);

    while (parser->song[parser->pos] == ' ') {
        parser->pos++;
    }

    return parser->song[parser->pos];
}

void advance(RTTTLParser *parser) {
    hard_assert(parser != NULL);
    hard_assert(parser->song != NULL);

    if (peek(parser) != '\0') parser->pos++;
}

void advance_number(RTTTLParser *parser) {
    hard_assert(parser != NULL);
    hard_assert(parser->song != NULL);

    for (char c = peek(parser); c != '\0' && '0' <= c && c <= '9'; c = peek(parser)) {
        advance(parser);
    };
}

bool is_valid_duration(uint8_t duration) {
    for (size_t i = 0; i < VALID_DURATION_COUNT; i++) {
        if (VALID_DURATIONS[i] == duration)
            return true;
    }

    printf("Error: duration=%d but should be in [", duration);

    for (size_t i = 0; i < VALID_DURATION_COUNT; i++) {
        printf("%d%s", VALID_DURATIONS[i], (i < VALID_DURATION_COUNT - 1) ? ", " : "]\n");
    }

    return false;
}

bool is_valid_octave(uint8_t octave) {
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

bool parse_control_pair(RTTTLParser *parser) {
    hard_assert(parser != NULL);
    hard_assert(parser->song != NULL);

    char c = peek(parser);

    if (c == '\0') {
        printf("Error: expected a control pair but got the end of the file\n");

        return false;
    }

    advance(parser);

    if (peek(parser) == '=') {
        advance(parser);
    }

    switch (c) {
        case 'o':
            uint8_t octave;

            if (!parse_u8(&parser->song[parser->pos], &octave)) {
                return false;
            }

            if (!is_valid_octave(octave)) {
                return false;
            }

            parser->settings.default_octave = octave;

            break;
        case 'd':
            uint8_t duration;

            if (!parse_u8(&parser->song[parser->pos], &duration)) {
                return false;
            }

            if (!is_valid_duration(duration)) {
                return false;
            }

            parser->settings.default_duration = duration;

            break;
        case 'b':
            uint16_t bpm;

            if (!parse_u16(&parser->song[parser->pos], &bpm)) {
                return false;
            }

            if (bpm == 0) {
                printf("Error: BPM should be larger than 0\n");

                return false;
            }

            parser->settings.bpm = bpm;

            break;
    }

    size_t pos_before = parser->pos;

    advance_number(parser);

    if (parser->pos == pos_before) {
        printf("Error: expected a numerical value in a control pair\n");

        return false;
    }

    return true;
}

bool parse_u8(const char *str, uint8_t *result) {
    if (*str == '-') return false;

    errno = 0;

    char *end;
    unsigned long value = strtoul(str, &end, 10);

    if (errno == ERANGE || value > UINT8_MAX) {
        printf("Error: number exceeds u8 capacity");

        return false;
    }

    *result = (uint8_t)value;

    return true;
}

bool parse_u16(const char *str, uint16_t *result) {
    if (*str == '-') return false;

    errno = 0;

    char *end;
    unsigned long value = strtoul(str, &end, 10);

    if (errno == ERANGE || value > UINT16_MAX) {
        printf("Error: number exceeds u16 capacity");

        return false;
    }

    *result = (uint16_t)value;

    return true;
}
