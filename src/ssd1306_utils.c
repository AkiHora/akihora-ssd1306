/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

/*
 * ssd1306_utils.c
 * Internal utility helpers for SSD1306 driver (geometry and timing).
 */

#include <stdint.h>
#include "ssd1306_utils.h"
#include "ssd1306_port.h"
#include "ssd1306_priv.h"

/* =======================================================================
 * Geometry / clipping helpers
 * ======================================================================= */

uint8_t ssd1306_geom_compute_out_code(int16_t x, int16_t y) {
	uint8_t code;

	code = SSD1306_CS_INSIDE;

	if (x < 0) {
		code |= SSD1306_CS_LEFT;
	} else if (x >= SSD1306_WIDTH) {
		code |= SSD1306_CS_RIGHT;
	}

	if (y < 0) {
		code |= SSD1306_CS_TOP;
	} else if (y >= SSD1306_HEIGHT) {
		code |= SSD1306_CS_BOTTOM;
	}

	return code;
}

void ssd1306_util_swap_int(int *a, int *b) {
	int t;

	t = *a;
	*a = *b;
	*b = t;
}

/* =======================================================================
 * Timing helpers (DWT / SysTick)
 * ======================================================================= */

/* Internal timing state */
static uint8_t  s_use_dwt = 0;
static uint32_t s_core_hz = 0;

void ssd1306_time_init(uint32_t hclk_hz) {
	uint32_t c0;
	uint32_t c1;
	volatile int i;

	s_core_hz = hclk_hz;

	/* Try to enable DWT->CYCCNT (Cortex-M3) */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0U;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	/* Check that the cycle counter is actually running */
	c0 = DWT->CYCCNT;
	for (i = 0; i < 64; ++i) {
		__NOP();
	}
	c1 = DWT->CYCCNT;
	s_use_dwt = ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) && (c1 != c0));
}

/* Blocking delay (HAL_Delay-like) */
void ssd1306_time_delay_ms(uint32_t ms) {
	uint64_t cycles_per_ms;
	uint32_t start;
	uint32_t wait;

	uint32_t load_plus1;
	uint64_t num;
	uint64_t den;
	uint64_t flags_needed;

	if (ms == 0U) {
		return;
	}

	if (s_use_dwt) {
		/* Accurate delay based on core cycles */
		cycles_per_ms = (uint64_t)s_core_hz / 1000ULL;
		while (ms--) {
			start = DWT->CYCCNT;
			wait = (uint32_t)cycles_per_ms;
			while ((uint32_t)(DWT->CYCCNT - start) < wait) {
				__NOP();
			}
		}
		return;
	}

	/* Fallback: use SysTick COUNTFLAG with current tick period */
	load_plus1 = SysTick->LOAD + 1U;
	num = (uint64_t)ms * (uint64_t)s_core_hz;
	den = (uint64_t)1000U * (uint64_t)load_plus1;
	flags_needed = (num + den - 1ULL) / den;

	while (flags_needed--) {
		while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) {
			__NOP();
		}
	}
}

/* Monotonic millisecond counter */
uint32_t ssd1306_time_ticks_ms(void) {
	/* If DWT is running, derive time from core cycles */
	if (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) {
		return (uint32_t)(DWT->CYCCNT / (s_core_hz / 1000U));
	} else {
		/* Fallback: accumulate milliseconds using COUNTFLAG */
		static uint32_t ms = 0U;
		uint32_t ctrl;

		/* COUNTFLAG is cleared by reading CTRL; consume all pending flags */
		do {
			ctrl = SysTick->CTRL;
			if ((ctrl & SysTick_CTRL_COUNTFLAG_Msk) != 0U) {
				ms++;
			}
		} while ((ctrl & SysTick_CTRL_COUNTFLAG_Msk) != 0U);

		return ms;
	}
}
