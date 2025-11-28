/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

/*
 * File: ssd1306_ui.h
 * Purpose: High-level UI helpers for SSD1306-based displays.
 *
 * Provides:
 * - Headers with alignment and line styles
 * - Vertical menus with scrolling and selection highlight
 * - Progress bars with optional percentage display
 * - Scrollbars (vertical and horizontal)
 */

#ifndef SSD1306_UI_H
#define SSD1306_UI_H


#include <stdint.h>
#include <stdbool.h>

#include "ssd1306_conf.h"
#include "ssd1306_fonts.h"
#include "ssd1306.h"

/* =====================================================================
 * Common layout types
 * ===================================================================== */

/* Inner padding (content offsets inside element) */
typedef struct {
	uint8_t	top;
	uint8_t	bottom;
	uint8_t	left;
	uint8_t	right;
} SSD1306_Padding_t;

/* Outer margin (space around element) */
typedef struct {
	uint8_t	top;
	uint8_t	bottom;
	uint8_t	left;
	uint8_t	right;
} SSD1306_Margin_t;

/* =====================================================================
 * Header
 * ===================================================================== */

/* Horizontal text alignment */
typedef enum {
	SSD1306_TEXT_ALIGN_LEFT,   /* Left aligned */
	SSD1306_TEXT_ALIGN_CENTER, /* Centered */
	SSD1306_TEXT_ALIGN_RIGHT   /* Right aligned */
} SSD1306_TextAlign_t;

/* Header style (lines around/below header text) */
typedef enum {
	SSD1306_HEADER_NONE,        /* No lines */
	SSD1306_HEADER_LINE,        /* Single line below header */
	SSD1306_HEADER_DOUBLE_LINE  /* Two lines below header */
} SSD1306_HeaderStyle_t;

/* Header descriptor */
typedef struct {
	const char				*text;      /* Header text */
	const SSD1306_Font_t	*font;      /* Header font */
	SSD1306_TextAlign_t		alignment;  /* Text alignment */
	SSD1306_HeaderStyle_t	style;      /* Line style below header */
	SSD1306_Padding_t		padding;    /* Inner padding */
	uint8_t					width;      /* Calculated text width */
	uint8_t					height;     /* Calculated full header height */
} SSD1306_Header_t;

/* Initialize header descriptor */
SSD1306_Header_t ssd1306_ui_header_init(
	const char				*text,     /* Header text */
	const SSD1306_Font_t	*font,     /* Header font */
	SSD1306_TextAlign_t		alignment, /* Text alignment */
	SSD1306_HeaderStyle_t	style,     /* Line style */
	SSD1306_Padding_t		padding   /* Inner padding */
);

/* Draw header (text + optional lines) */
void ssd1306_ui_draw_header(const SSD1306_Header_t *header);

/* =====================================================================
 * Progress bar
 * ===================================================================== */

/* Position of percentage label relative to progress bar */
typedef enum {
	SSD1306_PROGRESS_PERCENT_NONE,   /* Do not display percentage */
	SSD1306_PROGRESS_PERCENT_RIGHT,  /* Percentage to the right of bar */
	SSD1306_PROGRESS_PERCENT_BOTTOM  /* Percentage below the bar */
} SSD1306_ProgressPercentPosition_t;

/* Progress bar descriptor */
typedef struct {
	uint8_t								x;
	uint8_t								y;
	uint8_t								width;
	uint8_t								height;
	uint8_t								progress;         /* 0–100 */
	SSD1306_ProgressPercentPosition_t	percent_position; /* Where to draw percentage */
	bool								inner_padding;    /* Gap between border and fill */
	SSD1306_Padding_t					padding;          /* Outer padding for the bar */
} SSD1306_ProgressBar_t;

/* Initialize progress bar descriptor */
SSD1306_ProgressBar_t ssd1306_ui_progressbar_init(
	uint8_t								x,
	uint8_t								y,
	uint8_t								width,
	uint8_t								height,
	SSD1306_ProgressPercentPosition_t	percent_position,
	bool								inner_padding,
	SSD1306_Padding_t					padding
);

/* Draw progress bar according to current state */
void ssd1306_ui_draw_progressbar(const SSD1306_ProgressBar_t *bar);

/* Set progress value (0–100) and redraw bar */
void ssd1306_ui_progressbar_set(SSD1306_ProgressBar_t *bar, uint8_t value);

/* =====================================================================
 * Scrollbar
 * ===================================================================== */

/* Scrollbar orientation */
typedef enum {
	SSD1306_SCROLLBAR_VERTICAL,   /* Vertical scrollbar */
	SSD1306_SCROLLBAR_HORIZONTAL  /* Horizontal scrollbar */
} SSD1306_ScrollbarOrientation_t;

/* Scrollbar descriptor */
typedef struct {
	uint8_t							x;             /* Top-left X */
	uint8_t							y;             /* Top-left Y */
	uint8_t							width;         /* Scrollbar width */
	uint8_t							height;        /* Scrollbar height */
	uint16_t						total_items;   /* Total number of items */
	uint8_t							visible_items; /* Number of visible items */
	uint16_t						offset;        /* Index of first visible item */
	bool							enabled;       /* Whether scrollbar is active */
	SSD1306_ScrollbarOrientation_t	orientation;   /* Orientation */
	SSD1306_Margin_t				margin;        /* Outer margin inside reserved area */
} SSD1306_Scrollbar_t;

/* Initialize scrollbar descriptor */
SSD1306_Scrollbar_t ssd1306_ui_scrollbar_init(
	uint8_t							x,
	uint8_t							y,
	uint8_t							width,
	uint8_t							height,
	uint8_t							total_items,
	uint8_t							visible_items,
	uint8_t							offset,
	SSD1306_ScrollbarOrientation_t	orientation,
	SSD1306_Margin_t				margin
);

/* Draw scrollbar (track, arrows, slider) */
void ssd1306_ui_draw_scrollbar(const SSD1306_Scrollbar_t *bar);

/* =====================================================================
 * Menu
 * ===================================================================== */

/* Menu descriptor */
typedef struct {
	const char *const			*items;          /* Array of menu strings */
	uint8_t						total_count;     /* Number of items */
	uint8_t						selected_index;  /* Currently selected item index */
	uint8_t						visible_offset;  /* Index of first visible item */
	const SSD1306_Font_t		*font;           /* Menu font */
	const SSD1306_Header_t		*header;         /* Optional header (may be NULL) */
	uint8_t						line_spacing;    /* Extra pixels between lines */
	uint8_t						max_visible;     /* Calculated number of visible lines */
	SSD1306_TextAlign_t			alignment;       /* Text alignment for items */
	SSD1306_Padding_t			padding;         /* Inner padding for menu area */
	SSD1306_Scrollbar_t			scrollbar;       /* Scrollbar descriptor */
} SSD1306_Menu_t;

/* Helper structure for layout calculation (internal use in .c) */
typedef struct {
	uint8_t	line_height;
	uint8_t	header_height;
	uint8_t	y_offset;
	uint8_t	menu_height;
} SSD1306_MenuLayout;

/* Initialize menu descriptor */
SSD1306_Menu_t ssd1306_ui_menu_init(
	const char *const		*items,           /* Array of menu item strings */
	uint8_t					count,           /* Number of menu items */
	const SSD1306_Font_t	*font,            /* Menu font */
	const SSD1306_Header_t	*header,          /* Optional header (may be NULL) */
	uint8_t					line_spacing,     /* Line spacing in pixels */
	SSD1306_TextAlign_t		alignment,        /* Item text alignment */
	SSD1306_Padding_t		padding,          /* Inner padding for menu */
	SSD1306_Margin_t		scrollbar_margin  /* Outer margin for scrollbar area */
);

/* Draw full menu (header, items, scrollbar) */
void ssd1306_ui_draw_menu(SSD1306_Menu_t *menu);

/* Scroll menu selection up (and update internal offset as needed) */
void ssd1306_ui_menu_scroll_up(SSD1306_Menu_t *menu);

/* Scroll menu selection down (and update internal offset as needed) */
void ssd1306_ui_menu_scroll_down(SSD1306_Menu_t *menu);

#endif /* SSD1306_UI_H */
