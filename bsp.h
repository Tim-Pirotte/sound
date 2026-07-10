#ifndef BSP_H
#define BSP_H

static const int BUZZER_PIN = 28;

void __attribute__((noreturn)) __assert_func(const char *file, int line, const char *func, const char *failedexpr);
void play_tone(float frequency);

#endif
