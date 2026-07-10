#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/assert.h"

#include "frequency_lut.h"
#include "bsp.h"
#include "rtttl.h"

static const int DEFAULT_OCTAVE = 6;
static const int DEFAULT_DURATION = 4;
static const int DEFAULT_BPM = 63;
static const int MILLISECONDS_PER_MINUTE = 60 * 1000;
static const int QUARTER_NOTES_PER_WHOLE_NOTE = 4;
static const float DOTTED_NOTE_MULTIPLIER = 1.5f;
static const int INTERMEDIATE_DELAY_MS = 20;
static const int MAX_NAME_LENGTH = 10;

void run() {
    char song[] = "smwwd1:d=4,o=5,b=125:A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16F,16P,8C6,8A.,G,16C,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16A#,16A,16G,2F,16P,8A.,8F.,8C,8A.,F,16G#,16F,16C,16P,8G#.,2G,8A.,8F.,8C,8A.,F,16G#,16F,8C,2C6,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16F,16P,8C6,8A.,G,16C,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16A#,16A,16G,2F";

    int pos = 0;

    printf("Playing: ");

    for (char c = peek(song, &pos); c != ':'; c = peek(song, &pos)) {
        if (c == '\0') {
            printf("\nError: name should be followed by ':'\n");

            return;
        }

        if (pos + 1 > MAX_NAME_LENGTH) {
            printf("\nError: name cannot be longer than %d characters\n", MAX_NAME_LENGTH);

            return;
        }

        putchar(c);
        advance(song, &pos);
    }

    putchar('\n');

    hard_assert(peek(song, &pos) == ':');
    advance(song, &pos);

    Settings settings = {DEFAULT_OCTAVE, DEFAULT_DURATION, DEFAULT_BPM};

    for (char c = peek(song, &pos); c != ':'; c = peek(song, &pos)) {
        if (c == '\0') {
            printf("Error: expected the control section to end with ':'\n");

            return;
        }

        bool ok = parse_control_pair(song, &pos, &settings);

        if (!ok) {
            return;
        }

        c = peek(song, &pos);

        if (c == ',') {
            advance(song, &pos);
        } else if (c != ':') {
            printf("Error: control pairs should contain ',' between them\n");

            return;
        }
    };

    hard_assert(peek(song, &pos) == ':');
    advance(song, &pos);

    while (peek(song, &pos) != '\0') {
        int duration = atoi(&song[pos]);
        duration = duration ? duration : settings.default_duration;

        if (!is_valid_duration(duration)) {
            return;
        }

        advance_number(song, &pos);

        int  note = P;
        bool can_have_sharp = false;

        switch (peek(song, &pos)) {
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
                bool ok = parse_control_pair(song, &pos, &settings);

                if (!ok) {
                    return;
                }
        }

        advance(song, &pos);

        if (can_have_sharp && peek(song, &pos) == '#') {
            advance(song, &pos);

            note++;
        }

        hard_assert(settings.bpm != 0);
        hard_assert(duration != 0);

        float ms_per_note = MILLISECONDS_PER_MINUTE * QUARTER_NOTES_PER_WHOLE_NOTE / settings.bpm;
        float duration_ms = ms_per_note / duration;

        if (peek(song, &pos) == '.') {
            advance(song, &pos);

            duration_ms *= DOTTED_NOTE_MULTIPLIER;
        }

        int octave = atoi(&song[pos]);
        octave = octave ? octave : settings.default_octave;

        if (!is_valid_octave(octave)) {
            return;
        }

        advance_number(song, &pos);

        char c = peek(song, &pos);

        if (c == ',') {
            advance(song, &pos);
        } else if (c != '\0') {
            printf("Error: tone commands should contain ',' between them\n");

            return;
        }

        float frequency = FREQUENCY_LUT[octave * OCTAVE_SIZE + note];

        play_tone(frequency);
        sleep_ms(duration_ms);
        play_tone(0.0f);
        sleep_ms(INTERMEDIATE_DELAY_MS);
    }

    play_tone(0.0f);
}

int main()
{
    bsp_init();

    run();

    while (true) {
        sleep_ms(10000);
    }
}
