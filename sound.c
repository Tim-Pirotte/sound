#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/assert.h"

#include "frequency_lut.h"
#include "bsp.h"
#include "rtttl.h"

#define COMMAND_BUFFER_SIZE 2048

static const uint32_t INTERMEDIATE_DELAY_MS = 20;

void execute_command() {
    // char command[] = "smwwd1:d=4,o=5,b=125:A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16F,16P,8C6,8A.,G,16C,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16A#,16A,16G,2F,16P,8A.,8F.,8C,8A.,F,16G#,16F,16C,16P,8G#.,2G,8A.,8F.,8C,8A.,F,16G#,16F,8C,2C6,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16F,16P,8C6,8A.,G,16C,A,8F.,16C,16D,16F,16P,F,16D,16C,16P,16F,16P,16A#,16A,16G,2F";
    // char command[] = "pacman:d=4,o=5,b=112:32B,32P,32B6,32P,32F#6,32P,32D#6,32P,32B6,32F#6,16P,16D#6,16P,32C6,32P,32C7,32P,32G6,32P,32E6,32P,32C7,32G6,16P,16E6,16P,32B,32P,32B6,32P,32F#6,32P,32D#6,32P,32B6,32F#6,16P,16D#6,16P,32D#6,32E6,32F6,32P,32F6,32F#6,32G6,32P,32G6,32G#6,32A6,32P,32B.6";

    static char command[COMMAND_BUFFER_SIZE];

    if (!read_input(command, COMMAND_BUFFER_SIZE)) {
        printf("Error: command length exceeds buffer size of %d", COMMAND_BUFFER_SIZE);

        return;
    }

    RTTTLParser parser;

    if (!init_parser(&parser, command)) {
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

    while (true) {
        execute_command();
    }
}
