/*
 * MIT License
 * Copyright (c) 2025 Даниил Еремеев
 * See LICENSE file for details.
 */

/* NOTE:
 * This file is not user-configurable.
 * These are raw SSD1306 command byte constants.
 */

#ifndef THIRD_PARTY_SSD1306_INC_SSD1306_CMD_H_
#define THIRD_PARTY_SSD1306_INC_SSD1306_CMD_H_

/*
 * SSD1306 command definitions.
 * These values are sent as command bytes over I2C/SPI.
 */

/* --- Display on/off --- */
#define SSD1306_CMD_DISPLAY_OFF                0xAE
#define SSD1306_CMD_DISPLAY_ON                 0xAF

/* --- Memory addressing mode --- */
#define SSD1306_CMD_SET_MEMORY_MODE            0x20
#define SSD1306_ADDR_MODE_HORIZONTAL           0x00
#define SSD1306_ADDR_MODE_VERTICAL             0x01
#define SSD1306_ADDR_MODE_PAGE                 0x02

/* --- Contrast control --- */
#define SSD1306_CMD_SET_CONTRAST               0x81

/* --- Display mode (normal / inverted) --- */
#define SSD1306_CMD_SET_NORMAL_DISPLAY         0xA6
#define SSD1306_CMD_SET_INVERT_DISPLAY         0xA7

/* --- Page and column addressing --- */
#define SSD1306_CMD_SET_PAGE_START             0xB0
#define SSD1306_CMD_SET_LOW_COLUMN             0x00
#define SSD1306_CMD_SET_HIGH_COLUMN            0x10

/* --- COM (row) scan direction
 * 0xC0: scan from COM0 to COM[N-1]
 * 0xC8: scan from COM[N-1] to COM0
 */
#define SSD1306_CMD_SET_COM_OUTPUT_NORMAL      0xC0
#define SSD1306_CMD_SET_COM_OUTPUT_REMAPPED    0xC8

/* --- SEG (column) mapping
 * 0xA0: column 0 is SEG0
 * 0xA1: column 0 is SEG127
 */
#define SSD1306_CMD_SET_SEGMENT_REMAP_NORMAL   0xA0
#define SSD1306_CMD_SET_SEGMENT_REMAP_MIRROR   0xA1

/* --- Multiplexing and timing --- */
#define SSD1306_CMD_SET_MULTIPLEX_RATIO        0xA8
#define SSD1306_CMD_SET_DISPLAY_OFFSET         0xD3
#define SSD1306_CMD_SET_DISPLAY_CLOCK_DIV      0xD5
#define SSD1306_CMD_SET_PRECHARGE              0xD9
#define SSD1306_CMD_SET_COM_PINS               0xDA
#define SSD1306_CMD_SET_VCOM_DESELECT          0xDB

/* --- Charge pump control --- */
#define SSD1306_CMD_SET_CHARGE_PUMP            0x8D
#define SSD1306_CHARGE_PUMP_ENABLE             0x14
#define SSD1306_CHARGE_PUMP_DISABLE            0x10

/* --- Entire display on/off (RAM ignore) --- */
#define SSD1306_CMD_DISPLAY_ALL_ON_RESUME      0xA4
#define SSD1306_CMD_DISPLAY_ALL_ON             0xA5

#endif /* THIRD_PARTY_SSD1306_INC_SSD1306_CMD_H_ */
