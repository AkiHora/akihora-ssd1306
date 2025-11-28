# SSD1306 OLED driver for STM32 (C99)

Small, self-contained SSD1306 OLED driver for STM32 microcontrollers (F1 / L1) with an internal framebuffer, UTF-8 text rendering and a simple UI helper layer (menus, headers, progress bars, scrollbars).

The library is intended to be embedded into firmware projects (CubeIDE / Keil / etc.) as a third-party component.

---

## Features

- **C99, no HAL dependency**
  - Depends only on CMSIS device headers (`stm32f1xx.h` / `stm32l1xx.h`).
  - Uses direct I2C register access and DWT / SysTick for timing.

- **Supported MCUs**
  - `STM32F1` (`SSD1306_MCU_STM32F1`)
  - `STM32L1` (`SSD1306_MCU_STM32L1`)

- **Displays**
  - SSD1306 over I2C
  - Configurable display type in `ssd1306_conf.h`:
    - `SSD1306_DISPLAY_128x64`
    - `SSD1306_DISPLAY_64x32`
    - `SSD1306_DISPLAY_64x32_FAKE`

- **Framebuffer + dirty regions**
  - Full framebuffer in RAM.
  - All `_buffer_` functions modify only the framebuffer.
  - `ssd1306_flush_dirty()` updates only modified areas.

- **Text rendering**
  - UTF-8 charset (`SSD1306_CHARSET_UTF8`) with internal conversion.
  - Multiple bitmap fonts (enabled via `ssd1306_conf.h`):
    - `SSD1306_INCLUDE_FONT_8x8`
    - `SSD1306_INCLUDE_FONT_7x11`
    - `SSD1306_INCLUDE_FONT_7x14`
    - `SSD1306_INCLUDE_FONT_11x21`
    - `SSD1306_INCLUDE_FONT_16x30`
  - `SSD1306_FONT_DEFAULT` selects the default font.
  - Drawing API:
    - `ssd1306_buffer_draw_char_font(...)`
    - `ssd1306_buffer_draw_char(...)`
    - `ssd1306_buffer_draw_string_font(...)`
    - `ssd1306_buffer_draw_string(...)` (uses `SSD1306_FONT_DEFAULT`)

- **Drawing primitives**
  - Pixel: `ssd1306_buffer_draw_pixel(...)`
  - Lines, rectangles, triangles (outline and filled)
  - Bitmaps (e.g. logo from `ssd1306_images.h`)
  - All primitives work on the framebuffer and respect clipping.

- **Display control**
  - `ssd1306_init()`
  - `ssd1306_set_display_on(uint8_t on)`
  - `ssd1306_set_contrast(uint8_t value)`
  - `ssd1306_set_invert(uint8_t invert)`
  - `ssd1306_display_clear()`
  - `ssd1306_display_fill(SSD1306_COLOR_t color)`
  - `ssd1306_flush_dirty()`

- **UI helper layer (`ssd1306_ui`)**
  - Vertical menus with scrolling and active item highlight.
  - Headers with alignment and underline styles.
  - Progress bars with optional percentage display.
  - Scrollbars.
  - Used by demo tests in `ssd1306_tests.c`.

- **Timing helpers**
  - High-resolution delays based on DWT cycle counter (if available).
  - Fallback using `SysTick->COUNTFLAG`.
  - Public helpers in `ssd1306_utils.c`:
    - `ssd1306_time_init(uint32_t hclk_hz)`
    - `ssd1306_time_delay_ms(uint32_t ms)`
    - `ssd1306_time_ticks_ms(void)`

---

## Directory layout

```text
ssd1306/
  include/
    ssd1306.h         # Public API
    ssd1306_conf.h    # User configuration (MCU, I2C, display, fonts, charset)

  src/
    ssd1306.c         # Public API implementation
    ssd1306_priv.c    # Internal driver state, framebuffer, dirty flags, geometry
    ssd1306_utils.c   # Geometry helpers, timing helpers (DWT / SysTick)
    ssd1306_port.c    # Low-level I2C access and timing primitives
    ssd1306_fonts.c   # Font bitmaps and font descriptors
    ssd1306_images.c  # Example images (e.g. 64x64 logo)
    ssd1306_ui.c      # High-level UI widgets (menus, headers, progress bars)
    ssd1306_tests.c   # Demo / test routines

    inc/              # Internal headers
      ssd1306_cmd.h
      ssd1306_fonts.h
      ssd1306_images.h
      ssd1306_port.h
      ssd1306_priv.h
      ssd1306_tests.h
      ssd1306_ui.h
      ssd1306_utils.h
