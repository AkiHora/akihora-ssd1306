/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

#ifndef THIRD_PARTY_SSD1306_INC_SSD1306_FONTS_H_
#define THIRD_PARTY_SSD1306_INC_SSD1306_FONTS_H_

#include "ssd1306_conf.h"

// Структура шрифта для дисплея
typedef struct {
	const uint8_t width;           // Ширина символа в пикселях
	const uint8_t height;          // Высота символа в пикселях
	const uint8_t *const data;     // Указатель на массив битмапов символов
} SSD1306_Font_t;


#ifdef SSD1306_INCLUDE_FONT_8x8
	extern const uint8_t FONT_8x8_ARRAY[];
	extern const SSD1306_Font_t font_8x8;
	#define SSD1306_FONT_8x8    (&font_8x8)
#endif



	// PT Mono
#ifdef SSD1306_INCLUDE_FONT_7x11
	extern const uint8_t FONT_7x11_ARRAY[];
	extern const SSD1306_Font_t font_7x11;
	#define SSD1306_FONT_7x11   (&font_7x11)
#endif

	// PT Mono
#ifdef SSD1306_INCLUDE_FONT_7x14
	extern const uint8_t FONT_7x14_ARRAY[];
	extern const SSD1306_Font_t font_7x14;
	#define SSD1306_FONT_7x14   (&font_7x14)
#endif

#ifdef SSD1306_INCLUDE_FONT_11x21
	extern const uint8_t FONT_11x21_ARRAY[];
	extern const SSD1306_Font_t font_11x21;
	#define SSD1306_FONT_11x21  (&font_11x21)
#endif

#ifdef SSD1306_INCLUDE_FONT_16x30
	extern const uint8_t FONT_16x30_ARRAY[];
	extern const SSD1306_Font_t font_16x30;
#endif







#endif /* THIRD_PARTY_SSD1306_INC_SSD1306_FONTS_H_ */
