/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

/**
 * @file ssd1306.c
 * @brief Main implementation file for the SSD1306 driver.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ssd1306_cmd.h"
#include "ssd1306_port.h"
#include "ssd1306.h"
#include "ssd1306_priv.h"
#include "ssd1306_utils.h"

/* =======================================================================
 * Display control and initialization
 * ======================================================================= */

void ssd1306_set_display_on(uint8_t on) {
	if (on == SSD1306_DISPLAY_ON) {
		ssd1306_write_command(SSD1306_CMD_DISPLAY_ON);
		ssd1306_state.display_on = 1;
	} else if (on == SSD1306_DISPLAY_OFF) {
		ssd1306_write_command(SSD1306_CMD_DISPLAY_OFF);
		ssd1306_state.display_on = 0;
	}
}

void ssd1306_set_contrast(uint8_t value) {
	ssd1306_write_command_ex(SSD1306_CMD_SET_CONTRAST, value);
}

void ssd1306_set_invert(uint8_t invert) {
	if (invert == SSD1306_INVERT_ON) {
		ssd1306_write_command(SSD1306_CMD_SET_INVERT_DISPLAY);
	} else if (invert == SSD1306_INVERT_OFF) {
		ssd1306_write_command(SSD1306_CMD_SET_NORMAL_DISPLAY);
	}
}

void ssd1306_init(void) {
	SystemCoreClockUpdate();
	ssd1306_time_init(SystemCoreClock);
	SSD1306_DELAY_MS(100);

	SSD1306_PORT_SETUP_DEFAULT();

	ssd1306_set_display_on(SSD1306_DISPLAY_OFF);

	ssd1306_write_command(SSD1306_CMD_SET_MEMORY_MODE);
	ssd1306_write_command(SSD1306_ADDR_MODE_HORIZONTAL);

	ssd1306_set_page(0);
	ssd1306_set_column(0);

#ifdef SSD1306_MIRROR_VERT
	ssd1306_write_command(SSD1306_CMD_SET_COM_OUTPUT_REMAPPED);
#else
	ssd1306_write_command(SSD1306_CMD_SET_COM_OUTPUT_NORMAL);
#endif

#ifdef SSD1306_MIRROR_HORIZ
	ssd1306_write_command(SSD1306_CMD_SET_SEGMENT_REMAP_MIRROR);
#else
	ssd1306_write_command(SSD1306_CMD_SET_SEGMENT_REMAP_NORMAL);
#endif

#ifdef SSD1306_INVERSE_COLOR
	ssd1306_set_invert(SSD1306_INVERT_ON);
#else
	ssd1306_set_invert(SSD1306_INVERT_OFF);
#endif

	ssd1306_set_contrast(0xFF);

	ssd1306_write_command(SSD1306_CMD_SET_MULTIPLEX_RATIO);
#if (SSD1306_HEIGHT == 32) || (SSD1306_HEIGHT == 64) || (SSD1306_HEIGHT == 128)
	ssd1306_write_command(0x3F);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

	ssd1306_write_command(SSD1306_CMD_DISPLAY_ALL_ON_RESUME);

	ssd1306_write_command(SSD1306_CMD_SET_DISPLAY_OFFSET);
	ssd1306_write_command(0x00);

	ssd1306_write_command(SSD1306_CMD_SET_DISPLAY_CLOCK_DIV);
	ssd1306_write_command(0x80);

	ssd1306_write_command(SSD1306_CMD_SET_PRECHARGE);
	ssd1306_write_command(0xF1);

	ssd1306_write_command(SSD1306_CMD_SET_COM_PINS);
#if (SSD1306_HEIGHT == 32) || (SSD1306_HEIGHT == 64) || (SSD1306_HEIGHT == 128)
	ssd1306_write_command(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

	ssd1306_write_command(SSD1306_CMD_SET_VCOM_DESELECT);
	ssd1306_write_command(0x40);

	ssd1306_write_command(SSD1306_CMD_SET_CHARGE_PUMP);
	ssd1306_write_command(SSD1306_CHARGE_PUMP_ENABLE);

	ssd1306_set_display_on(SSD1306_DISPLAY_ON);

	ssd1306_buffer_fill(White);
	ssd1306_flush_dirty();

	ssd1306_state.initialized = 1;
}

/* =======================================================================
 * Pixel operations and dirty flags
 * ======================================================================= */

void ssd1306_buffer_draw_pixel(uint8_t x, uint8_t y, SSD1306_COLOR_t color) {
	uint32_t page;
	uint32_t buffer_index;
	uint8_t bit_mask;
	uint8_t is_new_value;
	uint32_t dirty_index;

	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
		return;
	}

	page = y / 8u;
	buffer_index = x + page * SSD1306_WIDTH;
	bit_mask = (uint8_t)(1u << (y % 8u));

	/* Only mark dirty and touch buffer if pixel value actually changes */
	is_new_value = (uint8_t)SSD1306_BOOL_XOR(ssd1306_buffer[buffer_index] & bit_mask, color);

	if (is_new_value) {
		dirty_index = page * SSD1306_WIDTH_BYTES + x / 8u;
		ssd1306_dirty_flags[dirty_index] |= (uint8_t)(1u << (x % 8u));

		if (color == White) {
			ssd1306_buffer[buffer_index] |= bit_mask;
		} else {
			ssd1306_buffer[buffer_index] &= (uint8_t)~bit_mask;
		}
	}
}

/* =======================================================================
 * Text rendering
 * ======================================================================= */

char ssd1306_buffer_draw_char_font(char ch,
				   uint8_t x,
				   uint8_t y,
				   const SSD1306_Font_t *font,
				   SSD1306_COLOR_t color) {
	uint8_t row, col_byte, byte, bit, pixel_on;
	uint8_t font_width;
	uint8_t font_height;
	uint8_t bytes_per_row;
	uint32_t char_offset;

	if (!font || !font->data) {
		return 0;
	}
	if (ch < 32) {
		return 0;
	}

	font_width = font->width;
	font_height = font->height;
	bytes_per_row = (uint8_t)((font_width + 7u) / 8u);
	char_offset = (uint32_t)(ch - 32) * font_height * bytes_per_row;

	if ((uint16_t)x + font_width > SSD1306_WIDTH ||
	    (uint16_t)y + font_height > SSD1306_HEIGHT) {
		return 0;
	}

	for (row = 0; row < font_height; row++) {
		for (col_byte = 0; col_byte < bytes_per_row; col_byte++) {
			byte = font->data[char_offset + (uint32_t)row * bytes_per_row + col_byte];

			for (bit = 0; bit < 8u; bit++) {
				pixel_on = (uint8_t)((byte >> (7u - bit)) & 0x01u);

				/* Non-transparent glyph: background is inverse of text color */
				ssd1306_buffer_draw_pixel(
					(uint8_t)(x + (uint8_t)(col_byte * 8u) + bit),
					(uint8_t)(y + row),
					pixel_on ? color : (SSD1306_COLOR_t)!color
				);
			}
		}
	}

	return ch;
}

char ssd1306_buffer_draw_char(char ch,
			      uint8_t x,
			      uint8_t y,
			      SSD1306_COLOR_t color) {
	return ssd1306_buffer_draw_char_font(ch, x, y, SSD1306_FONT_DEFAULT, color);
}

void ssd1306_buffer_draw_string_font(const char *str,
				     uint8_t x,
				     uint8_t y,
				     const SSD1306_Font_t *font,
				     SSD1306_COLOR_t color) {
	uint16_t codepoint;
	uint8_t ch;
	const char *ptr;
	const char *next;

	if (!str || !font) {
		return;
	}

	ptr = str;

	for (;;) {
		next = ssd1306_next_char(ptr, &codepoint);
		if (next == (const char *)0) {
			break;
		}

		ptr = next;
		ch = ssd1306_map_char_unicode(codepoint);
		ssd1306_buffer_draw_char_font((char)ch, x, y, font, color);
		x = (uint8_t)(x + font->width);
	}
}

void ssd1306_buffer_draw_string(const char *str,
				uint8_t x,
				uint8_t y,
				SSD1306_COLOR_t color) {
	ssd1306_buffer_draw_string_font(str, x, y, SSD1306_FONT_DEFAULT, color);
}

void ssd1306_buffer_fill(SSD1306_COLOR_t color) {
	memset(ssd1306_buffer, (color == Black) ? 0x00 : 0xFF, sizeof(ssd1306_buffer));
	memset(ssd1306_dirty_flags, 0xFF, sizeof(ssd1306_dirty_flags));
}

void ssd1306_display_clear(void) {
	ssd1306_buffer_fill(Black);
	ssd1306_flush_dirty();
}

void ssd1306_display_fill(SSD1306_COLOR_t color) {
	ssd1306_buffer_fill(color);
	ssd1306_flush_dirty();
}

/* =======================================================================
 * Geometry primitives (lines, rectangles, circles, triangles, bitmaps)
 * ======================================================================= */

void ssd1306_buffer_draw_line(int16_t x0,
			      int16_t y0,
			      int16_t x1,
			      int16_t y1,
			      SSD1306_COLOR_t color) {
	/* Cohen–Sutherland clipping, then Bresenham line drawing */
	uint8_t out0, out1, outcode_out;
	int16_t x, y;
	int16_t dx, dy, sx, sy, err, e2;
	bool accept;

	out0 = ssd1306_geom_compute_out_code(x0, y0);
	out1 = ssd1306_geom_compute_out_code(x1, y1);
	accept = false;

	while (1) {
		if (!(out0 | out1)) {
			/* both endpoints inside */
			accept = true;
			break;
		} else if (out0 & out1) {
			/* both endpoints in the same outside region -> invisible */
			return;
		} else {
			outcode_out = out0 ? out0 : out1;

			if (outcode_out & SSD1306_CS_TOP) {
				x = x0 + (x1 - x0) * (0 - y0) / (y1 - y0);
				y = 0;
			} else if (outcode_out & SSD1306_CS_BOTTOM) {
				x = x0 + (x1 - x0) * (SSD1306_HEIGHT - 1 - y0) / (y1 - y0);
				y = SSD1306_HEIGHT - 1;
			} else if (outcode_out & SSD1306_CS_RIGHT) {
				y = y0 + (y1 - y0) * (SSD1306_WIDTH - 1 - x0) / (x1 - x0);
				x = SSD1306_WIDTH - 1;
			} else {
				/* LEFT */
				y = y0 + (y1 - y0) * (0 - x0) / (x1 - x0);
				x = 0;
			}

			if (outcode_out == out0) {
				x0 = x;
				y0 = y;
				out0 = ssd1306_geom_compute_out_code(x0, y0);
			} else {
				x1 = x;
				y1 = y;
				out1 = ssd1306_geom_compute_out_code(x1, y1);
			}
		}
	}

	if (!accept) {
		return;
	}

	/* Bresenham */
	dx = (int16_t)abs(x1 - x0);
	dy = (int16_t)-abs(y1 - y0);
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = dx + dy;

	while (1) {
		ssd1306_buffer_draw_pixel((uint8_t)x0, (uint8_t)y0, color);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = (int16_t)(2 * err);
		if (e2 >= dy) {
			err = (int16_t)(err + dy);
			x0 = (int16_t)(x0 + sx);
		}
		if (e2 <= dx) {
			err = (int16_t)(err + dx);
			y0 = (int16_t)(y0 + sy);
		}
	}
}

void ssd1306_buffer_draw_rect_xy(int16_t x0,
				 int16_t y0,
				 int16_t x1,
				 int16_t y1,
				 SSD1306_COLOR_t color) {
	int16_t t;

	/* normalize corners so that (x0,y0) is top-left and (x1,y1) bottom-right */
	if (x0 > x1) {
		t = x0; x0 = x1; x1 = t;
	}
	if (y0 > y1) {
		t = y0; y0 = y1; y1 = t;
	}

	ssd1306_buffer_draw_line(x0, y0, x1, y0, color); /* top    */
	ssd1306_buffer_draw_line(x0, y0, x0, y1, color); /* left   */
	ssd1306_buffer_draw_line(x1, y0, x1, y1, color); /* right  */
	ssd1306_buffer_draw_line(x0, y1, x1, y1, color); /* bottom */
}

void ssd1306_buffer_draw_rect(int16_t x,
			      int16_t y,
			      int16_t w,
			      int16_t h,
			      SSD1306_COLOR_t color) {
	int16_t x1, y1;

	if (w <= 0 || h <= 0) {
		return;
	}

	x1 = (int16_t)(x + w - 1);
	y1 = (int16_t)(y + h - 1);
	ssd1306_buffer_draw_rect_xy(x, y, x1, y1, color);
}

void ssd1306_buffer_fill_rect_xy(int16_t x0,
				 int16_t y0,
				 int16_t x1,
				 int16_t y1,
				 SSD1306_COLOR_t color) {
	int16_t x, y, t;

	if (x0 > x1) {
		t = x0; x0 = x1; x1 = t;
	}
	if (y0 > y1) {
		t = y0; y0 = y1; y1 = t;
	}

	for (y = y0; y <= y1; y++) {
		if (y < 0 || y >= (int16_t)SSD1306_HEIGHT) {
			continue;
		}
		for (x = x0; x <= x1; x++) {
			if (x < 0 || x >= (int16_t)SSD1306_WIDTH) {
				continue;
			}
			ssd1306_buffer_draw_pixel((uint8_t)x, (uint8_t)y, color);
		}
	}
}

void ssd1306_buffer_fill_rect(int16_t x,
			      int16_t y,
			      int16_t w,
			      int16_t h,
			      SSD1306_COLOR_t color) {
	int16_t x1, y1;

	if (w <= 0 || h <= 0) {
		return;
	}

	x1 = (int16_t)(x + w - 1);
	y1 = (int16_t)(y + h - 1);
	ssd1306_buffer_fill_rect_xy(x, y, x1, y1, color);
}

void ssd1306_buffer_draw_circle(int16_t xc,
				int16_t yc,
				int16_t r,
				SSD1306_COLOR_t color) {
	int16_t x = 0;
	int16_t y = r;
	int16_t d = (int16_t)(3 - 2 * r);

	while (y >= x) {
		ssd1306_buffer_draw_pixel((uint8_t)(xc + x), (uint8_t)(yc + y), color);
		ssd1306_buffer_draw_pixel((uint8_t)(xc - x), (uint8_t)(yc + y), color);
		ssd1306_buffer_draw_pixel((uint8_t)(xc + x), (uint8_t)(yc - y), color);
		ssd1306_buffer_draw_pixel((uint8_t)(xc - x), (uint8_t)(yc - y), color);
		ssd1306_buffer_draw_pixel((uint8_t)(xc + y), (uint8_t)(yc + x), color);
		ssd1306_buffer_draw_pixel((uint8_t)(xc - y), (uint8_t)(yc + x), color);
		ssd1306_buffer_draw_pixel((uint8_t)(xc + y), (uint8_t)(yc - x), color);
		ssd1306_buffer_draw_pixel((uint8_t)(xc - y), (uint8_t)(yc - x), color);

		x++;
		if (d > 0) {
			y--;
			d = (int16_t)(d + 4 * (x - y) + 10);
		} else {
			d = (int16_t)(d + 4 * x + 6);
		}
	}
}

void ssd1306_buffer_fill_circle(int16_t xc,
				int16_t yc,
				int16_t r,
				SSD1306_COLOR_t color) {
	int16_t x = 0;
	int16_t y = r;
	int16_t d = (int16_t)(3 - 2 * r);
	int16_t i;

	while (y >= x) {
		/* horizontal spans between circle points */
		for (i = (int16_t)(xc - x); i <= (int16_t)(xc + x); i++) {
			ssd1306_buffer_draw_pixel((uint8_t)i, (uint8_t)(yc + y), color);
			ssd1306_buffer_draw_pixel((uint8_t)i, (uint8_t)(yc - y), color);
		}
		for (i = (int16_t)(xc - y); i <= (int16_t)(xc + y); i++) {
			ssd1306_buffer_draw_pixel((uint8_t)i, (uint8_t)(yc + x), color);
			ssd1306_buffer_draw_pixel((uint8_t)i, (uint8_t)(yc - x), color);
		}

		x++;
		if (d > 0) {
			y--;
			d = (int16_t)(d + 4 * (x - y) + 10);
		} else {
			d = (int16_t)(d + 4 * x + 6);
		}
	}
}

void ssd1306_buffer_draw_bitmap(int16_t x,
				int16_t y,
				const uint8_t *image,
				int16_t width,
				int16_t height,
				SSD1306_COLOR_t color) {
	int16_t i, j;
	uint8_t byte, bit;
	SSD1306_COLOR_t px_color;
	SSD1306_COLOR_t bg_color;
	int bytes_per_row;

	if (!image || width <= 0 || height <= 0) {
		return;
	}

	bg_color = (color == White) ? Black : White;

	/* Number of bytes per scanline (MSB-first, width up to panel width) */
	bytes_per_row = (int)((width + 7) / 8);

	for (j = 0; j < height; j++) {
		if (y + j < 0 || y + j >= (int16_t)SSD1306_HEIGHT) {
			continue;
		}
		for (i = 0; i < width; i++) {
			if (x + i < 0 || x + i >= (int16_t)SSD1306_WIDTH) {
				continue;
			}
			byte = image[j * bytes_per_row + (i / 8)];
			bit = (uint8_t)(7 - (i % 8));
			px_color = (byte & (uint8_t)(1u << bit)) ? color : bg_color;
			ssd1306_buffer_draw_pixel((uint8_t)(x + i), (uint8_t)(y + j), px_color);
		}
	}
}

void ssd1306_buffer_draw_triangle(int x0,
				  int y0,
				  int x1,
				  int y1,
				  int x2,
				  int y2,
				  SSD1306_COLOR_t color) {
	ssd1306_buffer_draw_line((int16_t)x0, (int16_t)y0, (int16_t)x1, (int16_t)y1, color);
	ssd1306_buffer_draw_line((int16_t)x1, (int16_t)y1, (int16_t)x2, (int16_t)y2, color);
	ssd1306_buffer_draw_line((int16_t)x2, (int16_t)y2, (int16_t)x0, (int16_t)y0, color);
}

void ssd1306_buffer_fill_triangle(int x0,
				  int y0,
				  int x1,
				  int y1,
				  int x2,
				  int y2,
				  SSD1306_COLOR_t color) {
	int total_height;
	int i;
	int segment_height;
	int ax, bx;
	bool second_half;
	float alpha, beta;

	/* sort vertices so that y0 <= y1 <= y2 */
	if (y0 > y1) { ssd1306_util_swap_int(&y0, &y1); ssd1306_util_swap_int(&x0, &x1); }
	if (y1 > y2) { ssd1306_util_swap_int(&y1, &y2); ssd1306_util_swap_int(&x1, &x2); }
	if (y0 > y1) { ssd1306_util_swap_int(&y0, &y1); ssd1306_util_swap_int(&x0, &x1); }

	total_height = y2 - y0;

	for (i = 0; i < total_height; i++) {
		second_half = (bool)(i > y1 - y0 || y1 == y0);
		segment_height = second_half ? (y2 - y1) : (y1 - y0);
		if (segment_height == 0) {
			continue;
		}

		alpha = (float)i / (float)total_height;
		beta = (float)(i - (second_half ? (y1 - y0) : 0)) / (float)segment_height;

		ax = x0 + (int)((x2 - x0) * alpha);
		bx = second_half ?
			(x1 + (int)((x2 - x1) * beta)) :
			(x0 + (int)((x1 - x0) * beta));

		if (ax > bx) {
			ssd1306_util_swap_int(&ax, &bx);
		}
		ssd1306_buffer_draw_line((int16_t)ax, (int16_t)(y0 + i),
					 (int16_t)bx, (int16_t)(y0 + i), color);
	}
}

/* =======================================================================
 * Dirty-region flush
 * ======================================================================= */

void ssd1306_flush_dirty(void) {
	/* Each bit in ssd1306_dirty_flags marks one vertical byte (8 pixels).
	 * We walk over all bits, compressing consecutive dirty bytes on the same
	 * page into blocks (batch_start + batch_len), then send those blocks.
	 */
	uint32_t bit_index = 0;				/* current bit index (0..WIDTH*HEIGHT/8), each bit = 1 vertical byte */
	uint32_t batch_start = 0;			/* start x (in pixels) of current dirty block */
	uint32_t batch_len = 0;				/* length of current block in bytes */
	uint32_t dirty_square_index = 0;	/* index in dirty_flags (each entry = 8x8 block) */
	uint8_t dirty_byte = 0;				/* each bit marks a dirty vertical byte */
	uint8_t last_byte_dirty = 0;		/* previous byte in block was dirty */
	uint8_t batch_found = 0;			/* a block to send is ready */
	uint8_t page = 0;					/* current display page */
	uint8_t i;

	if (!ssd1306_state.initialized) {
		return;
	}

	do {
		/* Process one page at a time: we don't cross page boundary in a single block */
		last_byte_dirty = 0;
		page = (uint8_t)(bit_index / SSD1306_WIDTH);

		do {
			dirty_square_index = bit_index / 8u;

			if (dirty_square_index >= SSD1306_DIRTY_FLAGS_SIZE) {
				break;
			}

			dirty_byte = ssd1306_dirty_flags[dirty_square_index];

			if (dirty_byte == 0xFFu) {
				/* all 8 bytes in this vertical group are dirty */
				if (!last_byte_dirty) {
					batch_start = bit_index % SSD1306_WIDTH;
				}

				batch_len += 8u;
				bit_index += 8u;
				last_byte_dirty = 1;
			} else {
				/* check individual bits within this 8x8 block */
				for (i = (uint8_t)(bit_index % 8u); i < 8u; i++) {
					if ((dirty_byte >> i) & 0x01u) {
						/* dirty bit */
						if (!last_byte_dirty) {
							batch_start = bit_index % SSD1306_WIDTH;
						}

						batch_len++;
						last_byte_dirty = 1;
					} else {
						/* clean bit */
						if (last_byte_dirty) {
							batch_found = 1;
							bit_index++;
							break;
						}
						last_byte_dirty = 0;
					}

					bit_index++;

					/* moved to next 8x8 block */
					if (dirty_square_index < (bit_index / 8u)) {
						break;
					}
				}
			}

			if (batch_found) {
				break;
			}

		} while (page == (bit_index / SSD1306_WIDTH));

		/* if we ended page with pending block */
		if (last_byte_dirty) {
			batch_found = 1;
		}

		if (batch_found) {
			page = (uint8_t)(dirty_square_index / SSD1306_WIDTH_BYTES);
			ssd1306_set_page(page);
			ssd1306_set_column((uint8_t)batch_start);
			ssd1306_send_block((uint8_t)batch_start, page, batch_len);
		}

		batch_found = 0;
		batch_len = 0;
		last_byte_dirty = 0;

	} while (bit_index < SSD1306_DIRTY_FLAGS_SIZE * 8u - 1u);
	/* bit_index iterates over all bits of ssd1306_dirty_flags */
}
