/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

/*
 * @file ssd1306_priv.h
 * @brief Internal definitions for SSD1306 driver (not part of public API).
 */


#ifndef SSD1306_PRIV_H
#define SSD1306_PRIV_H

#include <stdint.h>
#include "ssd1306_conf.h"

/* =====================================================================
 * Display type and geometry
 * ===================================================================== */

#define SSD1306_DISPLAY_128x64         0
#define SSD1306_DISPLAY_64x32          1
#define SSD1306_DISPLAY_64x32_FAKE     2

#ifndef SSD1306_DISPLAY_TYPE
#define SSD1306_DISPLAY_TYPE SSD1306_DISPLAY_128x64
#endif

#if (SSD1306_DISPLAY_TYPE == SSD1306_DISPLAY_128x64)

	#define SSD1306_WIDTH       128
	#define SSD1306_HEIGHT      64
	#define SSD1306_X_OFFSET    0
	#define SSD1306_PAGE_OFFSET 0

#elif (SSD1306_DISPLAY_TYPE == SSD1306_DISPLAY_64x32)

	#define SSD1306_WIDTH       64
	#define SSD1306_HEIGHT      32
	#define SSD1306_X_OFFSET    0
	#define SSD1306_PAGE_OFFSET 0


#elif (SSD1306_DISPLAY_TYPE == SSD1306_DISPLAY_64x32_FAKE)

	#define SSD1306_WIDTH       64
	#define SSD1306_HEIGHT      32
	#define SSD1306_X_OFFSET    32
	#define SSD1306_PAGE_OFFSET 4

#else
	#error "Unknown SSD1306_DISPLAY_TYPE"
#endif

/* Framebuffer size in bytes (1 byte = 8 vertical pixels) */
#define SSD1306_BUFFER_SIZE      (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

/* Dirty flags array size (1 bit per framebuffer byte) */
#define SSD1306_DIRTY_FLAGS_SIZE ((SSD1306_WIDTH / 8) * (SSD1306_HEIGHT / 8))

/* Column offset split into low/high nibbles for commands */
#define SSD1306_X_OFFSET_LOWER   (SSD1306_X_OFFSET & 0x0F)
#define SSD1306_X_OFFSET_UPPER   ((SSD1306_X_OFFSET >> 4) & 0x07)

/* --------------------------------------------------------------------------
 * Internal driver state
 * -------------------------------------------------------------------------- */

typedef struct {
	uint16_t cursor_x;    /* current X position in pixels */
	uint16_t cursor_y;    /* current Y position in pixels */
	uint8_t  initialized; /* display initialization flag */
	uint8_t  display_on;  /* display on/off flag */
} SSD1306_State_t;

/* Global driver state */
extern SSD1306_State_t ssd1306_state;

/* Framebuffer (graphics RAM shadow) */
extern uint8_t ssd1306_buffer[SSD1306_BUFFER_SIZE];

/* Dirty flags bitmap (1 bit per framebuffer byte) */
extern uint8_t ssd1306_dirty_flags[SSD1306_DIRTY_FLAGS_SIZE];

/* --------------------------------------------------------------------------
 * Internal functions
 * -------------------------------------------------------------------------- */

/* Send a single command byte to SSD1306 */
void ssd1306_write_command(uint8_t byte);

void ssd1306_write_command_ex(uint8_t cmd, uint8_t param);

/* Send a data block to SSD1306 (one packet, up to display width) */
void ssd1306_write_data(uint8_t *buffer, uint16_t buff_size);

/* Set current page (row of 8 pixels) */
void ssd1306_set_page(uint8_t page);

/* Set current column */
void ssd1306_set_column(uint8_t column);

/* Send a block of framebuffer data and clear corresponding dirty flags */
void ssd1306_send_block(uint8_t x, uint8_t page, uint32_t n_bytes);

/* Iterate over string and decode next character (according to charset) */
const char* ssd1306_next_char(const char *str, uint16_t *out_codepoint);

/* Calculate pixel width of text for fixed-width font */
uint16_t ssd1306_calc_text_width(const char *str, uint8_t font_width);

/* Map Unicode codepoint to font index (encoding-dependent) */
uint8_t ssd1306_map_char_unicode(uint16_t codepoint);


#endif // SSD1306_PRIV_H
