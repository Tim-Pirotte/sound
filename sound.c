#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/assert.h"
#include "hardware/watchdog.h"

#include "frequency_lut.h"

static const int BUZZER_PIN = 28;
static const int DEFAULT_OCTAVE = 6;
static const int DEFAULT_DURATION = 4;
static const int DEFAULT_BPM = 63;
static const int MILLISECONDS_PER_MINUTE = 60 * 1000;
static const int QUARTER_NOTES_PER_WHOLE_NOTE = 4;
static const float DOTTED_NOTE_MULTIPLIER = 1.5f;
static const int MIN_OCTAVE = 4;
static const int MAX_OCTAVE = 7;
static const int VALID_DURATIONS[] = {1, 2, 4, 8, 16, 32};
static const int VALID_DURATION_COUNT = sizeof(VALID_DURATIONS) / sizeof(VALID_DURATIONS[0]);
static const int INTERMEDIATE_DELAY_MS = 20;
static const int MAX_NAME_LENGTH = 10;

typedef struct {
    int default_octave;
    int default_duration;
    int bpm;
} Settings;

void __attribute__((noreturn)) __assert_func(const char *file, int line, const char *func, const char *failedexpr) {
    watchdog_reboot(0, 0, 0);

    while (true);
}

void play_tone(uint pin, float frequency) {
    uint slice_num = pwm_gpio_to_slice_num(pin);

    if (frequency == 0) {
        pwm_set_gpio_level(pin, 0);
        return;
    }

    float sys_clk = clock_get_hz(clk_sys);
    uint32_t wrap = 10000;

    hard_assert(frequency != 0);
    hard_assert(wrap != 0);

    float div = sys_clk / (frequency * wrap);

    pwm_set_clkdiv(slice_num, div);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(pin, wrap / 2);
    pwm_set_enabled(slice_num, true);
}

bool is_valid_duration(int duration) {
    for (size_t i = 0; i < VALID_DURATION_COUNT; i++) {
        if (VALID_DURATIONS[i] == duration)
            return true;
    }

    return false;
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

void parse_control_pair(const char *str, int *pos, Settings *settings) {
    hard_assert(str != NULL);
    hard_assert(pos != NULL);
    hard_assert(settings != NULL);

    char c = peek(str, pos);

    if (c == '\0') {
        // TODO error
    }

    advance(str, pos);

    if (peek(str, pos) == '=') {
        advance(str, pos);
    }

    switch (c) {
        case 'o':
            int octave = atoi(&str[*pos]);

            if (!(MIN_OCTAVE <= octave && octave <= MAX_OCTAVE)) {
                // TODO error
            }

            settings->default_octave = octave;

            break;
        case 'd':
            int duration = atoi(&str[*pos]);

            if (!is_valid_duration(duration)) {
                // TODO error
            }

            settings->default_duration = duration;

            break;
        case 'b':
            int bpm = atoi(&str[*pos]);

            if (bpm <= 0) {
                // TODO error
            }

            settings->bpm = bpm;

            break;
    }

    int pos_before = *pos;

    advance_number(str, pos);

    if (*pos == pos_before) {
        // TODO error
    }
}

int main()
{
    stdio_init_all();
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    char song[] = "smwwd1:d=4,o=5,b=125:A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16F,16P,8C6,8A.,G,16C,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16A#,16A,16G,2F,16P,8A.,8F.,8C,8A.,F,16G#,16F,16C,16P,8G#.,2G,8A.,8F.,8C,8A.,F,16G#,16F,8C,2C6,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16F,16P,8C6,8A.,G,16C,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16A#,16A,16G,2F";

    int pos = 0;

    // Skip the name
    for (char c = peek(song, &pos); c != ':'; c = peek(song, &pos)) {
        if (c == '\0') {
            // TODO error
        }

        if (pos + 1 > MAX_NAME_LENGTH) {
            // TODO handle error
        }

        advance(song, &pos);
    }

    hard_assert(peek(song, &pos) == ':');
    advance(song, &pos);

    Settings settings = {DEFAULT_OCTAVE, DEFAULT_DURATION, DEFAULT_BPM};

    for (char c = peek(song, &pos); c != ':'; c = peek(song, &pos)) {
        if (c == '\0') {
            // TODO error
        }

        parse_control_pair(song, &pos, &settings);

        c = peek(song, &pos);

        if (c == ',') {
            advance(song, &pos);
        } else if (c != ':') {
            // TODO error
        }
    };

    hard_assert(peek(song, &pos) == ':');
    advance(song, &pos);

    while (peek(song, &pos) != '\0') {
        int duration = atoi(&song[pos]);
        duration = duration ? duration : settings.default_duration;

        if (!is_valid_duration(duration)) {
            // TODO error
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
                parse_control_pair(song, &pos, &settings);
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

        if (!(MIN_OCTAVE <= octave && octave <= MAX_OCTAVE)) {
            // TODO error
        }

        advance_number(song, &pos);

        char c = peek(song, &pos);

        if (c == ',') {
            advance(song, &pos);
        } else if (c != '\0') {
            // TODO error
        }

        float frequency = FREQUENCY_LUT[octave * OCTAVE_SIZE + note];

        play_tone(BUZZER_PIN, frequency);
        sleep_ms(duration_ms);
        play_tone(BUZZER_PIN, 0.0f);
        sleep_ms(INTERMEDIATE_DELAY_MS);
    }

    play_tone(BUZZER_PIN, 0.0f);

    while (true) {
        sleep_ms(10000);
    }
}
