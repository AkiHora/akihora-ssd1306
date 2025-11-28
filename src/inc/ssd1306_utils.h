/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

/*
 * @file ssd1306_utils.h
 * @brief Utility macros and helpers used by the SSD1306 driver.
 *
 * This header is intended for internal use inside the library.
 */

#ifndef SSD1306_UTILS_H
#define SSD1306_UTILS_H

#include <stdint.h>

/*
 * Simple MIN/MAX without GCC typeof or statement-expressions.
 * IMPORTANT: do not pass expressions with side effects (i++, foo(), etc.).
 */
#define SSD1306_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SSD1306_MAX(a, b) (((a) > (b)) ? (a) : (b))

/* Clamp value v to the [lo; hi] range. */
#define SSD1306_CLAMP(v, lo, hi) \
	(((v) < (lo)) ? (lo) : (((v) > (hi)) ? (hi) : (v)))

/*
 * Logical XOR for integer values treated as boolean (0 / non-zero).
 * Result is 0 or 1.
 */
#define SSD1306_BOOL_XOR(a, b) ((((a) != 0) ^ ((b) != 0)))

/*
 * Region codes for Cohen–Sutherland line clipping.
 * Bit mask:
 *   0000 – inside
 *   0001 – left
 *   0010 – right
 *   0100 – below
 *   1000 – above
 */
#define SSD1306_CS_INSIDE  0 /* inside viewport */
#define SSD1306_CS_LEFT    1 /* to the left of viewport */
#define SSD1306_CS_RIGHT   2 /* to the right of viewport */
#define SSD1306_CS_BOTTOM  4 /* below viewport */
#define SSD1306_CS_TOP     8 /* above viewport */

/* Compute outcode for a point relative to current screen bounds. */
uint8_t ssd1306_geom_compute_out_code(int16_t x, int16_t y);

/* Swap two integers by value. */
void ssd1306_util_swap_int(int *a, int *b);

/* Delay / timing helpers implemented in ssd1306_utils.c (or user code). */
void ssd1306_time_init(uint32_t hclk_hz);
void ssd1306_time_delay_ms(uint32_t ms);
uint32_t ssd1306_time_ticks_ms(void);

#endif /* SSD1306_UTILS_H */
