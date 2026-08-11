#pragma once

// Select the hardware target.
// Leave all lines commented to use the ESP32-DIV V2 wiring.

// #define BOARD_CYD
// #define BOARD_ESP32_DIV_V1
// #define BOARD_ESP32_DIV_V2

// >>> NEW: ESP32 WROOM 30-pin + 0.96" OLED SSD1306 + GPIO buttons (no SD, no touch).
//     Flash storage via LittleFS replaces the SD card.
// May also be defined via -DBOARD_ESP32_WROOM_OLED=1 in platformio.ini build_flags.
#ifndef BOARD_ESP32_WROOM_OLED
#define BOARD_ESP32_WROOM_OLED
#endif

// Set to 0 to hide the on-screen touch nav bar (5 footer buttons).
// Touch button input will still work when this is disabled.
#define TOUCH_BUTTON_CUE_ENABLED 0

// Optional fixed PCF8574 I2C address (0x20-0x27). Leave commented for auto-detect.
// (Ignored on BOARD_ESP32_WROOM_OLED — buttons are wired to GPIO pins directly.)
//#define pcf_ADDR 0x20

// Optional per-board touch calibration overrides (raw XPT2046 ADC range).
// (Not used on the OLED board — no touch panel.)
//#define TOUCH_X_MIN 200
//#define TOUCH_X_MAX 3700
//#define TOUCH_Y_MIN 240
//#define TOUCH_Y_MAX 3800

#if !defined(BOARD_ESP32_DIV_V2) && !defined(BOARD_CYD) && !defined(BOARD_ESP32_DIV_V1) && !defined(BOARD_ESP32_WROOM_OLED)
#define BOARD_ESP32_DIV_V2
#endif
