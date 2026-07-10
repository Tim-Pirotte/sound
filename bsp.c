#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"

#include "bsp.h"

#define BUZZER_PIN 28

void __attribute__((noreturn)) __assert_func(const char *file, int line, const char *func, const char *failedexpr) {
    watchdog_reboot(0, 0, 0);

    while (true);
}

void bsp_init() {
    stdio_init_all();
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
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
}
