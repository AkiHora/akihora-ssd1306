/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

/*
 * @file ssd1306.h
 * @brief Public API for SSD1306 OLED display driver.
 *
 * This header provides:
 *  - basic types (pixel color)
 *  - functions to control the display and contrast
 *  - drawing API for pixels, text, primitives and bitmaps
 *
 * All functions whose name contains "_buffer" operate only on the
 * internal framebuffer and do not update the display immediately.
 * To make changes visible, call ssd1306_flush_dirty().
 *
 * Display parameters, enabled fonts and character encoding are
 * configured in ssd1306_conf.h.
 */

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include "ssd1306_conf.h"
#include "ssd1306_fonts.h"

/* Pixel color (without inversion) */
typedef enum {
	Black = 0x00,  /* pixel off  */
	White = 0x01   /* pixel on   */
} SSD1306_COLOR_t;

/* Display dimensions in bytes */
#define SSD1306_HEIGHT_BYTES (SSD1306_HEIGHT / 8)
#define SSD1306_WIDTH_BYTES  (SSD1306_WIDTH  / 8)

/* Display on/off flags */
#define SSD1306_DISPLAY_ON   1
#define SSD1306_DISPLAY_OFF  0

/* Invert mode flags */
#define SSD1306_INVERT_ON    1
#define SSD1306_INVERT_OFF   0

/* Built-in fonts (enabled via SSD1306_INCLUDE_FONT_xx in ssd1306_conf.h) */
#ifdef SSD1306_INCLUDE_FONT_16x30
extern const SSD1306_Font_t font_16x30;
#endif

#ifdef SSD1306_INCLUDE_FONT_11x21
extern const SSD1306_Font_t font_11x21;
#endif

#ifdef SSD1306_INCLUDE_FONT_8x8
extern const SSD1306_Font_t font_8x8;
#endif

/* --------------------------------------------------------------------------
 * Core control functions
 * -------------------------------------------------------------------------- */

/* Initialize the display and internal framebuffer */
void ssd1306_init(void);

/* Turn display on or off (SSD1306_DISPLAY_ON / SSD1306_DISPLAY_OFF) */
void ssd1306_set_display_on(uint8_t on);

/* Set display contrast (0..255) */
void ssd1306_set_contrast(uint8_t value);

/* Set display invert mode (SSD1306_INVERT_ON / SSD1306_INVERT_OFF) */
void ssd1306_set_invert(uint8_t invert);

/* Send command + single parameter to the controller */
void ssd1306_write_command_ex(uint8_t cmd, uint8_t param);

/* --------------------------------------------------------------------------
 * Framebuffer drawing API
 * -------------------------------------------------------------------------- */

void ssd1306_buffer_draw_pixel(uint8_t x, uint8_t y, SSD1306_COLOR_t color);

char ssd1306_buffer_draw_char_font(char ch,
                                   uint8_t x,
                                   uint8_t y,
                                   const SSD1306_Font_t *font,
                                   SSD1306_COLOR_t color);

char ssd1306_buffer_draw_char(char ch,
                              uint8_t x,
                              uint8_t y,
                              SSD1306_COLOR_t color);

void ssd1306_buffer_draw_string_font(const char *str,
                                     uint8_t x,
                                     uint8_t y,
                                     const SSD1306_Font_t *font,
                                     SSD1306_COLOR_t color);

void ssd1306_buffer_draw_string(const char *str,
                                uint8_t x,
                                uint8_t y,
                                SSD1306_COLOR_t color);

/* Fill entire framebuffer with given color */
void ssd1306_buffer_fill(SSD1306_COLOR_t color);

/* Fill framebuffer with black and flush to display */
void ssd1306_display_clear(void);

/* Fill framebuffer with given color and flush to display */
void ssd1306_display_fill(SSD1306_COLOR_t color);

void ssd1306_buffer_draw_line(int16_t x0,
                              int16_t y0,
                              int16_t x1,
                              int16_t y1,
                              SSD1306_COLOR_t color);

void ssd1306_buffer_draw_rect(int16_t x,
                              int16_t y,
                              int16_t w,
                              int16_t h,
                              SSD1306_COLOR_t color);

void ssd1306_buffer_draw_rect_xy(int16_t x0,
                                 int16_t y0,
                                 int16_t x1,
                                 int16_t y1,
                                 SSD1306_COLOR_t color);

void ssd1306_buffer_fill_rect(int16_t x,
                              int16_t y,
                              int16_t w,
                              int16_t h,
                              SSD1306_COLOR_t color);

void ssd1306_buffer_fill_rect_xy(int16_t x0,
                                 int16_t y0,
                                 int16_t x1,
                                 int16_t y1,
                                 SSD1306_COLOR_t color);

void ssd1306_buffer_draw_circle(int16_t xc,
                                int16_t yc,
                                int16_t r,
                                SSD1306_COLOR_t color);

void ssd1306_buffer_fill_circle(int16_t xc,
                                int16_t yc,
                                int16_t r,
                                SSD1306_COLOR_t color);

/*
 * Draw 1bpp bitmap (1 bit per pixel).
 * 'color' is used for bit=1, the opposite color is used for bit=0.
 */
void ssd1306_buffer_draw_bitmap(int16_t x,
                                int16_t y,
                                const uint8_t *image,
                                int16_t width,
                                int16_t height,
                                SSD1306_COLOR_t color);

void ssd1306_buffer_draw_triangle(int x0,
                                  int y0,
                                  int x1,
                                  int y1,
                                  int x2,
                                  int y2,
                                  SSD1306_COLOR_t color);

void ssd1306_buffer_fill_triangle(int x0,
                                  int y0,
                                  int x1,
                                  int y1,
                                  int x2,
                                  int y2,
                                  SSD1306_COLOR_t color);

/* Flush only modified areas of the framebuffer to the display */
void ssd1306_flush_dirty(void);

#endif /* SSD1306_H */
