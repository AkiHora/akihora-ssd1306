/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

#include "ssd1306_ui.h"
#include "ssd1306_priv.h"


/* =======================================================================
 * Local prototypes
 * ======================================================================= */
static SSD1306_MenuLayout _ssd1306_ui_calc_layout(const SSD1306_Menu_t *menu);
static void ssd1306_ui_buffer_draw_menu_item(const SSD1306_Menu_t *menu, const char *text,
				       uint8_t y, uint8_t selected,
				       uint8_t left_margin, uint8_t right_margin);
static void percent_to_str(uint8_t v, char out[6]);

/* Set runtime auto-flush behavior for ssd1306_ui_draw_* functions.
 * This overrides SSD1306_UI_AUTO_FLUSH_DEFAULT from ssd1306_conf.h.
 */
static uint8_t ssd1306_ui_auto_flush = SSD1306_UI_AUTO_FLUSH_DEFAULT;


void ssd1306_ui_set_auto_flush(uint8_t enabled) {
	ssd1306_ui_auto_flush = enabled ? 1u : 0u;
}


/* === Заголовок === */
SSD1306_Header_t ssd1306_ui_header_init(const char* text,
                                        const SSD1306_Font_t* font,
                                        SSD1306_TextAlign_t alignment,
                                        SSD1306_HeaderStyle_t style,
                                        SSD1306_Padding_t padding) {
    SSD1306_Header_t header;
    header.text = text;
    header.font = font;
    header.alignment = alignment;
    header.style = style;
    header.padding = padding;
    header.width = 0;
    header.height = 0;

    if (text && text[0]) {
        header.width = ssd1306_calc_text_width(text, font->width);
        header.height = (uint8_t)(padding.top + font->height + padding.bottom);

        if (style == SSD1306_HEADER_LINE) {
            header.height = (uint8_t)(header.height + 2);
        } else if (style == SSD1306_HEADER_DOUBLE_LINE) {
            header.height = (uint8_t)(header.height + 4);
        }
    }
    return header;
}

void ssd1306_ui_draw_header(const SSD1306_Header_t *header) {
	uint16_t text_width;
	int16_t x;
	int16_t y;

	if (!header || !header->text || header->text[0] == '\0') {
		return;
	}

	text_width = header->width;

	if (header->alignment == SSD1306_TEXT_ALIGN_CENTER) {
		x = (int16_t)((SSD1306_WIDTH - (int16_t)text_width) / 2);
	} else if (header->alignment == SSD1306_TEXT_ALIGN_RIGHT) {
		x = (int16_t)(SSD1306_WIDTH - header->padding.right - (int16_t)text_width);
	} else {
		x = (int16_t)header->padding.left;
	}

	y = (int16_t)header->padding.top;

	ssd1306_buffer_draw_string_font(header->text,
					(uint8_t)x,
					(uint8_t)y,
					header->font,
					White);

	y = (int16_t)(y + header->font->height);

	if (header->style == SSD1306_HEADER_LINE) {
		ssd1306_buffer_draw_line(header->padding.left,
					 (uint8_t)(y + 1),
					 (uint8_t)(SSD1306_WIDTH - header->padding.right - 1u),
					 (uint8_t)(y + 1),
					 White);
	} else if (header->style == SSD1306_HEADER_DOUBLE_LINE) {
		ssd1306_buffer_draw_line(header->padding.left,
					 (uint8_t)y,
					 (uint8_t)(SSD1306_WIDTH - header->padding.right - 1u),
					 (uint8_t)y,
					 White);
		ssd1306_buffer_draw_line(header->padding.left,
					 (uint8_t)(y + 2),
					 (uint8_t)(SSD1306_WIDTH - header->padding.right - 1u),
					 (uint8_t)(y + 2),
					 White);
	}

	if (ssd1306_ui_auto_flush) ssd1306_flush_dirty();
}


/* =======================================================================
 * Scrollbar
 * ======================================================================= */
SSD1306_Scrollbar_t ssd1306_ui_scrollbar_init(uint8_t x, uint8_t y,
					      uint8_t width, uint8_t height,
					      uint8_t total_items,
					      uint8_t visible_items,
					      uint8_t offset,
					      SSD1306_ScrollbarOrientation_t orientation,
					      SSD1306_Margin_t margin) {
	SSD1306_Scrollbar_t bar;

	bar.x = x;
	bar.y = y;
	bar.width = width;
	bar.height = height;
	bar.total_items = total_items;
	bar.visible_items = visible_items;
	bar.offset = offset;
	bar.orientation = orientation;
	bar.margin = margin;
	bar.enabled = true;

	return bar;
}

void ssd1306_ui_draw_scrollbar(const SSD1306_Scrollbar_t *bar) {
	uint8_t inner_x, inner_y;
	uint8_t inner_width, inner_height;
	uint8_t center_x, center_y;
	uint8_t usable_top, usable_height, slider_height, slider_y;
	uint8_t usable_left, usable_width, slider_width, slider_x;

	if (!bar || !bar->enabled || bar->width == 0u || bar->height == 0u) {
		return;
	}

	inner_x = (uint8_t)(bar->x + bar->margin.left);
	inner_y = (uint8_t)(bar->y + bar->margin.top);

	/* Clear entire scrollbar area */
	ssd1306_buffer_fill_rect(bar->x, bar->y, bar->width, bar->height, Black);

	if (bar->orientation == SSD1306_SCROLLBAR_VERTICAL) {
		/* Vertical scrollbar */
		inner_width = 5u;
		inner_height = bar->height;
		center_x = (uint8_t)(inner_x + inner_width / 2u);

		/* Vertical line */
		ssd1306_buffer_draw_line(center_x,
					 inner_y,
					 center_x,
					 (uint8_t)(inner_y + inner_height - 1u),
					 White);

		/* Up arrow */
		ssd1306_buffer_fill_triangle(
			(uint8_t)(center_x - 3u),
			(uint8_t)(inner_y + 5u),
			(uint8_t)(center_x + 3u),
			(uint8_t)(inner_y + 5u),
			center_x,
			inner_y,
			White);

		/* Down arrow */
		ssd1306_buffer_fill_triangle(
			(uint8_t)(center_x - 2u),
			(uint8_t)(inner_y + inner_height - 5u),
			(uint8_t)(center_x + 2u),
			(uint8_t)(inner_y + inner_height - 5u),
			center_x,
			(uint8_t)(inner_y + inner_height - 1u),
			White);

		/* Slider, 3 pixels wide */
		usable_top = (uint8_t)(inner_y + 7u);
		usable_height = (uint8_t)(inner_height - 14u);
		slider_height = (uint8_t)((usable_height * bar->visible_items) / bar->total_items);
		if (slider_height < 4u) {
			slider_height = 4u;
		}
		slider_y = (uint8_t)(usable_top +
				     (usable_height * bar->offset) / bar->total_items);

		ssd1306_buffer_fill_rect(
			(uint8_t)(center_x - 1u),
			slider_y,
			3u,
			slider_height,
			White);

	} else {
		/* Horizontal scrollbar */
		inner_width = bar->width;
		inner_height = 5u;
		center_y = (uint8_t)(inner_y + inner_height / 2u);

		/* Horizontal line */
		ssd1306_buffer_draw_line(
			inner_x,
			center_y,
			(uint8_t)(inner_x + inner_width - 1u),
			center_y,
			White);

		/* Left arrow */
		ssd1306_buffer_fill_triangle(
			(uint8_t)(inner_x - 1u + 4u),
			(uint8_t)(center_y - 2u),
			(uint8_t)(inner_x - 1u + 4u),
			(uint8_t)(center_y + 2u),
			(uint8_t)(inner_x - 1u),
			center_y,
			White);

		/* Right arrow */
		ssd1306_buffer_fill_triangle(
			(uint8_t)(inner_x + 1u + inner_width - 6u),
			(uint8_t)(center_y - 2u),
			(uint8_t)(inner_x + 1u + inner_width - 6u),
			(uint8_t)(center_y + 2u),
			(uint8_t)(inner_x + 1u + inner_width - 1u),
			center_y,
			White);

		/* Slider, 3 pixels high */
		usable_left = (uint8_t)(inner_x + 7u);
		usable_width = (uint8_t)(inner_width - 14u);
		slider_width = (uint8_t)((usable_width * bar->visible_items) / bar->total_items);
		if (slider_width < 2u) {
			slider_width = 2u;
		}
		slider_x = (uint8_t)(usable_left +
				     (usable_width * bar->offset) / bar->total_items);

		ssd1306_buffer_fill_rect(
			slider_x,
			(uint8_t)(center_y - 1u),
			slider_width,
			3u,
			White);
	}

	if (ssd1306_ui_auto_flush) ssd1306_flush_dirty();
}

/* =======================================================================
 * Menu
 * ======================================================================= */

static SSD1306_MenuLayout _ssd1306_ui_calc_layout(const SSD1306_Menu_t *menu) {
	SSD1306_MenuLayout layout = {0};

	layout.line_height   = (uint8_t)(menu->font->height + menu->line_spacing);
	layout.header_height = menu->header ? menu->header->height : 0;
	layout.y_offset      = (uint8_t)(menu->padding.top + layout.header_height);
	layout.menu_height   = (uint8_t)(layout.line_height * menu->max_visible);

	return layout;
}


static void ssd1306_ui_buffer_draw_menu_item(const SSD1306_Menu_t *menu,
				       const char *text,
				       uint8_t y,
				       uint8_t selected,
				       uint8_t left_margin,
				       uint8_t right_margin) {
	uint8_t line_height;
	SSD1306_COLOR_t fg;
	SSD1306_COLOR_t bg;
	int16_t x;
	uint16_t text_width;

	if (!text || !text[0]) {
		return;
	}

	line_height = (uint8_t)(menu->font->height + menu->line_spacing);
	fg = selected ? Black : White;
	bg = selected ? White : Black;

	ssd1306_buffer_fill_rect(
		left_margin,
		y,
		(uint8_t)(right_margin - left_margin),
		line_height,
		bg
	);

	text_width = ssd1306_calc_text_width(text, menu->font->width);

	if (menu->alignment == SSD1306_TEXT_ALIGN_CENTER) {
		x = (int16_t)((SSD1306_WIDTH - (int16_t)text_width) / 2);
	} else if (menu->alignment == SSD1306_TEXT_ALIGN_RIGHT) {
		x = (int16_t)((int16_t)right_margin - (int16_t)text_width - 1);
	} else {
		x = (int16_t)left_margin;
	}

	ssd1306_buffer_draw_string_font(
		text,
		(uint8_t)x,
		y,
		menu->font,
		fg
	);


}

SSD1306_Menu_t ssd1306_ui_menu_init(const char *const *items,
				    uint8_t count,
				    const SSD1306_Font_t *font,
				    const SSD1306_Header_t *header,
				    uint8_t line_spacing,
				    SSD1306_TextAlign_t alignment,
				    SSD1306_Padding_t padding,
				    SSD1306_Margin_t scrollbar_margin) {
	SSD1306_Menu_t menu;
	uint8_t header_height;
	uint8_t line_height;
	uint8_t available_height;
	uint8_t bar_total_width;
	uint8_t bar_x;

	menu.items          = items;
	menu.total_count    = count;
	menu.selected_index = 0;
	menu.visible_offset = 0;
	menu.font           = font;
	menu.header         = header;
	menu.line_spacing   = line_spacing;
	menu.alignment      = alignment;
	menu.padding        = padding;

	header_height    = header ? header->height : 0;
	line_height      = (uint8_t)(font->height + line_spacing);
	available_height = (uint8_t)(SSD1306_HEIGHT
				     - padding.top
				     - padding.bottom
				     - header_height);
	menu.max_visible = (uint8_t)(available_height / line_height);

	menu.scrollbar.enabled = (count > menu.max_visible) ? true : false;

	if (menu.scrollbar.enabled) {
		bar_total_width = (uint8_t)(5 + scrollbar_margin.left + scrollbar_margin.right);

		if (alignment == SSD1306_TEXT_ALIGN_RIGHT) {
			bar_x = padding.left;
		} else {
			bar_x = (uint8_t)(SSD1306_WIDTH - bar_total_width - padding.right);
		}

		menu.scrollbar.x             = bar_x;
		menu.scrollbar.y             = (uint8_t)(padding.top + header_height);
		menu.scrollbar.width         = bar_total_width;
		menu.scrollbar.height        = (uint8_t)(line_height * menu.max_visible);
		menu.scrollbar.total_items   = count;
		menu.scrollbar.visible_items = menu.max_visible;
		menu.scrollbar.offset        = 0;
		menu.scrollbar.orientation   = SSD1306_SCROLLBAR_VERTICAL;
		menu.scrollbar.margin        = scrollbar_margin;
	} else {
		/* Disabled scrollbar: reset geometry and state */
		menu.scrollbar.x             = 0;
		menu.scrollbar.y             = 0;
		menu.scrollbar.width         = 0;
		menu.scrollbar.height        = 0;
		menu.scrollbar.total_items   = 0;
		menu.scrollbar.visible_items = 0;
		menu.scrollbar.offset        = 0;
		menu.scrollbar.orientation   = SSD1306_SCROLLBAR_VERTICAL;
		menu.scrollbar.margin.top    = 0;
		menu.scrollbar.margin.bottom = 0;
		menu.scrollbar.margin.left   = 0;
		menu.scrollbar.margin.right  = 0;
		menu.scrollbar.enabled       = false;
	}

	return menu;
}

void ssd1306_ui_draw_menu(SSD1306_Menu_t *menu) {
	SSD1306_MenuLayout layout;
	uint8_t menu_x;
	uint8_t menu_width;
	uint8_t left_margin;
	uint8_t right_margin;
	uint8_t i;

	if (!menu) {
		return;
	}

	layout    = _ssd1306_ui_calc_layout(menu);
	menu_x    = menu->padding.left;
	menu_width = (uint8_t)(SSD1306_WIDTH
			       - menu->padding.left
			       - menu->padding.right);

	ssd1306_buffer_fill_rect(
		menu_x,
		layout.y_offset,
		menu_width,
		layout.menu_height,
		Black
	);

	left_margin  = menu->padding.left;
	right_margin = (uint8_t)(SSD1306_WIDTH - menu->padding.right);

	if (menu->scrollbar.enabled) {
		if (menu->alignment == SSD1306_TEXT_ALIGN_RIGHT) {
			left_margin = (uint8_t)(left_margin + menu->scrollbar.width);
		} else {
			right_margin = (uint8_t)(right_margin - menu->scrollbar.width);
		}
	}

	for (i = 0; i < menu->max_visible; i++) {
		uint8_t item_index;
		uint8_t selected;
		uint8_t y;
		const char *text;

		item_index = (uint8_t)(menu->visible_offset + i);
		if (item_index >= menu->total_count) {
			break;
		}

		selected = (item_index == menu->selected_index) ? 1u : 0u;
		y = (uint8_t)(layout.y_offset + (uint8_t)(i * layout.line_height));
		text = menu->items[item_index];

		ssd1306_ui_buffer_draw_menu_item(
			menu,
			text,
			y,
			selected,
			left_margin,
			right_margin
		);
	}

	if (menu->scrollbar.enabled) {
		menu->scrollbar.offset = menu->visible_offset;
		ssd1306_ui_draw_scrollbar(&menu->scrollbar);
	}

	ssd1306_ui_draw_header(menu->header);
	if (ssd1306_ui_auto_flush) ssd1306_flush_dirty();
}


/* =======================================================================
 * Scroll helpers
 * ======================================================================= */

void ssd1306_ui_scrollbar_scroll_up(SSD1306_Scrollbar_t *bar) {
	if (!bar || !bar->enabled) {
		return;
	}

	if (bar->offset > 0) {
		bar->offset--;
	}
}

void ssd1306_ui_scrollbar_scroll_down(SSD1306_Scrollbar_t *bar) {
	if (!bar || !bar->enabled) {
		return;
	}

	if ((uint8_t)(bar->offset + bar->visible_items) < bar->total_items) {
		bar->offset++;
	}
}

void ssd1306_ui_menu_scroll_up(SSD1306_Menu_t *menu) {
	if (!menu) {
		return;
	}

	if (menu->selected_index > 0) {
		menu->selected_index--;

		if (menu->selected_index < menu->visible_offset) {
			menu->visible_offset--;

			if (menu->scrollbar.enabled) {
				ssd1306_ui_scrollbar_scroll_up(&menu->scrollbar);
			}
		}
	}

}

void ssd1306_ui_menu_scroll_down(SSD1306_Menu_t *menu) {
	if (!menu) {
		return;
	}

	if ((uint8_t)(menu->selected_index + 1) < menu->total_count) {
		menu->selected_index++;

		if (menu->selected_index >= (uint8_t)(menu->visible_offset + menu->max_visible)) {
			menu->visible_offset++;

			if (menu->scrollbar.enabled) {
				ssd1306_ui_scrollbar_scroll_down(&menu->scrollbar);
			}
		}
	}

}

/* =======================================================================
 * Progress bar
 * ======================================================================= */

SSD1306_ProgressBar_t ssd1306_ui_progressbar_init(uint8_t x,
						  uint8_t y,
						  uint8_t width,
						  uint8_t height,
						  SSD1306_ProgressPercentPosition_t percent_position,
						  bool inner_padding,
						  SSD1306_Padding_t padding) {
	SSD1306_ProgressBar_t bar;

	bar.x                = x;
	bar.y                = y;
	bar.width            = width;
	bar.height           = height;
	bar.progress         = 0;
	bar.percent_position = percent_position;
	bar.inner_padding    = inner_padding;
	bar.padding          = padding;

	return bar;
}

void ssd1306_ui_draw_progressbar(const SSD1306_ProgressBar_t *bar) {
	uint8_t outer_x;
	uint8_t outer_y;
	uint8_t outer_width;
	uint8_t outer_height;
	uint8_t pad;
	uint8_t fill_x;
	uint8_t fill_y;
	uint8_t fill_w;
	uint8_t fill_h;
	char percent_str[6];
	uint16_t text_width;
	int16_t px;
	int16_t py;

	if (!bar) {
		return;
	}

	/* Outer dimensions with padding */
	outer_x = (uint8_t)(bar->x + bar->padding.left);
	outer_y = (uint8_t)(bar->y + bar->padding.top);
	outer_width  = (uint8_t)(bar->width  - bar->padding.left - bar->padding.right);
	outer_height = (uint8_t)(bar->height - bar->padding.top  - bar->padding.bottom);

	if (outer_width == 0 || outer_height == 0) {
		return;
	}

	/* Background and border */
	ssd1306_buffer_fill_rect(outer_x, outer_y, outer_width, outer_height, Black);
	ssd1306_buffer_draw_rect(outer_x, outer_y, outer_width, outer_height, White);

	/* Fill area */
	pad = bar->inner_padding ? 1u : 0u;
	fill_x = (uint8_t)(outer_x + 1u + pad);
	fill_y = (uint8_t)(outer_y + 1u + pad);
	fill_w = (uint8_t)(((outer_width - (uint8_t)(2u * pad) - 2u) * bar->progress) / 100u);
	fill_h = (uint8_t)(outer_height - (uint8_t)(2u * (1u + pad)));

	ssd1306_buffer_fill_rect(fill_x, fill_y, fill_w, fill_h, White);

	/* Percent text */
	if (bar->percent_position != SSD1306_PROGRESS_PERCENT_NONE) {
		percent_to_str(bar->progress, percent_str);
		text_width = ssd1306_calc_text_width(percent_str, SSD1306_FONT_DEFAULT->width);

		px = 0;
		py = 0;

		if (bar->percent_position == SSD1306_PROGRESS_PERCENT_RIGHT) {
			px = (int16_t)(outer_x + outer_width + 3);
			py = (int16_t)(outer_y +
				       (outer_height - SSD1306_FONT_DEFAULT->height) / 2);
		} else if (bar->percent_position == SSD1306_PROGRESS_PERCENT_BOTTOM) {
			px = (int16_t)(outer_x + ((int16_t)outer_width - (int16_t)text_width) / 2);
			py = (int16_t)(outer_y + outer_height + 1);
		}

		ssd1306_buffer_fill_rect(
			(uint8_t)(px - 1),
			(uint8_t)(py - 1),
			(uint8_t)(text_width + 2),
			(uint8_t)(SSD1306_FONT_DEFAULT->height + 2),
			Black
		);

		ssd1306_buffer_draw_string_font(
			percent_str,
			(uint8_t)px,
			(uint8_t)py,
			SSD1306_FONT_DEFAULT,
			White
		);
	}

	if (ssd1306_ui_auto_flush) ssd1306_flush_dirty();
}

void ssd1306_ui_progressbar_set(SSD1306_ProgressBar_t *bar, uint8_t value) {
	if (!bar) {
		return;
	}

	if (value > 100u) {
		value = 100u;
	}

	bar->progress = value;
}

/* =======================================================================
 * Helpers
 * ======================================================================= */

/* Format "NN%" into buffer of length 6 (enough for "100%") */
static void percent_to_str(uint8_t v, char out[6]) {
	if (v == 100u) {
		out[0] = '1';
		out[1] = '0';
		out[2] = '0';
		out[3] = '%';
		out[4] = '\0';
	} else {
		uint8_t d1 = (uint8_t)(v / 10u);
		uint8_t d0 = (uint8_t)(v % 10u);

		if (d1) {
			out[0] = (char)('0' + d1);
			out[1] = (char)('0' + d0);
			out[2] = '%';
			out[3] = '\0';
		} else {
			out[0] = (char)('0' + d0);
			out[1] = '%';
			out[2] = '\0';
		}
	}
}
