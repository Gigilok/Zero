/*
 * DisplayAdapter.cpp
 *
 * Implementation of the TFT_eSPI shim that renders to SSD1306 OLED.
 * See DisplayAdapter.h for behavior notes.
 */
#include "DisplayAdapter.h"
#include <Wire.h>

TFT_eSPI::TFT_eSPI()
    : _oled(nullptr),
      _rotation(0),
      _cursorX(0), _cursorY(0),
      _fgColor(TFT_WHITE),
      _bgColor(TFT_BLACK),
      _textSize(1),
      _textWrap(true),
      _textDatum(TL_DATUM),
      _font(1),
      _autoDisplay(true) {}

void TFT_eSPI::init(uint16_t /*tc*/) {
    if (!_oled) {
        _oled = new Adafruit_SSD1306(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
    }
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!_oled->begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        Serial.println(F("[DisplayAdapter] SSD1306 allocation failed"));
        return;
    }

    // WHITE BACKGROUND initialization.
    //
    // With our color mapping, dark source colors map to SSD1306_WHITE (pixel ON
    // in buffer = panel LIT = WHITE on screen). So calling fillScreen(TFT_BLACK)
    // — which the project interprets as "paint everything black" — actually
    // paints the OLED WHITE, giving the user the requested white background.
    //
    // After this, the project's normal UI flow calls fillScreen(UI_BG=BG_Dark)
    // which is also a dark source color → also renders WHITE on the OLED.
    // Light source colors (TFT_WHITE, UI_TEXT in dark theme, etc.) map to
    // SSD1306_BLACK (pixel OFF = panel DARK = BLACK ink on the white field).
    _oled->invertDisplay(false);   // normal panel polarity

    fillScreen(TFT_BLACK);   // → WHITE on panel (white background)
    _oled->display();
    setRotation(0);

    Serial.println(F("[DisplayAdapter] OLED 0.96\" ready (white background)"));
}

void TFT_eSPI::setRotation(uint8_t r) {
    _rotation = r & 0x03;
    if (_oled) {
        // Keep OLED in landscape so we get 128 wide x 64 tall.
        // r=0,2 -> landscape; r=1,3 -> portrait. We always use landscape.
        _oled->setRotation(_rotation & 1 ? 3 : 0);
    }
}

void TFT_eSPI::invertDisplay(bool /*invert*/) {
    // Intentionally a no-op: the panel is already configured for white background.
}

void TFT_eSPI::display() {
    if (_oled) _oled->display();
}

void TFT_eSPI::flush() {
    if (_oled && _autoDisplay) _oled->display();
}

// ---------- Color helpers ----------
// Compute perceived luminance of a 16-bit 565 color and classify it as
// "light" or "dark". Threshold chosen so that:
//   - TFT_BLACK (0x0000)         -> dark
//   - BG_Dark (0x20e4)           -> dark
//   - FG_Dark (0x3166)           -> dark
//   - L_Dark  (0x4208)           -> dark
//   - L_Light (0xC618)           -> light
//   - BG_Light (0xf7de)          -> light
//   - TFT_WHITE (0xFFFF)         -> light
//   - ORANGE (0xFD20)            -> light
bool TFT_eSPI::isLightColor(uint16_t c) {
    uint8_t r5 = (c >> 11) & 0x1F;
    uint8_t g6 = (c >> 5) & 0x3F;
    uint8_t b5 = c & 0x1F;
    // Weighted luminance (max = 31*4 + 63*8 + 31*4 = 124+504+124 = 752)
    uint16_t lum = (uint16_t)r5 * 4 + (uint16_t)g6 * 8 + (uint16_t)b5 * 4;
    // Threshold ~30% of max: 0x100 (256)
    return lum >= 256;
}

// ---------- Primitives ----------
void TFT_eSPI::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (!_oled) return;
    int16_t ox = sx(x), oy = sy(y);
    _oled->drawPixel(ox, oy, colorToInk(color));
    flush();
}

void TFT_eSPI::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (!_oled) return;
    _oled->drawFastVLine(sx(x), sy(y), sh(h), colorToInk(color));
    flush();
}

void TFT_eSPI::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (!_oled) return;
    _oled->drawFastHLine(sx(x), sy(y), sw(w), colorToInk(color));
    flush();
}

void TFT_eSPI::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (!_oled) return;
    _oled->drawLine(sx(x0), sy(y0), sx(x1), sy(y1), colorToInk(color));
    flush();
}

void TFT_eSPI::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!_oled) return;
    _oled->drawRect(sx(x), sy(y), sw(w), sh(h), colorToInk(color));
    flush();
}

void TFT_eSPI::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (!_oled) return;
    int16_t rr = sw(r);
    if (rr < 1) rr = 1;
    _oled->drawRoundRect(sx(x), sy(y), sw(w), sh(h), rr, colorToInk(color));
    flush();
}

void TFT_eSPI::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!_oled) return;
    _oled->fillRect(sx(x), sy(y), sw(w), sh(h), colorToBg(color));
    flush();
}

void TFT_eSPI::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (!_oled) return;
    int16_t rr = sw(r);
    if (rr < 1) rr = 1;
    _oled->fillRoundRect(sx(x), sy(y), sw(w), sh(h), rr, colorToBg(color));
    flush();
}

void TFT_eSPI::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (!_oled) return;
    int16_t rr = (sw(r) + sh(r)) / 2;
    if (rr < 1) rr = 1;
    _oled->drawCircle(sx(x), sy(y), rr, colorToInk(color));
    flush();
}

void TFT_eSPI::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (!_oled) return;
    int16_t rr = (sw(r) + sh(r)) / 2;
    if (rr < 1) rr = 1;
    _oled->fillCircle(sx(x), sy(y), rr, colorToBg(color));
    flush();
}

void TFT_eSPI::drawCircleHelper(int16_t x, int16_t y, int16_t r, uint8_t corner, uint16_t color) {
    if (!_oled) return;
    int16_t rr = (sw(r) + sh(r)) / 2;
    if (rr < 1) rr = 1;
    _oled->drawCircleHelper(sx(x), sy(y), rr, corner, colorToInk(color));
    flush();
}

void TFT_eSPI::fillCircleHelper(int16_t x, int16_t y, int16_t r, uint8_t corner, int16_t delta, uint16_t color) {
    if (!_oled) return;
    int16_t rr = (sw(r) + sh(r)) / 2;
    if (rr < 1) rr = 1;
    _oled->fillCircleHelper(sx(x), sy(y), rr, corner, delta, colorToBg(color));
    flush();
}

void TFT_eSPI::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (!_oled) return;
    _oled->drawTriangle(sx(x0), sy(y0), sx(x1), sy(y1), sx(x2), sy(y2), colorToInk(color));
    flush();
}

void TFT_eSPI::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (!_oled) return;
    _oled->fillTriangle(sx(x0), sy(y0), sx(x1), sy(y1), sx(x2), sy(y2), colorToBg(color));
    flush();
}

void TFT_eSPI::fillScreen(uint16_t color) {
    if (!_oled) return;
    _oled->fillScreen(colorToBg(color));
    flush();
}

// ---------- Bitmaps ----------
void TFT_eSPI::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                          int16_t w, int16_t h, uint16_t color) {
    if (!_oled) return;
    int16_t ox = sx(x), oy = sy(y);
    int16_t ow = sw(w), oh = sh(h);
    // If we can render at the exact scaled size, use Adafruit_GFX drawBitmap (1-bit).
    // Otherwise fall back to drawing pixel-by-pixel with non-uniform scaling.
    if (ow == w && oh == h) {
        _oled->drawBitmap(ox, oy, bitmap, ow, oh, colorToInk(color));
    } else {
        // Scale 1-bit XBM-style bitmap into OLED coordinates.
        int16_t srcRowBytes = (w + 7) / 8;
        for (int16_t row = 0; row < oh; ++row) {
            int16_t srcRow = (int32_t)row * h / oh;
            for (int16_t col = 0; col < ow; ++col) {
                int16_t srcCol = (int32_t)col * w / ow;
                uint16_t byte = pgm_read_byte(&bitmap[srcRow * srcRowBytes + (srcCol / 8)]);
                if (byte & (0x80 >> (srcCol & 7))) {
                    _oled->drawPixel(ox + col, oy + row, colorToInk(color));
                }
            }
        }
    }
    flush();
}

void TFT_eSPI::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                          int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    if (!_oled) return;
    int16_t ox = sx(x), oy = sy(y);
    int16_t ow = sw(w), oh = sh(h);
    // Pre-fill the bounding rect with the bg color (in OLED space), then draw
    // the foreground pixels on top — works for both scaled and unscaled paths.
    _oled->fillRect(ox, oy, ow, oh, colorToBg(bg));
    if (ow == w && oh == h) {
        _oled->drawBitmap(ox, oy, bitmap, ow, oh, colorToInk(color));
    } else {
        int16_t srcRowBytes = (w + 7) / 8;
        for (int16_t row = 0; row < oh; ++row) {
            int16_t srcRow = (int32_t)row * h / oh;
            for (int16_t col = 0; col < ow; ++col) {
                int16_t srcCol = (int32_t)col * w / ow;
                uint16_t byte = pgm_read_byte(&bitmap[srcRow * srcRowBytes + (srcCol / 8)]);
                if (byte & (0x80 >> (srcCol & 7))) {
                    _oled->drawPixel(ox + col, oy + row, colorToInk(color));
                }
            }
        }
    }
    flush();
}

void TFT_eSPI::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                           int16_t w, int16_t h, uint16_t color) {
    if (!_oled) return;
    int16_t ox = sx(x), oy = sy(y);
    int16_t ow = sw(w), oh = sh(h);
    _oled->drawXBitmap(ox, oy, bitmap, ow, oh, colorToInk(color));
    flush();
}

void TFT_eSPI::pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data) {
    if (!_oled) return;
    int16_t ox = sx(x), oy = sy(y);
    int16_t ow = sw(w), oh = sh(h);
    for (int16_t row = 0; row < oh; ++row) {
        int16_t srcRow = (int32_t)row * h / oh;
        for (int16_t col = 0; col < ow; ++col) {
            int16_t srcCol = (int32_t)col * w / ow;
            uint16_t c = data[srcRow * w + srcCol];
            _oled->drawPixel(ox + col, oy + row, colorToInk(c));
        }
    }
    flush();
}

void TFT_eSPI::pushRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data) {
    pushImage(x, y, w, h, data);
}

// ---------- Text ----------
void TFT_eSPI::setCursor(int16_t x, int16_t y) {
    _cursorX = x;
    _cursorY = y;
}

void TFT_eSPI::setCursor(int16_t x, int16_t y, uint8_t font) {
    _cursorX = x;
    _cursorY = y;
    setTextFont(font);
}

void TFT_eSPI::setTextColor(uint16_t c) {
    _fgColor = c;
    _bgColor = TFT_BLACK;  // transparent background
}

void TFT_eSPI::setTextColor(uint16_t c, uint16_t bg) {
    _fgColor = c;
    _bgColor = bg;
}

void TFT_eSPI::setTextSize(uint8_t s) {
    _textSize = s;
}

void TFT_eSPI::setTextWrap(bool w) {
    _textWrap = w;
}

void TFT_eSPI::setTextDatum(uint8_t d) {
    _textDatum = d;
}

void TFT_eSPI::setTextFont(uint8_t f) {
    _font = f;
}

int16_t TFT_eSPI::textWidth(const char *str) {
    if (!str) return 0;
    // Adafruit_GFX default font: 5px per char + 1px spacing, scaled by text size.
    // We approximate original TFT_eSPI font widths:
    //   font 1 -> 6px/char; font 2 -> 12px/char; font 4 -> 16px/char.
    int16_t perChar;
    switch (_font) {
        case 2:  perChar = 12; break;
        case 4:  perChar = 16; break;
        case 6:  perChar = 14; break;
        case 7:  perChar = 16; break;
        case 8:  perChar = 18; break;
        default: perChar = 6;  break;
    }
    return (int16_t)strlen(str) * perChar * (_textSize ? _textSize : 1);
}

int16_t TFT_eSPI::fontHeight(int8_t /*font*/) {
    switch (_font) {
        case 2:  return 16 * (_textSize ? _textSize : 1);
        case 4:  return 20 * (_textSize ? _textSize : 1);
        case 6:  return 14 * (_textSize ? _textSize : 1);
        case 7:  return 18 * (_textSize ? _textSize : 1);
        case 8:  return 22 * (_textSize ? _textSize : 1);
        default: return 8  * (_textSize ? _textSize : 1);
    }
}

size_t TFT_eSPI::write(uint8_t c) {
    if (!_oled) return 1;
    int16_t ox = sx(_cursorX), oy = sy(_cursorY);
    uint8_t  ts = scaledTextSize();
    uint16_t fg = colorToInk(_fgColor);
    uint16_t bg = (_bgColor == _fgColor) ? fg : colorToBg(_bgColor);
    // Adafruit_GFX drawChar advances cursor internally — but we manage cursor
    // ourselves in original space so rendering stays consistent.
    _oled->setCursor(ox, oy);
    _oled->setTextSize(ts);
    _oled->setTextColor(fg, bg);
    _oled->setTextWrap(_textWrap);
    _oled->write(c);
    flush();

    // Advance cursor in ORIGINAL coordinate space (so subsequent print()s chain).
    // Approximate char advance: 6 * textSize (matches Adafruit_GFX default).
    _cursorX += 6 * (_textSize ? _textSize : 1);
    return 1;
}

size_t TFT_eSPI::write(const uint8_t *buffer, size_t size) {
    size_t n = 0;
    for (size_t i = 0; i < size; ++i) {
        if (buffer[i] == '\n') {
            _cursorX = 0;
            _cursorY += fontHeight();
        } else if (buffer[i] == '\r') {
            // ignore
        } else {
            n += write(buffer[i]);
        }
    }
    return n;
}

void TFT_eSPI::drawChar(int16_t x, int16_t y, unsigned char c,
                        uint16_t color, uint16_t bg, uint8_t size) {
    if (!_oled) return;
    _oled->drawChar(sx(x), sy(y), c, colorToInk(color), colorToBg(bg), size);
    flush();
}

void TFT_eSPI::drawString(const char *str, int16_t x, int16_t y) {
    int16_t saveX = _cursorX, saveY = _cursorY;
    setCursor(x, y);
    for (const char *p = str; *p; ++p) {
        write((uint8_t)*p);
    }
    _cursorX = saveX;
    _cursorY = saveY;
}

void TFT_eSPI::drawCentreString(const char *str, int16_t x, int16_t y, uint8_t) {
    int16_t w = textWidth(str);
    drawString(str, x - w / 2, y);
}

void TFT_eSPI::drawCentreString(const String &s, int16_t x, int16_t y, uint8_t f) {
    drawCentreString(s.c_str(), x, y, f);
}

void TFT_eSPI::drawRightString(const char *str, int16_t x, int16_t y, uint8_t) {
    int16_t w = textWidth(str);
    drawString(str, x - w, y);
}

void TFT_eSPI::drawRightString(const String &s, int16_t x, int16_t y, uint8_t f) {
    drawRightString(s.c_str(), x, y, f);
}

void TFT_eSPI::drawNumber(long n, int16_t x, int16_t y) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%ld", n);
    drawString(buf, x, y);
}

void TFT_eSPI::drawFloat(float fl, uint8_t dp, int16_t x, int16_t y) {
    char buf[24];
    snprintf(buf, sizeof(buf), "%.*f", dp, fl);
    drawString(buf, x, y);
}
