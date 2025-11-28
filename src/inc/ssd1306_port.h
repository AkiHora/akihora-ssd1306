/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

#ifndef SSD1306_PORT_H
#define SSD1306_PORT_H

#include <stdint.h>
#include "ssd1306_conf.h"
#include "ssd1306_utils.h"

/* MCU-specific includes */
#if defined(SSD1306_MCU_STM32F1)
	#include "stm32f1xx.h"
#elif defined(SSD1306_MCU_STM32L1)
	#include "stm32l1xx.h"
#else
	#error "Define SSD1306_MCU_STM32Fx in ssd1306_conf.h"
#endif

/*
 * Driver status codes
 */
typedef enum {
	SSD1306_OK = 0,
	SSD1306_ERR,
	SSD1306_TIMEOUT,
	SSD1306_BUSY
} ssd1306_status_t;

/*
 * Minimal bus descriptor (similar to HAL handle)
 */
typedef struct {
	I2C_TypeDef *i2c;     /* I2C1 or I2C2 */
	uint16_t     addr8;   /* address << 1 */
	uint32_t     timeout; /* timeout in ms */
} ssd1306_bus_t;

/* Default bus configuration macro */
#define SSD1306_PORT_SETUP_DEFAULT() \
	ssd1306_port_init(SSD1306_I2C_PORT, SSD1306_I2C_ADDR, SSD1306_I2C_TIMEOUT)

/* Initialize port handle */
void ssd1306_port_init(I2C_TypeDef *i2c, uint16_t addr8, uint32_t timeout_ms);

/* Transmit a block over I2C (STOP is always generated) */
ssd1306_status_t ssd1306_port_i2c_write(const uint8_t *data, uint16_t size);

/* Platform-specific watchdog hook */
void ssd1306_port_watchdog_feed(void);

#endif
