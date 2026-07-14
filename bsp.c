#include <stdio.h>
#include <string.h>
#include <hardware/sync.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"

#include "bsp.h"

#define BUZZER_PIN 28

#define UART_READ_BUFFER_SIZE 2048

typedef enum {
    READING,
    READY,
} line_state_t;

typedef enum {
    CLEAR,
    OVERFLOWING
} buffer_state_t;

static char uart_read_buf[UART_READ_BUFFER_SIZE];
static volatile line_state_t line_state = READING;
static volatile buffer_state_t buffer_state = CLEAR;
static volatile size_t pos = 0;

void __attribute__((noreturn)) __assert_func(const char *file, int line, const char *func, const char *failedexpr) {
    watchdog_reboot(0, 0, 0);

    while (true);
}

void bsp_init(void) {
    stdio_init_all();
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    gpio_set_function(27, GPIO_FUNC_PWM);

    uart_set_fifo_enabled(uart0, false);
    irq_set_exclusive_handler(UART0_IRQ, handle_uart_read);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);
}

void play_tone(float frequency) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    if (frequency == 0) {
        pwm_set_gpio_level(BUZZER_PIN, 0);

        return;
    }

    float sys_clk = clock_get_hz(clk_sys);
    uint32_t wrap = 10000;

    hard_assert(frequency != 0);
    hard_assert(wrap != 0);

    float div = sys_clk / (frequency * wrap);

    pwm_set_clkdiv(slice_num, div);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(BUZZER_PIN, wrap / 2);
    pwm_set_enabled(slice_num, true);




    slice_num = pwm_gpio_to_slice_num(27);

    if (frequency == 0) {
        pwm_set_gpio_level(27, 0);

        return;
    }

    pwm_set_clkdiv(slice_num, div);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(27, wrap / 2);
    pwm_set_enabled(slice_num, true);
}

// TODO buffer to ensure capture of all chars over 921600 Bd
void handle_uart_read(void) {
    uint8_t c = uart_getc(uart0);
    bool is_end_of_line = c == '\n' || c == '\r';

    switch (line_state) {
    case READING:
        switch (buffer_state) {
        case CLEAR:
            if (is_end_of_line) {
                if (pos != 0) {
                    uart_read_buf[pos] = '\0';
                    line_state = READY;
                }

                return;
            }

            if (pos + 1 >= UART_READ_BUFFER_SIZE) {
                // TODO find a solution for showing the error without waiting on the end of command execution
                // We can do this in an RTOS thread
                // printf(
                //     "Error: unread input exceeds uart read buffer size of %d."
                //     " Command will be discarded",
                //     UART_READ_BUFFER_SIZE
                // );

                buffer_state = OVERFLOWING;

                return;
            }

            uart_read_buf[pos++] = c;

            break;
        case OVERFLOWING:
            if (is_end_of_line) {
                pos = 0;
                buffer_state = CLEAR;
            }

            break;
        }

        break;
    case READY:
        switch (buffer_state) {
        case CLEAR:
            if (!is_end_of_line) {
                buffer_state = OVERFLOWING;
            }

            break;
        case OVERFLOWING:
            if (is_end_of_line) {
                buffer_state = CLEAR;
            }

            break;
        }

        break;
    }
}

bool read_line(char *buf, size_t len) {
    while (line_state != READY) {
        __wfi();
    }

    if (pos + 1 > len) {
        return false;
    }

    strcpy(buf, uart_read_buf);

    pos = 0;
    line_state = READING;

    return true;
}
