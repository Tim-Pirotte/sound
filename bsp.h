#ifndef BSP_H
#define BSP_H

void __attribute__((noreturn)) __assert_func(const char *file, int line, const char *func, const char *failedexpr);
void bsp_init();
void play_tone(float frequency);

#endif
