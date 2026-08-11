/*
 * XPT2046Adapter.h
 *
 * Stub for the XPT2046_Touchscreen library. The 0.96" OLED on the ESP32 WROOM
 * 30-pin board has no touch layer, so all touch reads return "not touched".
 *
 * The project source includes <XPT2046_Touchscreen.h> and calls:
 *   ts.begin(spi)
 *   ts.setRotation(r)
 *   ts.tirqTouched()
 *   ts.touched()
 *   ts.getPoint()         -> returns TS_Point
 *
 * We provide a minimal TS_Point and XPT2046_Touchscreen class that satisfy the
 * calls above without requiring the actual library to be installed.
 */
#pragma once

#include <Arduino.h>
#include <SPI.h>

struct TS_Point {
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
    TS_Point() {}
    TS_Point(int16_t x_, int16_t y_, int16_t z_) : x(x_), y(y_), z(z_) {}
};

class XPT2046_Touchscreen {
public:
    XPT2046_Touchscreen(uint8_t cs = 255, uint8_t irq = 255) : _cs(cs), _irq(irq) {}

    bool begin(SPIClass &spi = SPI) { (void)spi; return true; }
    bool begin() { return true; }

    void setRotation(uint8_t r) { _rotation = r; }
    uint8_t getRotation() const { return _rotation; }

    // No touch panel — always returns false.
    bool tirqTouched() { return false; }
    bool touched()     { return false; }
    TS_Point getPoint() { return TS_Point(0, 0, 0); }

    // Some sketches read this; expose it as a no-op pin number.
    uint8_t csPin() const { return _cs; }
    uint8_t irqPin() const { return _irq; }

private:
    uint8_t _cs;
    uint8_t _irq;
    uint8_t _rotation = 0;
};
