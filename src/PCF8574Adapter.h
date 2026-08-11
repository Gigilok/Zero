/*
 * PCF8574Adapter.h
 *
 * Drop-in shim for the PCF8574 library that reads physical push-buttons wired
 * directly to ESP32 GPIO pins instead of through an I2C port expander.
 *
 * Original ESP32-DIV V2 uses a PCF8574 to read 5 push-buttons on a single I2C
 * bus. The WROOM 30-pin adaptation requested by the user wires the buttons
 * directly to GPIO pins:
 *
 *   BTN_UP     -> GPIO 5   (cinza)
 *   BTN_DOWN   -> GPIO 27  (vermelho)
 *   BTN_SELECT -> GPIO 32  (marrom)
 *   BTN_4      -> GPIO 33  (preto)  -- treated as BTN_LEFT or BTN_RIGHT by the project
 *
 * The shim maps the PCF8574 "button number" constants used by the project to
 * these physical GPIOs, so existing `pcf.digitalRead(BTN_UP)` calls keep
 * working unchanged.
 *
 * Buttons are wired active-low (one side to GND, the other to the GPIO with
 * the internal pull-up enabled). digitalRead() returns 0 while the button is
 * held down — matching PCF8574's INPUT_PULLUP behavior.
 */
#pragma once

#include <Arduino.h>

// The project's BTN_* constants are mapped in shared.h based on the board
// target. For BOARD_ESP32_WROOM_OLED we set:
//   BTN_UP     0
//   BTN_DOWN   1
//   BTN_LEFT   2   (button 4 — black wire, used as LEFT)
//   BTN_RIGHT  3   (unused — no 5th button)
//   BTN_SELECT 4
// and we map each index to a physical GPIO via the array below.

class PCF8574 {
public:
    PCF8574(uint8_t addr = 0x20) : _addr(addr), _begun(false) {}

    // The PCF8574 library's begin(addr) returns true when the I2C device acks.
    // We don't have an I2C device — we just configure the GPIOs as inputs with
    // pull-ups and return true so the project's `if (!pcf.begin(addr))` checks
    // treat this as "buttons available".
    bool begin(uint8_t addr);
    bool begin() { return begin(_addr); }
    bool begin(int sda, int scl) { (void)sda; (void)scl; return begin(_addr); }

    void pinMode(uint8_t pin, uint8_t mode);
    void digitalWrite(uint8_t pin, uint8_t value);
    uint8_t digitalRead(uint8_t pin);

    // No-op I2C address setters — kept for source compatibility.
    void setAddress(uint8_t addr) { _addr = addr; }
    uint8_t getAddress() { return _addr; }

private:
    uint8_t _addr;
    bool _begun;

    // Map PCF8574 pin index to physical ESP32 GPIO. The order is set in
    // shared.h's BTN_* constants for BOARD_ESP32_WROOM_OLED.
    static const int8_t *_gpioMap();
    static int8_t gpioFor(uint8_t pin);
};
