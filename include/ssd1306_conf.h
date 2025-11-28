/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

/**
 * @file ssd1306_conf.h
 * @brief Configuration for SSD1306 display library.
 *
 * This file is intended to be the *only* user-edited configuration file.
 * Here you select display type/geometry, I2C address/port and fonts.
 */

#ifndef SSD1306_CONF_H
#define SSD1306_CONF_H

#include <stdint.h>

/* =====================================================================
 * MCU family selection (used by port layer)
 * ===================================================================== */

/*
 * Select your MCU family here.
 * Only one of these should be defined.
 */
#define SSD1306_MCU_STM32F1
// #define SSD1306_MCU_STM32L1

/* Integration hooks (delay, watchdog)
 * Implement these functions in user code; the macros below map library
 * calls to your implementations.
 */

void ssd1306_time_delay_ms(uint32_t ms);
void ssd1306_port_watchdog_feed(void);

#define SSD1306_DELAY_MS(ms)    ssd1306_time_delay_ms(ms)
#define SSD1306_FEED_WATCHDOG   ssd1306_port_watchdog_feed


/* =====================================================================
 * I2C interface
 * ===================================================================== */

/* 8-bit I2C address (7-bit 0x3C shifted left by 1) */
#define SSD1306_I2C_ADDR      (0x3C << 1)

/*
 * I2C instance used to talk to the display.
 * For STM32 this is typically I2C1 / I2C2 / I2C3 (I2C_TypeDef*).
 * Adjust to match your board wiring.
 */
#define SSD1306_I2C_PORT      I2C2

/* I2C operation timeout, in milliseconds */
#define SSD1306_I2C_TIMEOUT   100


/* =====================================================================
 * Character encoding and fonts
 * ===================================================================== */

/* Source code / font encoding */
#define SSD1306_CHARSET_UTF8
/* Alternative options (uncomment if implemented in your project): */
// #define SSD1306_CHARSET_WIN1251
// #define SSD1306_CHARSET_ISO8859_1

/*
 * Included fonts.
 * Keep only those you actually use to save flash.
 */
#define SSD1306_INCLUDE_FONT_8x8
// #define SSD1306_INCLUDE_FONT_7x11
#define SSD1306_INCLUDE_FONT_7x14
// #define SSD1306_INCLUDE_FONT_11x21
// #define SSD1306_INCLUDE_FONT_16x30

/* Possible values: SSD1306_FONT_8x8,
 *                  SSD1306_FONT_7x11,
 *                  SSD1306_FONT_7x14,
 *                  SSD1306_FONT_11x21,
 *                  SSD1306_FONT_16x30
 */
#define SSD1306_FONT_DEFAULT   SSD1306_FONT_7x14


/* =====================================================================
 * Display type and geometry
 * ===================================================================== */

/*
 * Select your display type here.
 * One of:  SSD1306_DISPLAY_128x64,
 * 			SSD1306_DISPLAY_64x32,
 * 			SSD1306_DISPLAY_64x32_FAKE
 */
#define SSD1306_DISPLAY_TYPE   SSD1306_DISPLAY_128x64


/* =====================================================================
 * Optional display transforms
 * ===================================================================== */

/* Flip vertically (top <-> bottom) */
#define SSD1306_MIRROR_VERT

/* Flip horizontally (left <-> right) */
#define SSD1306_MIRROR_HORIZ

/* Invert colors (black <-> white) */
// #define SSD1306_INVERSE_COLOR


/* =====================================================================
 * UI configuration
 * ===================================================================== */

/*
 * Default auto-flush behavior for ssd1306_ui_* drawing functions.
 * 1 = each draw call flushes dirty regions immediately.
 * 0 = draw functions only update the RAM buffer; you must call
 *     ssd1306_flush_dirty() manually where appropriate.
 */
#define SSD1306_UI_AUTO_FLUSH_DEFAULT   1


#endif /* SSD1306_CONF_H */
