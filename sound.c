#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "frequency_lut.h"

#define BUZZER_PIN 28

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

char advance(const char *str, int *pos) {
    if (peek(str, pos) != '\0') (*pos)++;
}

void advance_after(const char *str, int *pos, const char to_skip) {
    for (char c = peek(str, pos); c != '\0' && c != to_skip; c = peek(str, pos)) {
        advance(str, pos);
    }

    advance(str, pos);
}

int main()
{
    stdio_init_all();
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    char song[] = "smario2:d=4,o=5,b=125:8g,16c,8e,8g.,16c,8e,16g,16c,16e,16g,8b,a,8p,16c,8g,16c,8e,8g.,16c,8e,16g,16c#,16e,16g,8b,a,8p,16b,8c6,16b,8c6,8a.,16c6,8b,16a,8g,16f#,8g,8e.,16c,8d,16e,8f,16e,8f,8b.4,16e,8d.,c";

    int pos = 0;

    // Skip the name
    advance_after(song, &pos, ':');

    // TODO handle end of string error

    advance_after(song, &pos, '=');
    int default_duration = atoi(&song[pos]);
    advance_after(song, &pos, ',');

    advance_after(song, &pos, '=');
    int default_octave = atoi(&song[pos]);
    advance_after(song, &pos, ',');

    advance_after(song, &pos, '=');
    int bpm = atoi(&song[pos]);
    float ms_per_note = (60.0f * 1000.0f * 4.0f) / bpm;
    advance_after(song, &pos, ':');

    while (pos < strlen(song)) {
        int duration = atoi(&song[pos]);
        duration = duration ? duration : default_duration;

        for (char c = peek(song, &pos); c != '\0' && '0' <= c && c <= '9'; c = peek(song, &pos)) {
            advance(song, &pos);
        };

        int  note           = P;
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

            duration_ms *= 1.5f;
        }

        int octave = atoi(&song[pos]);
        octave = octave ? octave : default_octave;

        for (char c = peek(song, &pos); c != '\0' && '0' <= c && c <= '9'; c = peek(song, &pos)) {
            advance(song, &pos);
        }

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
