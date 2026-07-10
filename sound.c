#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "frequency_lut.h"

#define BUZZER_PIN 28

#define MILLISECONDS_PER_MINUTE 60 * 1000
#define QUARTER_NOTES_PER_WHOLE_NOTE 4
#define DOTTED_NOTE_MULTIPLIER 1.5f

enum Note {
    P,
    A,
    A_SHARP,
    B,
    C,
    C_SHARP,
    D,
    D_SHARP,
    E,
    F,
    F_SHARP,
    G,
    G_SHARP
};

typedef struct {
    int default_octave;
    int default_duration;
    int bpm;
} Settings;

void play_tone(uint pin, float frequency) {
    uint slice_num = pwm_gpio_to_slice_num(pin);

    if (frequency == 0) {
        pwm_set_gpio_level(pin, 0);
        return;
    }

    float sys_clk = clock_get_hz(clk_sys);
    uint32_t wrap = 10000;
    float div = sys_clk / (frequency * wrap);

    pwm_set_clkdiv(slice_num, div);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(pin, wrap / 2);
    pwm_set_enabled(slice_num, true);
}

char peek(const char *str, int *pos) {
    while (str[*pos] == ' ') {
        (*pos)++;
    }

    return str[(*pos)];
}

void advance(const char *str, int *pos) {
    if (peek(str, pos) != '\0') (*pos)++;
}

void advance_number(const char *str, int *pos) {
    for (char c = peek(str, pos); c != '\0' && '0' <= c && c <= '9'; c = peek(str, pos)) {
        advance(str, pos);
    };
}

void parse_control_pair(const char *str, int *pos, Settings *settings) {
    char c = peek(str, pos);

    if (c == '\0') {
        // TODO error
    }

    advance(str, pos);

    if (peek(str, pos) != '=') {
        // TODO error
    }

    advance(str, pos);

    switch (c) {
        case 'o':
            settings->default_octave = atoi(&str[*pos]);
            break;
        case 'd':
            settings->default_duration = atoi(&str[*pos]);
            break;
        case 'b':
            settings->bpm = atoi(&str[*pos]);

            if (settings->bpm <= 0) {
                // TODO error
            }

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

    char song[] = "smario2:d=4,o=5,b=125:8g,16c,8e,8g.,16c,8e,16g,16c,16e,16g,8b,a,8p,16c,8g,16c,8e,8g.,16c,8e,16g,16c#,16e,16g,8b,a,8p,16b,8c6,16b,8c6,8a.,16c6,8b,16a,8g,16f#,8g,8e.,16c,8d,16e,8f,16e,8f,8b.4,16e,8d.,c";

    int pos = 0;

    // Skip the name
    for (char c = peek(song, &pos); c != ':'; c = peek(song, &pos)) {
        if (c == '\0') {
            // TODO error
        }

        if (pos + 1 > 10) {
            // TODO handle error
        }

        advance(song, &pos);
    }

    advance(song, &pos);

    Settings settings = {6, 4, 63};

    bool must_follow_with_control_section = false;

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

    // TODO assert ':'

    advance(song, &pos);

    float ms_per_note = MILLISECONDS_PER_MINUTE * QUARTER_NOTES_PER_WHOLE_NOTE / settings.bpm;

    while (peek(song, &pos) != '\0') {
        int duration = atoi(&song[pos]);
        duration = duration ? duration : settings.default_duration;

        advance_number(song, &pos);

        int  note = P;
        bool can_have_sharp = false;

        switch (peek(song, &pos)) {
            case 'a':
                note = A;
                can_have_sharp = true;

                break;
            case 'b':
                note = B;

                break;
            case 'c':
                note = C;
                can_have_sharp = true;

                break;
            case 'd':
                note = D;
                can_have_sharp = true;

                break;
            case 'e':
                note = E;

                break;
            case 'f':
                note = F;
                can_have_sharp = true;

                break;
            case 'g':
                note = G;
                can_have_sharp = true;

                break;
            case 'p':
                note = P;

                break;
            default:
                // TODO handle error
        }

        advance(song, &pos);

        if (can_have_sharp && peek(song, &pos) == '#') {
            advance(song, &pos);

            note++;
        }

        float duration_ms = ms_per_note / duration;

        if (peek(song, &pos) == '.') {
            advance(song, &pos);

            duration_ms *= DOTTED_NOTE_MULTIPLIER;
        }

        int octave = atoi(&song[pos]);
        octave = octave ? octave : settings.default_octave;

        advance_number(song, &pos);

        if (peek(song, &pos) == ',') {
            advance(song, &pos);
        }

        float frequency = FREQUENCY_LUT[octave * 13 + note];

        play_tone(BUZZER_PIN, frequency);
        sleep_ms(duration_ms);
        play_tone(BUZZER_PIN, 0.0f);
        sleep_ms(20);
    }

    play_tone(BUZZER_PIN, 0.0f);

    while (true) {
        sleep_ms(1000);
    }
}
