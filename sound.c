#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/assert.h"

#include "frequency_lut.h"
#include "bsp.h"
#include "rtttl.h"

static const int INTERMEDIATE_DELAY_MS = 20;

void run() {
    char song[] = "smwwd1:d=4,o=5,b=125:A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16F,16P,8C6,8A.,G,16C,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16A#,16A,16G,2F,16P,8A.,8F.,8C,8A.,F,16G#,16F,16C,16P,8G#.,2G,8A.,8F.,8C,8A.,F,16G#,16F,8C,2C6,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16F,16P,8C6,8A.,G,16C,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16A#,16A,16G,2F";

    RTTTLParser parser;

    if (!init_parser(&parser, song)) {
        return;
    }

    Note note;

    while (get_next_note(&parser, &note)) {
        play_tone(note.frequency_hz);
        sleep_ms(note.duration_ms);
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
