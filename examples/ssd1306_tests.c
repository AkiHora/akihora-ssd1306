/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

#include <string.h>
#include <stdio.h>

#include "ssd1306_conf.h"
#include "ssd1306_utils.h"
#include "ssd1306.h"
#include "ssd1306_priv.h"
#include "ssd1306_fonts.h"
#include "ssd1306_tests.h"
#include "ssd1306_images.h"
#include "ssd1306_ui.h"

static void ssd1306_test_progressbar(uint8_t width, uint8_t height,
                              SSD1306_ProgressPercentPosition_t percent_position,
                              bool inner_padding,
                              SSD1306_Padding_t padding)
{
    SSD1306_ProgressBar_t bar;
    uint8_t i;

    bar = ssd1306_ui_progressbar_init(
        10,                /* x */
        30,                /* y */
        width,             /* width */
        height,            /* height */
        percent_position,  /* позиция процентов */
        inner_padding,
        padding
    );

    /* Обновляем прогресс в цикле 0..100 */
    for (i = 0; i <= 100; i++) {
        ssd1306_ui_progressbar_set(&bar, i);
        ssd1306_ui_draw_progressbar(&bar);
        SSD1306_FEED_WATCHDOG();
    }
}

static void ssd1306_test_menu(const SSD1306_Font_t* font,
                       uint8_t count,
                       uint8_t line_spacing,
                       SSD1306_TextAlign_t alignment,
                       const char* text,
                       SSD1306_Padding_t menu_padding,
                       SSD1306_Padding_t header_padding,
                       SSD1306_Margin_t scroll_margin)
{
    uint8_t i;
    const char* menu_items[7];
    SSD1306_Header_t header;
    SSD1306_Menu_t menu;

    /* Пример пунктов меню */
    menu_items[0] = "Настройки";
    menu_items[1] = "Информация";
    menu_items[2] = "Яркость";
    menu_items[3] = "Контраст";
    menu_items[4] = "Сброс";
    menu_items[5] = "Сохранить";
    menu_items[6] = "Выход";

    header = ssd1306_ui_header_init(
        text,
		SSD1306_FONT_DEFAULT,
        SSD1306_TEXT_ALIGN_CENTER,
        SSD1306_HEADER_LINE,
        header_padding
    );

    menu = ssd1306_ui_menu_init(
        menu_items,
        count,
        SSD1306_FONT_DEFAULT,
        &header,
        line_spacing,          /* межстрочный интервал */
        alignment,
        menu_padding,          /* паддинг меню */
        scroll_margin          /* паддинг скролл-бара */
    );

    ssd1306_ui_draw_menu(&menu);
    SSD1306_DELAY_MS(1000);

    for (i = 0; i < count; i++) {
        ssd1306_ui_menu_scroll_down(&menu);
        ssd1306_ui_draw_menu(&menu);
        SSD1306_DELAY_MS(100);
    }
    for (i = 0; i < count; i++) {
        ssd1306_ui_menu_scroll_up(&menu);
        ssd1306_ui_draw_menu(&menu);
        SSD1306_DELAY_MS(100);
    }
}

static void ssd1306_test_header(void) {
    SSD1306_Padding_t header_padding = {0};
    SSD1306_Header_t header;

    ssd1306_buffer_fill(Black);

    header_padding.top    = 2;
    header_padding.bottom = 2;
    header_padding.left   = 4;
    header_padding.right  = 4;

    header = ssd1306_ui_header_init(
        "Заголовок",
        SSD1306_FONT_DEFAULT,
        SSD1306_TEXT_ALIGN_CENTER,
        SSD1306_HEADER_DOUBLE_LINE,
        header_padding
    );
    ssd1306_ui_draw_header(&header);

    header_padding.top    = 22;
    header_padding.bottom = 2;
    header_padding.left   = 15;
    header_padding.right  = 15;

    header = ssd1306_ui_header_init(
        "Заголовок",
        SSD1306_FONT_DEFAULT,
        SSD1306_TEXT_ALIGN_CENTER,
        SSD1306_HEADER_LINE,
        header_padding
    );
    ssd1306_ui_draw_header(&header);
}

static void ssd1306_test_scrollbar(void) {
    SSD1306_Margin_t margin = {0};
    SSD1306_Scrollbar_t bar;

    margin.top    = 1;
    margin.bottom = 1;
    margin.left   = 1;
    margin.right  = 1;

    bar = ssd1306_ui_scrollbar_init(
        120,  /* x */
        8,    /* y */
        5,    /* width */
        48,   /* height */
        10,   /* total_items */
        4,    /* visible_items */
        3,    /* offset (начиная с 3-го элемента) */
        SSD1306_SCROLLBAR_VERTICAL,
        margin
    );

    ssd1306_buffer_fill(Black);
    ssd1306_ui_draw_scrollbar(&bar);
    ssd1306_flush_dirty();
}

static void ssd1306_test_progressbar_full(void) {
    SSD1306_Padding_t pad0;
    SSD1306_Padding_t padR;

    pad0.top = 0; pad0.bottom = 0; pad0.left = 0; pad0.right = 0;
    padR = pad0; padR.right = 30;

    ssd1306_test_progressbar(100, 10, SSD1306_PROGRESS_PERCENT_NONE,   0, pad0);
    ssd1306_buffer_fill(Black);
    ssd1306_test_progressbar(100, 10, SSD1306_PROGRESS_PERCENT_BOTTOM, 1, pad0);
    ssd1306_buffer_fill(Black);
    ssd1306_test_progressbar(100, 10, SSD1306_PROGRESS_PERCENT_RIGHT,  1, padR);

}



static void ssd1306_test_fill_disp_by_symbols(uint8_t first_ch, const SSD1306_Font_t* font) {
    uint8_t ch;
    uint16_t j;
    uint16_t i;

    ssd1306_buffer_fill(Black);
    ch = first_ch;

    for (j = 0; j < (uint16_t)(SSD1306_HEIGHT - font->height); j += font->height) {
        for (i = 0; i < (uint16_t)(SSD1306_WIDTH - font->width); i += font->width) {
            ssd1306_buffer_draw_char_font(ch, (uint8_t)i, (uint8_t)j, font, White);
            ch++;
        }
    }
    ssd1306_flush_dirty();
}

static void ssd1306_test_menu_full(void) {
    SSD1306_Padding_t p0;     /* {0,0,0,0} */
    SSD1306_Padding_t pA;     /* {0,1,5,5} или {0,0,5,5} */
    SSD1306_Padding_t pB;     /* {8,8,0,0} */
    SSD1306_Padding_t pC;     /* {8,1,5,5} */
    SSD1306_Margin_t  m0;     /* {0,0,0,0} */
    SSD1306_Margin_t  mA;     /* {0,0,1,1} */

    /* инициализации «вручную» */
    p0.top = 0; p0.bottom = 0; p0.left = 0; p0.right = 0;
    m0.top = 0; m0.bottom = 0; m0.left = 0; m0.right = 0;

    ssd1306_buffer_fill(Black);

    if (SSD1306_HEIGHT > 100) {
        /* 1) ... , (0,0,0,0), (0,0,0,0), (0,0,0,0) */
        ssd1306_test_menu(SSD1306_FONT_DEFAULT, 3, 0, SSD1306_TEXT_ALIGN_LEFT,
                          "Меню", p0, p0, m0);

        /* 2) вторые padding = {0,1,5,5} */
        pA = p0; pA.bottom = 1; pA.left = 5; pA.right = 5;
        ssd1306_test_menu(SSD1306_FONT_DEFAULT, 5, 1, SSD1306_TEXT_ALIGN_CENTER,
                          "Меню", p0, pA, m0);

        /* 3) margin = {0,0,1,1} */
        mA = m0; mA.left = 1; mA.right = 1;
        ssd1306_test_menu(SSD1306_FONT_DEFAULT, 7, 1, SSD1306_TEXT_ALIGN_RIGHT,
                          "", p0, pA, mA);

        ssd1306_buffer_draw_string_font("Доп. информация", 0, 0, &font_8x8, White);
        ssd1306_buffer_draw_string_font("Доп. информация", 0, SSD1306_HEIGHT - 9, &font_8x8, White);
        ssd1306_flush_dirty();
        SSD1306_DELAY_MS(500);

        /* 4) первые padding = {8,8,0,0}, вторые = {8,1,5,5}, margin = {0,0,1,1} */
        pB.top = 8; pB.bottom = 8; pB.left = 0; pB.right = 0;
        pC = pA; pC.top = 8; /* из {0,1,5,5} -> {8,1,5,5} */
        ssd1306_test_menu(SSD1306_FONT_DEFAULT, 7, 1, SSD1306_TEXT_ALIGN_RIGHT,
                          "", pB, pC, mA);
    } else {
        /* вторые padding = {0,0,5,5} */
        pA = p0; pA.left = 5; pA.right = 5;
        ssd1306_test_menu(SSD1306_FONT_DEFAULT, 7, 0, SSD1306_TEXT_ALIGN_LEFT,
                          "", p0, pA, m0);
    }
}



static void test_figures_and_lines(void) {
    ssd1306_buffer_fill(Black);
	ssd1306_buffer_draw_rect(10, 10, 20, 30, White);
	ssd1306_buffer_draw_rect_xy(12, 12, 32, 42, White);
	ssd1306_buffer_fill_rect(40, 10, 20, 30, White);
	ssd1306_buffer_draw_circle(80, 20, 10, White);
	ssd1306_buffer_draw_circle(80, 20, 12, White);
	ssd1306_buffer_draw_circle(80, 20, 5, White);
	ssd1306_buffer_draw_circle(80, 20, 20, White);
	ssd1306_buffer_fill_circle(80, 40, 10, White);
	ssd1306_buffer_draw_triangle(2, 2, 50, 50, 9, 45, White);
	ssd1306_buffer_fill_triangle(50, 52, 60, 50, 60, 60, White);
	ssd1306_flush_dirty();
}

static void test_logo(void) {
    ssd1306_buffer_draw_bitmap(0, 0, hots_logo_64x64, IMG_LOGO_WIDTH, IMG_LOGO_HEIGHT, White);
    ssd1306_buffer_draw_bitmap(IMG_LOGO_WIDTH, 0, hots_logo_64x64, IMG_LOGO_WIDTH, IMG_LOGO_HEIGHT, White);
    ssd1306_flush_dirty();
}

static void draw_mouse_face(void) {
    int cx;
    int cy;
    int r_head;
    int r_ear;
    int r_eye;
    int r_nose;

    cx = SSD1306_WIDTH / 2;
    cy = SSD1306_HEIGHT / 2;

    r_head = SSD1306_HEIGHT / 3;
    r_ear  = SSD1306_HEIGHT / 7;
    r_eye  = SSD1306_HEIGHT / 10;
    r_nose = SSD1306_HEIGHT / 24;

    ssd1306_buffer_fill(Black);

    /* Уши */
    ssd1306_buffer_fill_circle(cx - r_head + r_ear / 2, cy - r_head, r_ear, White);
    ssd1306_buffer_fill_circle(cx + r_head - r_ear / 2, cy - r_head, r_ear, White);

    /* Голова */
    ssd1306_buffer_fill_circle(cx, cy, r_head, White);

    /* Глаза */
    ssd1306_buffer_fill_circle(cx - r_head / 3, cy - r_head / 5, r_eye, Black);
    ssd1306_buffer_fill_circle(cx + r_head / 3, cy - r_head / 5, r_eye, Black);

    /* Нос */
    ssd1306_buffer_fill_circle(cx, cy + r_head / 3, r_nose, Black);

    ssd1306_flush_dirty();
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

void ssd1306_full_test(void) {
    uint32_t delay_ms;
    delay_ms = 2000; /* Задержка между тестами, мс */

    ssd1306_test_scrollbar();
    ssd1306_test_header();
    SSD1306_DELAY_MS(delay_ms);

    test_figures_and_lines();
    SSD1306_DELAY_MS(delay_ms);

    ssd1306_test_menu_full();

/*  SSD1306_DELAY_MS(delay_ms); */
    draw_mouse_face();
    SSD1306_DELAY_MS(delay_ms);
/*  test_logo(); */
/*  SSD1306_DELAY_MS(delay_ms); */
    test_fps_symbols();
    SSD1306_DELAY_MS(delay_ms);
    test_fps_full_display_drawing();
    SSD1306_DELAY_MS(delay_ms);
    ssd1306_test_fill_disp_by_symbols(' ', SSD1306_FONT_DEFAULT);
    SSD1306_DELAY_MS(delay_ms);
    ssd1306_test_progressbar_full();
    SSD1306_DELAY_MS(delay_ms);
    test_logo();
}



void ssd1306_custom_test(void) {
    ssd1306_full_test();
//	ssd1306_buffer_fill(Black);

//	ssd1306_buffer_draw_rect_xy(0, 0, SSD1306_WIDTH-1, SSD1306_HEIGHT-1, White);
//	ssd1306_flush_dirty();
}

