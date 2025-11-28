/**
 * SSD1306 demo: basic showcase for drawing primitives and UI helpers.
 *
 * Assumptions:
 *  - ssd1306_init() is called before running this demo (e.g. in main)
 *  - port / I2C / delay hooks are configured via ssd1306_conf.h
 */

#include <stdio.h>
#include <string.h>

#include "ssd1306_conf.h"   /* Display configuration: width, height, hooks */
#include "ssd1306_priv.h"   /* Internal SSD1306 definitions (buffer, macros) */
#include "ssd1306_utils.h"
#include "ssd1306.h"
#include "ssd1306_ui.h"

/* ======================================================================
 * Simple showcase demo (geometry, UI widgets, text)
 * ====================================================================== */

static void ssd1306_demo_header(void);
static void ssd1306_demo_progressbar(void);
static void ssd1306_demo_fill_ascii(void);
static void ssd1306_demo_menu_basic(void);

/* Public entry point: run full demo sequence */
void ssd1306_example_showcase(void) {
	uint32_t delay_ms = 2000;

	ssd1306_demo_header();
	SSD1306_DELAY_MS(delay_ms);

	ssd1306_demo_progressbar();
	SSD1306_DELAY_MS(delay_ms);

//	ssd1306_demo_fill_ascii();
//	SSD1306_DELAY_MS(delay_ms);

	ssd1306_demo_menu_basic();
	SSD1306_DELAY_MS(delay_ms);
}

/* === Scenes === */

static void ssd1306_demo_header(void) {
	SSD1306_Padding_t pad;
	SSD1306_Header_t header;

	ssd1306_display_clear();

	pad.top    = 14;
	pad.bottom = 4;
	pad.left   = 4;
	pad.right  = 4;

	ssd1306_buffer_draw_rect_xy(0, 0, SSD1306_WIDTH-1, SSD1306_HEIGHT-1, White);

	header = ssd1306_ui_header_init(
		"SSD1306 demo",
		SSD1306_FONT_DEFAULT,
		SSD1306_TEXT_ALIGN_CENTER,
		SSD1306_HEADER_DOUBLE_LINE,
		pad
	);

	ssd1306_ui_draw_header(&header);

	pad.top    += 20;

	header = ssd1306_ui_header_init(
			"(c) 2025 AkiHora",
			SSD1306_FONT_DEFAULT,
			SSD1306_TEXT_ALIGN_CENTER,
			SSD1306_HEADER_NONE,
			pad
		);

	ssd1306_ui_draw_header(&header);

}

static void ssd1306_demo_progressbar(void) {
	SSD1306_Padding_t pad;
	SSD1306_ProgressBar_t bar;
	uint8_t value;
	SSD1306_Header_t header;

	ssd1306_display_clear();

	pad.top    = 4;
	pad.bottom = 0;
	pad.left   = 0;
	pad.right  = 0;

	header = ssd1306_ui_header_init(
		"Progress bar",
		SSD1306_FONT_DEFAULT,
		SSD1306_TEXT_ALIGN_CENTER,
		SSD1306_HEADER_LINE,
		pad
	);
	ssd1306_ui_draw_header(&header);

	pad.top    = 0;
	pad.bottom = 0;
	pad.left   = 0;
	pad.right  = 0;

	bar = ssd1306_ui_progressbar_init(
		10,                               /* x */
		30,								  /* y */
		(uint8_t)(SSD1306_WIDTH - 20),    /* width */
		16,                               /* height */
		SSD1306_PROGRESS_PERCENT_BOTTOM,
		true,                             /* inner_padding */
		pad
	);

	for (value = 0; value <= 100; value = (uint8_t)(value + 5U)) {
		ssd1306_ui_progressbar_set(&bar, value);
		ssd1306_ui_draw_progressbar(&bar);
	}
}

static void ssd1306_demo_fill_ascii(void) {
	const SSD1306_Font_t *font = SSD1306_FONT_DEFAULT;
	uint8_t ch;
	uint16_t j;
	uint16_t i;

	ssd1306_buffer_fill(Black);
	ch = (uint8_t)' ';

	/* Fill screen with a grid of ASCII characters */
	for (j = 0; j + font->height <= SSD1306_HEIGHT; j = (uint16_t)(j + font->height)) {
		for (i = 0; i + font->width <= SSD1306_WIDTH; i = (uint16_t)(i + font->width)) {
			ssd1306_buffer_draw_char_font(
				(char)ch,
				(uint8_t)i,
				(uint8_t)j,
				font,
				White
			);
			ch++;
		}
	}

	ssd1306_flush_dirty();
}

static void ssd1306_demo_menu_basic(void) {
	const char *menu_items[5];
	SSD1306_Padding_t header_pad;
	SSD1306_Padding_t menu_pad;
	SSD1306_Margin_t  scroll_margin;
	SSD1306_Header_t  header;
	SSD1306_Menu_t    menu;
	uint8_t i;

	/* Menu items (example menu similar to test code) */
	menu_items[0] = "Settings";
	menu_items[1] = "Information";
	menu_items[2] = "Save";
	menu_items[3] = "Language";
	menu_items[4] = "Exit";

	ssd1306_buffer_fill(Black);

	/* Header padding */
	header_pad.top    = 0;
	header_pad.bottom = 1;
	header_pad.left   = 4;
	header_pad.right  = 4;

	header = ssd1306_ui_header_init(
		"Menu",
		SSD1306_FONT_DEFAULT,
		SSD1306_TEXT_ALIGN_CENTER,
		SSD1306_HEADER_LINE,
		header_pad
	);

	/* Menu padding:
	 * top = 0 → menu starts immediately below header.height
	 */
	menu_pad.top    = 0;
	menu_pad.bottom = 0;
	menu_pad.left   = 4;
	menu_pad.right  = 4;

	/* Scrollbar outer margin inside its reserved area */
	scroll_margin.top    = 1;
	scroll_margin.bottom = 1;
	scroll_margin.left   = 1;
	scroll_margin.right  = 1;

	menu = ssd1306_ui_menu_init(
		menu_items,
		(uint8_t)(sizeof(menu_items) / sizeof(menu_items[0])),
//		SSD1306_FONT_DEFAULT, // Can use it
		SSD1306_FONT_7x14,
		&header,
		1,                        /* line_spacing in pixels */
		SSD1306_TEXT_ALIGN_LEFT,  /* item alignment */
		menu_pad,
		scroll_margin
	);

	/* Initial drawing */
	ssd1306_ui_draw_menu(&menu);
	SSD1306_DELAY_MS(800);

	/* Scroll down through all items */
	for (i = 0; i < menu.total_count; i++) {
		ssd1306_ui_menu_scroll_down(&menu);
		ssd1306_ui_draw_menu(&menu);
		SSD1306_DELAY_MS(150);
	}

	/* Scroll up back to the first item */
	for (i = 0; i < menu.total_count; i++) {
		ssd1306_ui_menu_scroll_up(&menu);
		ssd1306_ui_draw_menu(&menu);
		SSD1306_DELAY_MS(150);
	}
}

static float test_fps_torn_drawing(void) {
    uint32_t start;
    uint32_t end;
    float fps;
    uint32_t iter;

    start = ssd1306_time_ticks_ms();
    end   = start;
    fps   = 0.0f;
    iter  = 0U;

    do {
        memset(ssd1306_buffer, (iter % 2U) ? White : Black, sizeof(ssd1306_buffer));
        memset(ssd1306_dirty_flags, 0xAA, sizeof(ssd1306_dirty_flags));
        ssd1306_flush_dirty();

        fps  += 1.0f;
        iter += 1U;
        end   = ssd1306_time_ticks_ms();
        SSD1306_FEED_WATCHDOG();
    } while ((end - start) < 3000U);

    fps = fps / ((end - start) / 1000.0f);
    return fps;
}

static float test_fps_whole_drawing(void) {
    uint32_t start;
    uint32_t end;
    float fps;
    uint32_t iter;

    start = ssd1306_time_ticks_ms();
    end   = start;
    fps   = 0.0f;
    iter  = 0U;

    do {
        ssd1306_buffer_fill((iter % 2U) ? White : Black);
        ssd1306_flush_dirty();

        fps  += 1.0f;
        iter += 1U;
        end   = ssd1306_time_ticks_ms();
        SSD1306_FEED_WATCHDOG();
    } while ((end - start) < 5000U);

    fps = fps / ((end - start) / 1000.0f);
    return fps;
}

static float _test_fps_symbol(const SSD1306_Font_t *font) {
    uint32_t start;
    uint32_t end;
    uint32_t iter;
    float fps;
    char ch;

    start = ssd1306_time_ticks_ms();
    end   = start;
    iter  = 0U;
    fps   = 0.0f;
    ch    = (char)0x20;

    do {
        if (ch < (char)0x70) ch = (char)(ch + 1); else ch = (char)0x20;

        ssd1306_buffer_draw_char_font(
            ch,
            (uint8_t)((SSD1306_WIDTH  - font->width)  / 2),
            (uint8_t)((SSD1306_HEIGHT - font->height) / 2),
            font,
            Black
        );
        ssd1306_flush_dirty();

        fps  += 1.0f;
        iter += 1U;
        end   = ssd1306_time_ticks_ms();
        SSD1306_FEED_WATCHDOG();
    } while ((end - start) < 3000U);

    fps = fps / ((end - start) / 1000.0f);
    return fps;
}

static void test_fps_symbols(void) {
    char buff[64];
    const SSD1306_Font_t *fonts_pointers[3];
    float fpss[3];
    uint8_t counter;
    uint16_t y_pix;
    uint8_t i;

    counter = 0U;
    ssd1306_buffer_fill(White);

#ifdef SSD1306_INCLUDE_FONT_16x30
    fpss[counter] = _test_fps_symbol(&font_16x30);
    fonts_pointers[counter] = &font_16x30;
    ssd1306_buffer_fill(White);
    counter++;
#endif

#ifdef SSD1306_INCLUDE_FONT_11x21
    fpss[counter] = _test_fps_symbol(&font_11x21);
    fonts_pointers[counter] = &font_11x21;
    ssd1306_buffer_fill(White);
    counter++;
#endif

#ifdef SSD1306_INCLUDE_FONT_8x8
    fpss[counter] = _test_fps_symbol(&font_8x8);
    fonts_pointers[counter] = &font_8x8;
    ssd1306_buffer_fill(White);
    counter++;
#endif

    y_pix = 2U;
    for (i = 0U; i < counter; i++) {
    	SSD1306_FEED_WATCHDOG();
        /* snprintf оставляю — обычно доступен и в Keil; при желании можно заменить на простой форматтер */
        (void)snprintf(buff, sizeof(buff), "~%.1f FPS", fpss[i]);
        ssd1306_buffer_draw_string_font(buff, 8, y_pix, fonts_pointers[i], Black);
        y_pix = (uint16_t)(y_pix + fonts_pointers[i]->height + 2U);
    }

    ssd1306_flush_dirty();
}

static void test_fps_full_display_drawing(void) {
    float torn_drawing_fps;
    float whole_drawing_fps;
    char buff[64];

    torn_drawing_fps  = test_fps_torn_drawing();
    whole_drawing_fps = test_fps_whole_drawing();

    ssd1306_buffer_draw_string_font("Torn drawing:", 0, 0, SSD1306_FONT_DEFAULT, Black);
    (void)snprintf(buff, sizeof(buff), "~%.1f FPS", torn_drawing_fps);
    ssd1306_buffer_draw_string_font(buff, 8, (uint8_t)((SSD1306_FONT_DEFAULT->height + 1) * 1), SSD1306_FONT_DEFAULT, Black);

    ssd1306_buffer_draw_string_font("Whole drawing:", 0, (uint8_t)((SSD1306_FONT_DEFAULT->height + 1) * 2), SSD1306_FONT_DEFAULT, Black);
    (void)snprintf(buff, sizeof(buff), "~%.1f FPS", whole_drawing_fps);
    ssd1306_buffer_draw_string_font(buff, 8, (uint8_t)((SSD1306_FONT_DEFAULT->height + 1) * 3), SSD1306_FONT_DEFAULT, Black);

    ssd1306_flush_dirty();
}
