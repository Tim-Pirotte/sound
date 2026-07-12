#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"

#include "bsp.h"

#define BUZZER_PIN 28

#define UART_READ_BUFFER_SIZE 2048
#define WAIT_FOR_INPUT_SLEEP_MS 5

static char uart_read_buf[UART_READ_BUFFER_SIZE];
static volatile size_t pos = 0;
static volatile bool ready = false;
static volatile bool command_in_buffer_err_shown = false;

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

void handle_uart_read(void) {
    // This is problematic since it can go from ready=false to ready=true
    // in the middle of sending a command
    if (ready && !command_in_buffer_err_shown) {
        printf(
            "Error: there is still an unprocessed command waiting to be handled."
            " Input will be ignored."
        );

        command_in_buffer_err_shown = true;

        return;
    }

    uint8_t c = uart_getc(uart0);

    if (c == '\n' || c == '\r') {
        if (pos != 0) {
            uart_read_buf[pos] = '\0';
            ready = true;

            return;
        }
    } else if (pos < UART_READ_BUFFER_SIZE - 1) {
        uart_read_buf[pos++] = c;
    } else {
        printf("Error: unread input exceeds uart read buffer size of %d", UART_READ_BUFFER_SIZE);
    }
}

bool read_input(char *buf, size_t len) {
    while (!ready) {
        sleep_ms(WAIT_FOR_INPUT_SLEEP_MS);
    }

    if (pos + 1 > len) {
        return false;
    }

    strcpy(buf, uart_read_buf);

    pos = 0;
    command_in_buffer_err_shown = false;
    ready = false;

    return true;
}
