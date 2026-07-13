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
    READY_OVERFLOWING,
    SKIPPING_OVERFLOWED_LINE
} buffer_state_t;

static char uart_read_buf[UART_READ_BUFFER_SIZE];
static volatile buffer_state_t buffer_state = READING;
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

void transition_to_reading(void) {
    uint32_t irq_state = save_and_disable_interrupts();

    pos = 0;
    buffer_state = READING;

    restore_interrupts(irq_state);
}

void transition_to_ready(void) {
    buffer_state = READY;
}

void transition_to_ready_overflowing(void) {
    // TODO find a solution for showing the error without waiting on the end of command execution
    // printf(
    //     "Error: there is still an unprocessed command waiting to be handled."
    //     " Input will be ignored."
    // );

    buffer_state = READY_OVERFLOWING;
}

void transition_to_skipping_overflowed_line(void) {
    buffer_state = SKIPPING_OVERFLOWED_LINE;
}

void handle_uart_read(void) {
    uint8_t c = uart_getc(uart0);
    bool is_end_of_line = c == '\n' || c == '\r';

    switch (buffer_state) {
    case READING:
        if (is_end_of_line) {
            if (pos != 0) {
                uart_read_buf[pos] = '\0';
                transition_to_ready();
            }

            return;
        }

        if (pos + 1 >= UART_READ_BUFFER_SIZE) {
            // TODO find a solution for showing the error without waiting on the end of command execution
            // printf(
            //     "Error: unread input exceeds uart read buffer size of %d."
            //     " Command will be discarded",
            //     UART_READ_BUFFER_SIZE
            // );

            transition_to_skipping_overflowed_line();

            return;
        }

        uart_read_buf[pos++] = c;

        break;
    case READY:
        if (!is_end_of_line) {
            transition_to_ready_overflowing();
        }

        break;
    case READY_OVERFLOWING:
        if (is_end_of_line) {
            transition_to_ready();
        }

        break;
    case SKIPPING_OVERFLOWED_LINE:
        if (is_end_of_line) {
            transition_to_reading();
        }

        break;
    }
}

bool read_line(char *buf, size_t len) {
    while (buffer_state != READY && buffer_state != READY_OVERFLOWING) {
        __wfi();
    }

    if (pos + 1 > len) {
        return false;
    }

    strcpy(buf, uart_read_buf);

    // TODO This can probably be removed by using a separate state space for overflowing
    uint32_t irq_state = save_and_disable_interrupts();

    switch (buffer_state) {
    case READY:
        transition_to_reading();

        break;
    case READY_OVERFLOWING:
        transition_to_skipping_overflowed_line();

        break;
    }

    restore_interrupts(irq_state);

    return true;
}
