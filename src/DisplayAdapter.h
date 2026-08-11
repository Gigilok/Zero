/*
 * DisplayAdapter.h
 *
 * TFT_eSPI compatibility shim that renders to a 0.96" SSD1306 OLED
 * (128x64, I2C, SDA=21, SCL=22) instead of the original 240x320 color TFT.
 *
 * Designed for the ESP32 WROOM 30-pin adaptation of ESP32-DIV.
 *
 * Behavior summary:
 *   * WHITE BACKGROUND — the OLED buffer is initialized to all-white; any
 *     pixel drawn with BLACK becomes a black dot (the "ink"). All other
 *     colors also become black ink (since the panel is 1-bit).
 *   * The original 240x320 (portrait) coordinate space is auto-scaled to the
 *     128x64 OLED. The same scale is applied to primitives, rectangles and
 *     bitmaps so the original layout is preserved proportionally.
 *   * Text is rendered with Adafruit_GFX's default font, with integer size
 *     derived from the original setTextSize() call (minimum 1).
 *   * `display()` is called automatically after every primitive so the
 *     shim behaves like TFT_eSPI's immediate-mode API.
 *
 *  I2C wiring:
 *    SDA -> GPIO21
 *    SCL -> GPIO22
 *    ADDR-> GND (0x3C)
 */
#pragma once

#include <Arduino.h>
#include <Print.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---- Original screen coordinate space (kept identical to TFT_eSPI build) ----
#ifndef TFT_WIDTH
#define TFT_WIDTH  240
#endif
#ifndef TFT_HEIGHT
#define TFT_HEIGHT 320
#endif

// ---- Color constants (kept identical to the original project's palette) ----
#define TFT_BLACK       0x0000
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x03EF
#define TFT_MAROON      0x7800
#define TFT_PURPLE      0x780F
#define TFT_OLIVE       0x7BE0
#define TFT_LIGHTGREY   0xC618
#define TFT_DARKGREY    0x7BEF
#define TFT_BLUE        0x001F
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_ORANGE      0xFD20
#define TFT_GREENYELLOW 0xAFE5
#define TFT_PINK        0xF81F
#define TFT_BROWN       0xBC40
#define TFT_GOLD        0xFEA0
#define TFT_SILVER      0xC618
#define TFT_SKYBLUE     0x867D
#define TFT_SALMON      0xFC00
#define TFT_WHITE       0xFFFF

// Backwards-compat aliases used in some source files
#define TFT_DARKBLUE    0x3166
#define TFT_LIGHTBLUE   0x051F
#define TFTWHITE        0xFFFF
#define TFT_GRAY        0x8410
#define TFT_GREEN_YELLOW 0xAFE5

#ifdef TFT_GREEN
#undef TFT_GREEN
#endif
#define TFT_GREEN 0x07E0

// Datum constants used by setTextDatum (subset of TFT_eSPI's enum)
#define TL_DATUM  0
#define TC_DATUM  1
#define TR_DATUM  2
#define ML_DATUM  3
#define MC_DATUM  4
#define MR_DATUM  5
#define BL_DATUM  6
#define BC_DATUM  7
#define BR_DATUM  8
#define L_BASELINE 9
#define C_BASELINE 10
#define R_BASELINE 11

// OLED hardware dimensions
#define OLED_SCREEN_WIDTH  128
#define OLED_SCREEN_HEIGHT 64
#define OLED_I2C_ADDR      0x3C
#define OLED_RESET_PIN     -1  // -1 = share reset with ESP32

class TFT_eSPI : public Print {
public:
    TFT_eSPI();

    // ---- Lifecycle ----
    void init(uint16_t tc = 0xFFFF);
    void begin(uint16_t tc = 0xFFFF) { init(tc); }
    void setRotation(uint8_t r);
    uint8_t getRotation() const { return _rotation; }
    void invertDisplay(bool invert);
    void display();

    // ---- Geometry reported back to the original code ----
    // Always report the original 240x320 so the project's layout code is unchanged.
    int16_t width() const  { return TFT_WIDTH;  }
    int16_t height() const { return TFT_HEIGHT; }

    // ---- Primitives (input is in original 240x320 space; auto-scaled) ----
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void drawCircleHelper(int16_t x, int16_t y, int16_t r, uint8_t corner, uint16_t color);
    void fillCircleHelper(int16_t x, int16_t y, int16_t r, uint8_t corner, int16_t delta, uint16_t color);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillScreen(uint16_t color);

    // ---- Bitmaps ----
    void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                    int16_t w, int16_t h, uint16_t color);
    void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                    int16_t w, int16_t h, uint16_t color, uint16_t bg);
    void drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                     int16_t w, int16_t h, uint16_t color);
    void pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data);
    void pushRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data);

    // ---- Text (Print API forwards through write()) ----
    using Print::write;
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;

    void setCursor(int16_t x, int16_t y);
    void setCursor(int16_t x, int16_t y, uint8_t font);
    int16_t getCursorX() const { return _cursorX; }
    int16_t getCursorY() const { return _cursorY; }
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t bg);
    void setTextSize(uint8_t s);
    void setTextWrap(bool w);
    void setTextDatum(uint8_t d);
    void setTextFont(uint8_t f);
    uint8_t getTextFont() const { return _font; }
    int16_t textWidth(const char *str);
    int16_t textWidth(const String &str) { return textWidth(str.c_str()); }
    int16_t fontHeight(int8_t font = -1);

    void drawChar(int16_t x, int16_t y, unsigned char c,
                  uint16_t color, uint16_t bg, uint8_t size);
    void drawString(const char *str, int16_t x, int16_t y);
    void drawString(const String &str, int16_t x, int16_t y) { drawString(str.c_str(), x, y); }
    void drawCentreString(const char *str, int16_t x, int16_t y, uint8_t = 1);
    void drawCentreString(const String &s, int16_t x, int16_t y, uint8_t f = 1);
    void drawRightString(const char *str, int16_t x, int16_t y, uint8_t = 1);
    void drawRightString(const String &s, int16_t x, int16_t y, uint8_t f = 1);
    void drawNumber(long n, int16_t x, int16_t y);
    void drawNumber(long n, int16_t x, int16_t y, uint8_t) { drawNumber(n, x, y); }
    void drawFloat(float fl, uint8_t dp, int16_t x, int16_t y);

    // ---- Raw SPI passthrough stubs (no-op on I2C OLED) ----
    void startWrite() {}
    void endWrite()   {}
    void writecommand(uint8_t)  {}
    void writedata(uint8_t)     {}
    void spiwrite(uint8_t)      {}

    // ---- Touch stubs (the OLED board has no touch) ----
    uint8_t  getTouchRaw(uint16_t *x = nullptr, uint16_t *y = nullptr) { if (x) *x = 0; if (y) *y = 0; return false; }
    uint16_t getTouchRawZ() { return 0; }
    bool     getTouch(uint16_t *x = nullptr, uint16_t *y = nullptr, uint16_t = 600) {
        if (x) *x = 0; if (y) *y = 0; return false;
    }

    // Set SPI bus used internally (no-op on I2C OLED)
    void setSpiBus(SPIClass &) {}

    // Returns the underlying SSD1306 driver (used by utils.cpp status bar / etc.).
    Adafruit_SSD1306 *oled() { return _oled; }

private:
    Adafruit_SSD1306 *_oled;
    uint8_t  _rotation;
    int16_t  _cursorX, _cursorY;     // in original 240x320 coordinate space
    uint16_t _fgColor, _bgColor;     // 16-bit TFT color
    uint8_t  _textSize;              // original-space size
    bool     _textWrap;
    uint8_t  _textDatum;
    uint8_t  _font;
    bool     _autoDisplay;

    // Convert a 16-bit TFT color to a 1-bit OLED ink value.
    //
    // WHITE BACKGROUND theme: the panel shows a white field by default, and we
    // draw BLACK INK on top. To preserve contrast for BOTH themes:
    //   - LIGHT source colors (TFT_WHITE, BG_Light, etc.) → pixel OFF in buffer
    //     → panel shows BLACK (used as ink on a white field, or as a "selected"
    //     highlight rectangle)
    //   - DARK  source colors (TFT_BLACK, BG_Dark, etc.)  → pixel ON  in buffer
    //     → panel shows WHITE (used as background fill, so dark UI backgrounds
    //     render as a white field on the OLED)
    //
    // The net effect: any UI drawn with the project's default Dark theme
    // (dark bg + light text) appears on the OLED as WHITE BACKGROUND with BLACK
    // INK — exactly what the user asked for.
    static bool isLightColor(uint16_t c);
    static inline uint16_t colorToInk(uint16_t c) {
        return isLightColor(c) ? SSD1306_BLACK : SSD1306_WHITE;
    }
    static inline uint16_t colorToBg(uint16_t c) {
        return isLightColor(c) ? SSD1306_BLACK : SSD1306_WHITE;
    }

    // Scale an original-coordinate to OLED pixel space.
    inline int16_t sx(int16_t x) const { return (int16_t)((int32_t)x * OLED_SCREEN_WIDTH  / TFT_WIDTH); }
    inline int16_t sy(int16_t y) const { return (int16_t)((int32_t)y * OLED_SCREEN_HEIGHT / TFT_HEIGHT); }
    inline int16_t sw(int16_t w) const { return (int16_t)((int32_t)w * OLED_SCREEN_WIDTH  / TFT_WIDTH); }
    inline int16_t sh(int16_t h) const { return (int16_t)((int32_t)h * OLED_SCREEN_HEIGHT / TFT_HEIGHT); }

    // Map original textSize (used in 240x320 space) to OLED-space integer size.
    // 0 or 1 -> 1; >=2 -> 1 (we keep size 1 because 240->128 scale is ~0.53).
    inline uint8_t scaledTextSize() const { return (_textSize < 2) ? 1 : 1; }

    void flush();
};
