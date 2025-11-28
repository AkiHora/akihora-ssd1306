/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

#include "ssd1306_priv.h"
#include "ssd1306_utils.h"
#include "ssd1306_cmd.h"
#include "ssd1306_port.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Global driver data
 * -------------------------------------------------------------------------- */

SSD1306_State_t ssd1306_state;
uint8_t ssd1306_buffer[SSD1306_BUFFER_SIZE];
uint8_t ssd1306_dirty_flags[SSD1306_DIRTY_FLAGS_SIZE];

/* --------------------------------------------------------------------------
 * Low-level write helpers
 * -------------------------------------------------------------------------- */

void ssd1306_write_command(uint8_t byte) {
	uint8_t pkt[2];

	pkt[0] = 0x00;	/* control: Co=0, D/C#=0 */
	pkt[1] = byte;

	(void)ssd1306_port_i2c_write(pkt, 2);
}

void ssd1306_write_command_ex(uint8_t cmd, uint8_t param) {
	ssd1306_write_command(cmd);
	ssd1306_write_command(param);
}

void ssd1306_write_data(uint8_t *buffer, uint16_t size) {
	uint8_t	pkt[1 + SSD1306_WIDTH];
	uint16_t i;
	uint16_t n = size;

	if (n > SSD1306_WIDTH) {
		n = SSD1306_WIDTH;
	}

	pkt[0] = 0x40; /* control: Co=0, D/C#=1 */
	for (i = 0; i < n; i++) {
		pkt[1 + i] = buffer[i];
	}

	(void)ssd1306_port_i2c_write(pkt, (uint16_t)(1 + n));
}

void ssd1306_set_page(uint8_t page) {
	uint8_t offset_page = (uint8_t)(page + SSD1306_PAGE_OFFSET);

	ssd1306_write_command((uint8_t)(SSD1306_CMD_SET_PAGE_START | (offset_page & 0x07)));
}

void ssd1306_set_column(uint8_t column) {
	uint8_t offset_column = (uint8_t)(column + SSD1306_X_OFFSET);

	ssd1306_write_command((uint8_t)(SSD1306_CMD_SET_LOW_COLUMN  | (offset_column & 0x0F)));
	ssd1306_write_command((uint8_t)(SSD1306_CMD_SET_HIGH_COLUMN | ((offset_column >> 4) & 0x0F)));
}

/* --------------------------------------------------------------------------
 * Framebuffer / dirty flags
 * -------------------------------------------------------------------------- */

void ssd1306_send_block(uint8_t x, uint8_t page, uint32_t n_bytes) {
	uint32_t n_bytes_actual;
	uint32_t page_index;
	uint32_t dirty_index;
	uint32_t i;

	/* Clamp block size so we don't write past the right screen edge */
	n_bytes_actual = SSD1306_MIN(n_bytes, (uint32_t)(SSD1306_WIDTH - x));
	page_index = (uint32_t)page * (uint32_t)SSD1306_WIDTH;
	dirty_index = (uint32_t)(x + page * SSD1306_WIDTH) / 8U;

	ssd1306_write_data(&ssd1306_buffer[x + page_index], (uint16_t)n_bytes_actual);

	/* Clear dirty flags in the left partial byte of the range */
	if ((x % 8U) != 0U) {
		ssd1306_dirty_flags[dirty_index] &= (uint8_t)(0xFFu << (x % 8U));
	}

	/* Clear dirty flags in fully covered bytes of the range */
	for (i = 0; i < (n_bytes_actual / 8U); i++) {
		ssd1306_dirty_flags[dirty_index + i] = 0x00;
	}

	/* Clear dirty flags in the right partial byte of the range */
	if (((x + n_bytes_actual) % 8U) != 0U) {
		ssd1306_dirty_flags[dirty_index + (n_bytes_actual / 8U)] &=
			(uint8_t)(0xFFu << ((x + n_bytes_actual) % 8U));
	}

	ssd1306_state.cursor_x = (uint16_t)(x + n_bytes_actual);
}

/* --------------------------------------------------------------------------
 * Text / charset helpers
 * -------------------------------------------------------------------------- */

const char* ssd1306_next_char(const char *str, uint16_t *out_codepoint) {
	uint8_t byte;
	uint8_t b1;
	uint8_t b2;
	uint16_t codepoint;

	if (!str || !*str) {
		return NULL;
	}

#if defined(SSD1306_CHARSET_UTF8)

	byte = (uint8_t)*str++;
	codepoint = 0;

	if (byte < 0x80u) {
		codepoint = byte;
	} else if ((byte & 0xE0u) == 0xC0u && (str[0] & 0xC0u) == 0x80u) {
		codepoint = (uint16_t)(((byte & 0x1Fu) << 6) | ((uint8_t)*str++ & 0x3Fu));
	} else if ((byte & 0xF0u) == 0xE0u &&
	           (str[0] & 0xC0u) == 0x80u &&
	           (str[1] & 0xC0u) == 0x80u) {
		b1 = (uint8_t)*str++;
		b2 = (uint8_t)*str++;
		codepoint = (uint16_t)(((byte & 0x0Fu) << 12) |
		                       ((b1 & 0x3Fu) << 6) |
		                       (b2 & 0x3Fu));
	} else {
		codepoint = (uint16_t)'?';
	}

#elif defined(SSD1306_CHARSET_WIN1251)

	*out_codepoint = (uint8_t)*str++;
	return str;

#elif defined(SSD1306_CHARSET_ISO8859_1)

	*out_codepoint = (uint8_t)*str++;
	return str;

#else

	byte = (uint8_t)*str++;
	*out_codepoint = (uint16_t)((byte >= 0x20u && byte <= 0x7Fu) ? byte : '?');
	return str;

#endif

	*out_codepoint = codepoint;
	return str;
}

uint16_t ssd1306_calc_text_width(const char *str, uint8_t font_width) {
	uint16_t codepoint;
	uint16_t width = 0;

	for (; (str = ssd1306_next_char(str, &codepoint)) != NULL; ) {
		(void)codepoint;
		width = (uint16_t)(width + font_width);
	}

	return width;
}

uint8_t ssd1306_map_char_unicode(uint16_t codepoint) {
#if defined(SSD1306_CHARSET_UTF8)
	/* ASCII */
	if (codepoint < 128u) {
		return (uint8_t)codepoint;
	}

	/* U+0410..U+044F (А..я) -> 0xC0..0xFF */
	if (codepoint >= 0x0410u && codepoint <= 0x044Fu) {
		return (uint8_t)(0xC0u + (codepoint - 0x0410u));
	}

	return (uint8_t)'?';

#elif defined(SSD1306_CHARSET_WIN1251)

	if (codepoint < 256u) {
		return (uint8_t)codepoint;
	}
	return (uint8_t)'?';

#elif defined(SSD1306_CHARSET_ISO8859_1)

	return (uint8_t)((codepoint >= 0x20u && codepoint <= 0x7Fu) ? codepoint : '?');

#else

	return (uint8_t)((codepoint >= 0x20u) ? codepoint : '?');

#endif
}
