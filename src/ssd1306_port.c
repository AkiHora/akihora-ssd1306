/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

#include "ssd1306_port.h"

/* Internal bus configuration (I2C instance, address, timeout) */
static ssd1306_bus_t ssd1306_bus;

/* Shortcut to the active I2C peripheral */
#define I2Cx (ssd1306_bus.i2c)

/* =======================================================================
 * Timing helpers (DWT-based and fallback)
 * ======================================================================= */

/* DWT timing state */
static uint8_t  _use_dwt = 0;
static uint32_t _core_hz = 0;

/* Initialize timing helpers (DWT cycle counter or fallback) */
static void ssd1306_port_timing_init(void) {
	uint32_t a, b;

	_core_hz = SystemCoreClock;

	/* Try enabling DWT->CYCCNT (Cortex-M3) */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0U;
	DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

	a = DWT->CYCCNT;
	__NOP();
	b = DWT->CYCCNT;

	_use_dwt = (b != a);
}

void ssd1306_port_init(I2C_TypeDef *i2c, uint16_t addr8, uint32_t timeout_ms) {
	ssd1306_port_timing_init();
	ssd1306_bus.i2c     = i2c;
	ssd1306_bus.addr8   = addr8;
	ssd1306_bus.timeout = timeout_ms;
}

/* =======================================================================
 * Generic wait helper with timeout
 * ======================================================================= */
/*
 * Returns 0 on success, -1 on timeout.
 */
static int wait_ok(int (*ok)(void), uint32_t ms) {
	uint64_t budget;
	uint32_t start;
	uint32_t loops;

	if (ms == 0U) {
		return ok() ? 0 : -1;
	}

	if (_use_dwt) {
		/* Deadline in core cycles */
		budget = ((uint64_t)_core_hz * (uint64_t)ms) / 1000ULL;
		start  = DWT->CYCCNT;

		for (;;) {
			if (ok()) {
				return 0;
			}
			if ((uint32_t)(DWT->CYCCNT - start) >= (uint32_t)budget) {
				return -1;
			}
			__NOP();
		}
	}

	/* Coarse fallback without DWT (iteration-based) */
	/* Factor 8 ≈ cycles per loop (conservative) */
	loops = ((_core_hz / 1000U) * ms) / 8U + 1U;

	while (!ok()) {
		if (loops-- == 0U) {
			return -1;
		}
		__NOP();
	}

	return 0;
}

/* =======================================================================
 * I2C ready-flag helpers (CMSIS bitfields)
 * ======================================================================= */
static int ok_bus_free(void) { return ((I2Cx->SR2 & I2C_SR2_BUSY) == 0U); }
static int ok_SB(void)       { return ((I2Cx->SR1 & I2C_SR1_SB)   != 0U); }
static int ok_ADDR(void)     { return ((I2Cx->SR1 & I2C_SR1_ADDR) != 0U); }
static int ok_TXE(void)      { return ((I2Cx->SR1 & I2C_SR1_TXE)  != 0U); }
static int ok_BTF(void)      { return ((I2Cx->SR1 & I2C_SR1_BTF)  != 0U); }

/* =======================================================================
 * I2C write transaction
 * ======================================================================= */
/*
 * START → address → data → STOP.
 * STOP is always generated if START was issued.
 */
ssd1306_status_t ssd1306_port_i2c_write(const uint8_t *data, uint16_t size) {
	ssd1306_status_t rc = SSD1306_OK;
	uint16_t i;
	uint32_t tmp;
	uint8_t started = 0;

	if (I2Cx == 0) {
		return SSD1306_ERR;
	}
	if ((I2Cx->CR1 & I2C_CR1_PE) == 0U) {
		return SSD1306_ERR;
	}

	do {
		/* Bus free? */
		if (wait_ok(ok_bus_free, ssd1306_bus.timeout)) {
			rc = SSD1306_BUSY;
			break;
		}

		/* START */
		I2Cx->CR1 |= I2C_CR1_START;
		started = 1;

		if (wait_ok(ok_SB, ssd1306_bus.timeout)) {
			rc = SSD1306_TIMEOUT;
			break;
		}

		/* Address (already shifted <<1) */
		I2Cx->DR = (uint8_t)ssd1306_bus.addr8;

		if (wait_ok(ok_ADDR, ssd1306_bus.timeout)) {
			rc = SSD1306_TIMEOUT;
			break;
		}

		/* Clear ADDR by reading SR1 then SR2 */
		tmp = I2Cx->SR1; (void)tmp;
		tmp = I2Cx->SR2; (void)tmp;

		/* Data bytes */
		for (i = 0; i < size; i++) {
			if (wait_ok(ok_TXE, ssd1306_bus.timeout)) {
				rc = SSD1306_TIMEOUT;
				break;
			}

			I2Cx->DR = data[i];

			/* For last byte also wait for BTF */
			if (i == (uint16_t)(size - 1U)) {
				if (wait_ok(ok_BTF, ssd1306_bus.timeout)) {
					rc = SSD1306_TIMEOUT;
					break;
				}
			}
		}

	} while (0);

	/* If NACK received → clear AF */
	if ((I2Cx->SR1 & I2C_SR1_AF) != 0U) {
		I2Cx->SR1 &= (uint16_t)~I2C_SR1_AF;
	}

	/* STOP only if we issued START */
	if (started) {
		I2Cx->CR1 |= I2C_CR1_STOP;
	}

	return rc;
}

/* Watchdog hook (platform-dependent, intentionally empty here) */
void ssd1306_port_watchdog_feed(void) {
	;
}
