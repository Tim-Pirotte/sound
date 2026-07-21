#ifndef BSP_H
#define BSP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void __attribute__((noreturn)) __assert_func(const char *file, int line, const char *func, const char *failedexpr);
void bsp_init(void);
void play_tone(float frequency);
void handle_uart_read(void);
bool read_line(char *buf, size_t len);
void bsp_assert(bool expression);
void bsp_sleep_ms(uint32_t ms);

#endif
