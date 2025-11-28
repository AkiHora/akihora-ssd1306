/**
 * SSD1306 demo: basic showcase for drawing primitives and UI helpers.
 *
 * Assumptions:
 *  - ssd1306_init() is called before running this demo (e.g. in main)
 *  - port / I2C / delay hooks are configured via ssd1306_conf.h
 */

#include "ssd1306_conf.h"   /* Display configuration: width, height, hooks */
#include "ssd1306_priv.h"   /* Internal SSD1306 definitions (buffer, macros) */
#include "ssd1306.h"
#include "ssd1306_ui.h"

/* ======================================================================
 * Simple showcase demo (geometry, UI widgets, text)
 * ====================================================================== */

static void ssd1306_demo_header(void);
static void ssd1306_demo_figures(void);
static void ssd1306_demo_scrollbar(void);
static void ssd1306_demo_progressbar(void);
static void ssd1306_demo_fill_ascii(void);
static void ssd1306_demo_menu_basic(void);

/* Public entry point: run full demo sequence */
void ssd1306_example_showcase(void)
{
	uint32_t delay_ms = 2000;

	ssd1306_demo_header();
	SSD1306_DELAY_MS(delay_ms);

	ssd1306_demo_figures();
	SSD1306_DELAY_MS(delay_ms);

	ssd1306_demo_progressbar();
	SSD1306_DELAY_MS(delay_ms);

	ssd1306_demo_fill_ascii();
	SSD1306_DELAY_MS(delay_ms);

	ssd1306_demo_menu_basic();
	SSD1306_DELAY_MS(delay_ms);
}

/* === Scenes === */

static void ssd1306_demo_header(void)
{
	SSD1306_Padding_t pad;
	SSD1306_Header_t header;

	ssd1306_display_clear();

	pad.top    = 4;
	pad.bottom = 4;
	pad.left   = 4;
	pad.right  = 4;

	header = ssd1306_ui_header_init(
		"SSD1306 demo",
		SSD1306_FONT_DEFAULT,
		SSD1306_TEXT_ALIGN_CENTER,
		SSD1306_HEADER_DOUBLE_LINE,
		pad
	);
	ssd1306_ui_draw_header(&header);
}

static void ssd1306_demo_figures(void)
{
	uint8_t cx, cy;

	ssd1306_buffer_fill(Black);

	/* --- Left column: rectangles --- */

	/* Outline rectangle */
	ssd1306_buffer_draw_rect(
		4,     /* x */
		4,     /* y */
		26,    /* width */
		14,    /* height */
		White
	);

	/* Filled rectangle inside */
	ssd1306_buffer_fill_rect(
		6,     /* x */
		6,     /* y */
		22,    /* width */
		10,    /* height */
		White
	);

	/* Two "shelf" rectangles below */
	ssd1306_buffer_draw_rect(
		4,
		24,
		26,
		10,
		White
	);

	ssd1306_buffer_fill_rect(
		4,
		38,
		26,
		10,
		White
	);

	/* --- Center: circles --- */

	cx = (uint8_t)(SSD1306_WIDTH / 2);
	cy = 16;

	/* Outer circle */
	ssd1306_buffer_draw_circle(cx, cy, 12, White);
	/* Inner circle */
	ssd1306_buffer_draw_circle(cx, cy, 8, White);
	/* Filled circle below */
	ssd1306_buffer_fill_circle(cx, (uint8_t)(cy + 20), 8, White);

	/* --- Right column: triangles --- */

	/* Outline triangle */
	ssd1306_buffer_draw_triangle(
		(uint8_t)(SSD1306_WIDTH - 24), 6,   /* x1,y1 */
		(uint8_t)(SSD1306_WIDTH - 6),  6,   /* x2,y2 */
		(uint8_t)(SSD1306_WIDTH - 15), 20,  /* x3,y3 */
		White
	);

	/* Filled triangle below */
	ssd1306_buffer_fill_triangle(
		(uint8_t)(SSD1306_WIDTH - 24), 30,
		(uint8_t)(SSD1306_WIDTH - 6),  30,
		(uint8_t)(SSD1306_WIDTH - 15), 46,
		White
	);

	ssd1306_flush_dirty();
}

static void ssd1306_demo_scrollbar(void)
{
	SSD1306_Margin_t margin;
	SSD1306_Scrollbar_t bar;

	margin.top    = 1;
	margin.bottom = 1;
	margin.left   = 1;
	margin.right  = 1;

	bar = ssd1306_ui_scrollbar_init(
		(uint8_t)(SSD1306_WIDTH - 8), /* x */
		8,                            /* y */
		5,                            /* width */
		48,                           /* height */
		10,                           /* total_items */
		4,                            /* visible_items */
		3,                            /* offset */
		SSD1306_SCROLLBAR_VERTICAL,
		margin
	);

	ssd1306_buffer_fill(Black);
	ssd1306_ui_draw_scrollbar(&bar);
	/* draw_scrollbar will flush if UI auto-flush is enabled */
}

static void ssd1306_demo_progressbar(void)
{
	SSD1306_Padding_t pad;
	SSD1306_ProgressBar_t bar;
	uint8_t value;

	pad.top    = 0;
	pad.bottom = 0;
	pad.left   = 0;
	pad.right  = 0;

	ssd1306_buffer_fill(Black);
	ssd1306_flush_dirty();

	bar = ssd1306_ui_progressbar_init(
		10,                               /* x */
		(uint8_t)(SSD1306_HEIGHT / 2 - 8),/* y */
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

static void ssd1306_demo_fill_ascii(void)
{
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

static void ssd1306_demo_menu_basic(void)
{
	const char *menu_items[7];
	SSD1306_Padding_t header_pad;
	SSD1306_Padding_t menu_pad;
	SSD1306_Margin_t  scroll_margin;
	SSD1306_Header_t  header;
	SSD1306_Menu_t    menu;
	uint8_t i;

	/* Menu items (example menu similar to test code) */
	menu_items[0] = "Настройки";
	menu_items[1] = "Информация";
	menu_items[2] = "Яркость";
	menu_items[3] = "Контраст";
	menu_items[4] = "Сброс";
	menu_items[5] = "Сохранить";
	menu_items[6] = "Выход";

	ssd1306_buffer_fill(Black);

	/* Header padding */
	header_pad.top    = 0;
	header_pad.bottom = 1;
	header_pad.left   = 4;
	header_pad.right  = 4;

	header = ssd1306_ui_header_init(
		"Меню",
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
//		SSD1306_FONT_DEFAULT,
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
