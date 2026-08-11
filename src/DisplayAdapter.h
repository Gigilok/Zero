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

// Adafruit_SSD1306.h #defines common color names (BLACK, WHITE, RED, GREEN,
// BLUE, YELLOW, CYAN, MAGENTA) as macros that expand to SSD1306_* constants.
// The project's shared.h then declares `const uint16_t BLACK = 0x0000;` etc.,
// which the preprocessor turns into `const uint16_t 0 = 0x0000;` — a syntax
// error. #undef them here so the names are free for shared.h to claim.
#ifdef BLACK
#undef BLACK
#endif
#ifdef WHITE
#undef WHITE
#endif
#ifdef RED
#undef RED
#endif
#ifdef GREEN
#undef GREEN
#endif
#ifdef BLUE
#undef BLUE
#endif
#ifdef YELLOW
#undef YELLOW
#endif
#ifdef CYAN
#undef CYAN
#endif
#ifdef MAGENTA
#undef MAGENTA
#endif

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
#ifdef TFT_GREEN
#undef TFT_GREEN
#endif
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_ORANGE      0xFD20
#ifdef TFT_GREENYELLOW
#undef TFT_GREENYELLOW
#endif
#define TFT_GREENYELLOW 0xAFE5
#define TFT_PINK        0xF81F
#define TFT_BROWN       0xBC40
#define TFT_GOLD        0xFEA0
#define TFT_SILVER      0xC618
#define TFT_SKYBLUE     0x867D
#define TFT_SALMON      0xFC00
#define TFT_WHITE       0xFFFF

// Backwards-compat aliases used in some source files. Use #undef guards
// because shared.h / utils.h may already have defined these (with different
// values for the OLED theme — e.g. TFT_GREEN is mapped to GREEN = 0xB721).
#ifdef TFT_DARKBLUE
#undef TFT_DARKBLUE
#endif
#define TFT_DARKBLUE    0x3166
#ifdef TFT_LIGHTBLUE
#undef TFT_LIGHTBLUE
#endif
#define TFT_LIGHTBLUE   0x051F
#ifdef TFTWHITE
#undef TFTWHITE
#endif
#define TFTWHITE        0xFFFF
#ifdef TFT_GRAY
#undef TFT_GRAY
#endif
#define TFT_GRAY        0x8410
#define TFT_GREEN_YELLOW 0xAFE5

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

// ILI9341 scroll command opcodes (used by utils.cpp's terminal scroll code).
// On the SSD1306 OLED these are no-ops — tft.writecommand() / tft.writedata()
// are stubbed to empty functions below — but the constants must exist so the
// project's source compiles unmodified.
#ifndef ILI9341_VSCRDEF
#define ILI9341_VSCRDEF   0x33
#endif
#ifndef ILI9341_VSCRSADD
#define ILI9341_VSCRSADD  0x37
#endif

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
    // 7-arg variant with background color (Adafruit_GFX convention). The bg
    // fills "0" bits in the XBM data; the color fills "1" bits.
    void drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                     int16_t w, int16_t h, uint16_t color, uint16_t bg);
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
    // 2-arg overload with a `font` parameter (TFT_eSPI convention). Font is
    // ignored on the OLED; we honor the call so the project compiles unchanged.
    int16_t textWidth(const char *str, uint8_t /*font*/) { return textWidth(str); }
    int16_t textWidth(const String &str, uint8_t font) { return textWidth(str.c_str(), font); }
    int16_t fontHeight(int8_t font = -1);

    void drawChar(int16_t x, int16_t y, unsigned char c,
                  uint16_t color, uint16_t bg, uint8_t size);
    // 4-arg overload (TFT_eSPI convention) — uses current text color/size and
    // returns the character's width so the caller can advance the cursor.
    // Used by utils.cpp's terminal: `xPos += tft.drawChar(data, xPos, yDraw, 2);`
    int16_t drawChar(unsigned char c, int16_t x, int16_t y, uint8_t size);
    void drawString(const char *str, int16_t x, int16_t y);
    void drawString(const String &str, int16_t x, int16_t y) { drawString(str.c_str(), x, y); }
    // 4-arg overloads with a trailing `font` parameter (TFT_eSPI convention).
    // The font number is ignored on the OLED (single built-in font), but the
    // signature must match because the project calls e.g. drawString(s, x, y, 2).
    void drawString(const char *str, int16_t x, int16_t y, uint8_t /*font*/) { drawString(str, x, y); }
    void drawString(const String &str, int16_t x, int16_t y, uint8_t font) { drawString(str.c_str(), x, y, font); }
    void drawCentreString(const char *str, int16_t x, int16_t y, uint8_t = 1);
    void drawCentreString(const String &s, int16_t x, int16_t y, uint8_t f = 1);
    void drawRightString(const char *str, int16_t x, int16_t y, uint8_t = 1);
    void drawRightString(const String &s, int16_t x, int16_t y, uint8_t f = 1);
    void drawNumber(long n, int16_t x, int16_t y);
    void drawNumber(long n, int16_t x, int16_t y, uint8_t) { drawNumber(n, x, y); }
    void drawFloat(float fl, uint8_t dp, int16_t x, int16_t y);
    void drawFloat(float fl, uint8_t dp, int16_t x, int16_t y, uint8_t) { drawFloat(fl, dp, x, y); }

    // ---- Raw SPI passthrough stubs (no-op on I2C OLED) ----
    void startWrite() {}
    void endWrite()   {}
    void writecommand(uint8_t)  {}
    void writedata(uint8_t)     {}
    void spiwrite(uint8_t)      {}

    // ---- Software vertical scroll (OLED replacement for ILI9341 hardware scroll) ----
    // Shifts the SSD1306 framebuffer UP by `pixelRows` rows (0..64). The bottom
    // `pixelRows` rows are erased (filled with the background ink value). Call
    // display() afterwards (or rely on the next primitive's auto-flush).
    //
    // This is the standard software-scroll technique for SSD1306: getBuffer()
    // exposes the 1024-byte framebuffer (128 cols × 8 pages of 8 vertical px),
    // and we memmove pages + bit-shift partial-page remainders.
    //
    // Used by Terminal::scroll_line() (utils.cpp) and any feature that needs
    // terminal-style line scrolling. pixelRows should typically equal the
    // font's line height (e.g. 8 for size=1, 16 for size=2).
    void scrollUp(uint8_t pixelRows);

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

    // ---- Color mapping (public so TFT_eSprite can reuse it) ----
    // Maps a 16-bit TFT color to a 1-bit OLED ink value.
    //
    // BLACK BACKGROUND theme (default after user testing — the OLED's natural
    // off-state is black, which gives better contrast than inverted white):
    //   - LIGHT source colors (TFT_WHITE, BG_Light, light text)  -> pixel ON
    //     -> panel shows WHITE (used as ink / text on a black field)
    //   - DARK  source colors (TFT_BLACK, BG_Dark, dark bg fills) -> pixel OFF
    //     -> panel shows BLACK (used as background fill)
    //
    // The net effect: the project's default Dark theme (dark bg + light text)
    // appears on the OLED as BLACK BACKGROUND with WHITE INK — the standard
    // OLED look.
    static bool isLightColor(uint16_t c);
    static inline uint16_t colorToInk(uint16_t c) {
        return isLightColor(c) ? SSD1306_WHITE : SSD1306_BLACK;
    }
    static inline uint16_t colorToBg(uint16_t c) {
        return isLightColor(c) ? SSD1306_WHITE : SSD1306_BLACK;
    }

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

    // Scale an original-coordinate to OLED pixel space.
    inline int16_t sx(int16_t x) const { return (int16_t)((int32_t)x * OLED_SCREEN_WIDTH  / TFT_WIDTH); }
    inline int16_t sy(int16_t y) const { return (int16_t)((int32_t)y * OLED_SCREEN_HEIGHT / TFT_HEIGHT); }
    inline int16_t sw(int16_t w) const { return (int16_t)((int32_t)w * OLED_SCREEN_WIDTH  / TFT_WIDTH); }
    inline int16_t sh(int16_t h) const { return (int16_t)((int32_t)h * OLED_SCREEN_HEIGHT / TFT_HEIGHT); }

    // Map original textSize (used in 240x320 space) to OLED-space integer size.
    //
    // On the 128x64 OLED, Adafruit_GFX size=1 (6x8 px per char) gives 21 chars
    // per line × 8 lines — the same visual density the original 240x320 layout
    // was designed for (size=1 = 6 px advance × 40 chars on 240 px wide TFT).
    // Size=2 (12x16 px) only fits 10 chars × 4 lines and made menu text
    // "enormous" / overlapping per user feedback.
    //
    // We now ALWAYS render at size=1 regardless of source _textSize. The
    // original layout's larger textSize calls (e.g. drawString(...,2)) were
    // meant for a 240x320 TFT where size=2 = 12 px tall — proportionally the
    // SAME as size=1 on the 128x64 OLED. Forcing size=1 here keeps the visual
    // proportions correct while staying readable.
    inline uint8_t scaledTextSize() const { return 1; }

    void flush();
};

// =========================================================================
// TFT_eSprite — off-screen drawing surface (drop-in for TFT_eSPI's sprite).
// =========================================================================
// The project uses TFT_eSprite in gps.cpp to compose the GNSS scan panel
// before blitting it to the screen (avoids visible progressive drawing).
//
// On the 1-bit OLED we implement the sprite as a GFXcanvas1 (Adafruit_GFX's
// 1-bit-per-pixel in-memory canvas). When pushSprite(x, y) is called, the
// canvas buffer is copied to the parent TFT_eSPI via drawXBitmap, which uses
// the same packed format as SSD1306's internal buffer (8 vertical pixels per
// byte, LSB at top).
//
// All color arguments passed to sprite primitives are mapped to 1-bit ink
// via the same isLightColor() rule as the main display.
//
// setColorDepth() is accepted but ignored — the canvas is always 1-bit.
// Create the sprite with TFT_eSprite(&tft) where tft is the global display.
// =========================================================================
class TFT_eSprite : public Print {
public:
    explicit TFT_eSprite(TFT_eSPI *parent)
        : Print(), _parent(parent), _created(false), _canvas(nullptr),
          _cw(0), _ch(0),
          _fgColor(0xFFFF), _bgColor(0x0000), _textSize(1), _textDatum(0), _font(1),
          _cursorX(0), _cursorY(0) {}

    // ---- Sprite lifecycle ----
    // createSprite allocates (or re-allocates) the canvas to the given size.
    // Returns true on success. The original TFT_eSPI returns void; we return
    // bool because gps.cpp checks the result. (Void-returning call sites still
    // work - they just ignore the return value.)
    bool createSprite(int16_t w, int16_t h) {
        if (w <= 0 || h <= 0) return false;
        if (_canvas) { delete _canvas; _canvas = nullptr; }
        _canvas = new GFXcanvas1(w, h);
        if (!_canvas) { _created = false; _cw = _ch = 0; return false; }
        _created = true;
        _cw = w; _ch = h;
        _canvas->fillScreen(0);
        return true;
    }
    void deleteSprite() {
        if (_canvas) { delete _canvas; _canvas = nullptr; }
        _created = false;
        _cw = _ch = 0;
    }
    void setColorDepth(uint8_t /*bpp*/) { /* always 1-bit on OLED */ }
    void pushSprite(int16_t x, int16_t y) {
        if (!_parent || !_canvas) return;
        // Canvas stores: 1 = ink (dark color drawn → black on white OLED),
        //                0 = background (light color → white on white OLED).
        // Parent's drawXBitmap(7-arg) maps:
        //   1-bits → color arg → colorToInk(color).  For BLACK (0x0000, dark),
        //            colorToInk returns SSD1306_WHITE (pixel ON = black on screen). ✓
        //   0-bits → bg arg    → colorToBg(bg).       For WHITE (0xFFFF, light),
        //            colorToBg returns SSD1306_BLACK (pixel OFF = white on screen). ✓
        _parent->drawXBitmap(x, y, _canvas->getBuffer(), _cw, _ch, 0x0000, 0xFFFF);
    }
    void pushSprite(int16_t x, int16_t y, uint16_t /*transparent*/) {
        pushSprite(x, y);
    }

    // ---- Geometry ----
    int16_t width() const  { return _cw; }
    int16_t height() const { return _ch; }

    // ---- Primitives (route to the canvas, mapping color to 1-bit) ----
    void drawPixel(int16_t x, int16_t y, uint16_t color) {
        if (!_canvas) return;
        _canvas->drawPixel(x, y, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
        if (!_canvas) return;
        _canvas->drawFastVLine(x, y, h, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
        if (!_canvas) return;
        _canvas->drawFastHLine(x, y, w, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
        if (!_canvas) return;
        _canvas->drawLine(x0, y0, x1, y1, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        if (!_canvas) return;
        _canvas->drawRect(x, y, w, h, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        if (!_canvas) return;
        _canvas->fillRect(x, y, w, h, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
        if (!_canvas) return;
        _canvas->drawRoundRect(x, y, w, h, r, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
        if (!_canvas) return;
        _canvas->fillRoundRect(x, y, w, h, r, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
        if (!_canvas) return;
        _canvas->drawCircle(x, y, r, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
        if (!_canvas) return;
        _canvas->fillCircle(x, y, r, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
        if (!_canvas) return;
        _canvas->drawTriangle(x0, y0, x1, y1, x2, y2, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
        if (!_canvas) return;
        _canvas->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_eSPI::colorToInk(color) ? 1 : 0);
    }
    void fillScreen(uint16_t color) {
        if (!_canvas) return;
        _canvas->fillScreen(TFT_eSPI::colorToInk(color) ? 1 : 0);
    }

    // ---- Text (cursor-based, using Adafruit_GFX's default font) ----
    void setCursor(int16_t x, int16_t y) { _cursorX = x; _cursorY = y; if (_canvas) _canvas->setCursor(x, y); }
    int16_t getCursorX() const { return _cursorX; }
    int16_t getCursorY() const { return _cursorY; }
    void setTextColor(uint16_t c) { _fgColor = c; _bgColor = c; if (_canvas) _canvas->setTextColor(TFT_eSPI::colorToInk(c) ? 1 : 0); }
    void setTextColor(uint16_t c, uint16_t bg) { _fgColor = c; _bgColor = bg; if (_canvas) _canvas->setTextColor(TFT_eSPI::colorToInk(c) ? 1 : 0, TFT_eSPI::colorToInk(bg) ? 1 : 0); }
    void setTextSize(uint8_t s) { _textSize = s; if (_canvas) _canvas->setTextSize(s); }
    void setTextWrap(bool w) { if (_canvas) _canvas->setTextWrap(w); }
    void setTextDatum(uint8_t d) { _textDatum = d; }
    void setTextFont(uint8_t f) { _font = f; }
    uint8_t getTextFont() const { return _font; }
    int16_t textWidth(const char *str) { return _textSize * 6 * (int16_t)strlen(str); }
    int16_t textWidth(const String &str) { return textWidth(str.c_str()); }
    int16_t textWidth(const char *str, uint8_t) { return textWidth(str); }
    int16_t fontHeight(int8_t = -1) { return _textSize * 8; }

    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
        if (!_canvas) return;
        _canvas->drawChar(x, y, c, TFT_eSPI::colorToInk(color) ? 1 : 0, TFT_eSPI::colorToInk(bg) ? 1 : 0, size);
    }
    void drawString(const char *str, int16_t x, int16_t y) {
        if (!_canvas) return;
        _canvas->setCursor(x, y);
        _canvas->setTextColor(TFT_eSPI::colorToInk(_fgColor) ? 1 : 0,
                              TFT_eSPI::colorToInk(_bgColor) ? 1 : 0);
        _canvas->setTextSize(_textSize);
        while (*str) { _canvas->write((uint8_t)*str++); }
    }
    void drawString(const String &str, int16_t x, int16_t y) { drawString(str.c_str(), x, y); }
    void drawString(const char *str, int16_t x, int16_t y, uint8_t) { drawString(str, x, y); }
    void drawString(const String &str, int16_t x, int16_t y, uint8_t f) { drawString(str.c_str(), x, y, f); }
    void drawCentreString(const char *str, int16_t x, int16_t y, uint8_t = 1) {
        drawString(str, x - textWidth(str) / 2, y);
    }
    void drawRightString(const char *str, int16_t x, int16_t y, uint8_t = 1) {
        drawString(str, x - textWidth(str), y);
    }

    // Print API (so sprite.print(...) works like on TFT_eSPI)
    using Print::write;
    size_t write(uint8_t c) override {
        if (_canvas) _canvas->write(c);
        return 1;
    }
    size_t write(const uint8_t *buf, size_t size) override {
        if (_canvas) { for (size_t i = 0; i < size; i++) _canvas->write(buf[i]); }
        return size;
    }

    // StartWrite/endWrite are no-ops on the canvas.
    void startWrite() {}
    void endWrite()   {}

private:
    TFT_eSPI *_parent;
    bool      _created;
    GFXcanvas1 *_canvas;
    int16_t   _cw, _ch;
    uint16_t  _fgColor, _bgColor;
    uint8_t   _textSize;
    uint8_t   _textDatum;
    uint8_t   _font;
    int16_t   _cursorX, _cursorY;
};
