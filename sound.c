#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

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

const float FREQUENCY_LUT[] = {
    0.0f,    // P
    27.500f, // A0
    29.135f, // A0#
    30.868f, // B0
    16.352f, // C0
    17.324f, // C0#
    18.354f, // D0
    19.445f, // D0#
    20.602f, // E0
    21.827f, // F0
    23.125f, // F0#
    24.500f, // G0
    25.957f, // G0#

    0.0f,    // P
    55.000f, // A1
    58.270f, // A1#
    61.735f, // B1
    32.703f, // C1
    34.648f, // C1#
    36.708f, // D1
    38.891f, // D1#
    41.203f, // E1
    43.654f, // F1
    46.249f, // F1#
    48.999f, // G1
    51.913f, // G1#

    0.0f,     // P
    110.000f, // A2
    116.541f, // A2#
    123.471f, // B2
    65.406f,  // C2
    69.296f,  // C2#
    73.416f,  // D2
    77.782f,  // D2#
    82.407f,  // E2
    87.307f,  // F2
    92.499f,  // F2#
    97.999f,  // G2
    103.826f, // G2#

    0.0f,     // P
    220.000f, // A3
    233.082f, // A3#
    246.942f, // B3
    130.813f, // C3
    138.591f, // C3#
    146.832f, // D3
    155.563f, // D3#
    164.814f, // E3
    174.614f, // F3
    184.997f, // F3#
    195.998f, // G3
    207.652f, // G3#

    0.0f,     // P
    440.000f, // A4
    466.164f, // A4#
    493.883f, // B4
    261.626f, // C4
    277.183f, // C4#
    293.665f, // D4
    311.127f, // D4#
    329.628f, // E4
    349.228f, // F4
    369.994f, // F4#
    391.995f, // G4
    415.305f, // G4#

    0.0f,     // P
    880.000f, // A5
    932.328f, // A5#
    987.767f, // B5
    523.251f, // C5
    554.365f, // C5#
    587.330f, // D5
    622.254f, // D5#
    659.255f, // E5
    698.456f, // F5
    739.989f, // F5#
    783.991f, // G5
    830.609f, // G5#

    0.0f,      // P
    1760.000f, // A6
    1864.655f, // A6#
    1975.533f, // B6
    1046.502f, // C6
    1108.731f, // C6#
    1174.659f, // D6
    1244.508f, // D6#
    1318.510f, // E6
    1396.913f, // F6
    1479.978f, // F6#
    1567.982f, // G6
    1661.219f, // G6#

    0.0f,      // P
    3520.000f, // A7
    3729.310f, // A7#
    3951.066f, // B7
    2093.005f, // C7
    2217.461f, // C7#
    2349.318f, // D7
    2489.016f, // D7#
    2637.021f, // E7
    2793.826f, // F7
    2959.955f, // F7#
    3135.964f, // G7
    3322.438f, // G7#

    0.0f,      // P
    7040.000f, // A8
    7458.620f, // A8#
    7902.132f, // B8
    4186.009f, // C8
    4434.922f, // C8#
    4698.636f, // D8
    4978.032f, // D8#
    5274.042f, // E8
    5587.652f, // F8
    5919.910f, // F8#
    6271.928f, // G8
    6644.876f, // G8#

    0.0f,       // P
    14080.000f, // A9
    14917.240f, // A9#
    15804.264f, // B9
    8372.018f,  // C9
    8869.844f,  // C9#
    9397.272f,  // D9
    9956.064f,  // D9#
    10548.084f, // E9
    11175.304f, // F9
    11839.820f, // F9#
    12543.856f, // G9
    13289.752f  // G9#
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

int main()
{
    stdio_init_all();
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    char song[] = "smario2:d=4,o=5,b=125:8g,16c,8e,8g.,16c,8e,16g,16c,16e,16g,8b,a,8p,16c,8g,16c,8e,8g.,16c,8e,16g,16c#,16e,16g,8b,a,8p,16b,8c6,16b,8c6,8a.,16c6,8b,16a,8g,16f#,8g,8e.,16c,8d,16e,8f,16e,8f,8b.4,16e,8d.,c";

    int pos = 0;

    // Skip the name
    while(song[pos] != ':') pos++;
    pos++;

    pos += 2;
    int default_duration = atoi(&song[pos]);
    while(song[pos] != ',') pos++;
    pos++;

    pos += 2;
    int default_octave = atoi(&song[pos]);
    while(song[pos] != ',') pos++;
    pos++;

    pos += 2;
    int bpm = atoi(&song[pos]);
    float ms_per_note = (60.0f * 1000.0f * 4.0f) / bpm;
    while(song[pos] != ':') pos++;
    pos++;

    while (pos < strlen(song)) {
        int duration = atoi(&song[pos]);

        if (duration == 0) {
            duration = default_duration;
        }

        while('0' <= song[pos] && song[pos] <= '9') pos++;

        int  note           = P;
        bool can_have_sharp = false;

        switch (song[pos]) {
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
        }

        pos++;

        if (can_have_sharp && song[pos] == '#') {
            note++;
            pos++;
        }

        float duration_ms = ms_per_note / duration;

        if (song[pos] == '.') {
            duration_ms *= 1.5f;
            pos++;
        }

        int octave = atoi(&song[pos]);

        if (octave == 0) {
            octave = default_octave;
        }

        while('0' <= song[pos] && song[pos] <= '9') pos++;

        if (song[pos] == ',') {
            pos++;
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
