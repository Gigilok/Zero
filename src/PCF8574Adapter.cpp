/*
 * PCF8574Adapter.cpp
 *
 * Reads physical push-buttons wired to ESP32 GPIO pins in lieu of a PCF8574.
 * See PCF8574Adapter.h for the wiring table.
 */
#include "PCF8574Adapter.h"
#include "shared.h"   // for BTN_UP/BTN_DOWN/BTN_LEFT/BTN_RIGHT/BTN_SELECT

// PCF8574 pin index -> ESP32 GPIO. Order matches BTN_* in shared.h for the
// BOARD_ESP32_WROOM_OLED target. The mapping is shared here so any code that
// calls pcf.digitalRead(BTN_x) gets the right physical pin.
static const int8_t kGpioMap[8] = {
    /* BTN_UP     = 0 */ 5,
    /* BTN_DOWN   = 1 */ 27,
    /* BTN_LEFT   = 2 */ 33,    // black wire (button 4) -> BTN_LEFT
    /* BTN_RIGHT  = 3 */ -1,    // unused (no 5th button)
    /* BTN_SELECT = 4 */ 32,
    /* 5          */ -1,
    /* 6          */ -1,
    /* 7          */ -1,
};

const int8_t *PCF8574::_gpioMap() { return kGpioMap; }

int8_t PCF8574::gpioFor(uint8_t pin) {
    if (pin >= sizeof(kGpioMap)) return -1;
    return kGpioMap[pin];
}

bool PCF8574::begin(uint8_t addr) {
    _addr = addr;
    _begun = true;
    // Configure all button GPIOs as inputs with pull-ups.
    for (uint8_t i = 0; i < sizeof(kGpioMap); ++i) {
        int8_t g = kGpioMap[i];
        if (g < 0) continue;
        ::pinMode(g, INPUT_PULLUP);
    }
    Serial.println(F("[PCF8574Adapter] buttons configured on GPIO 5/27/32/33"));
    return true;
}

void PCF8574::pinMode(uint8_t pin, uint8_t mode) {
    int8_t g = gpioFor(pin);
    if (g < 0) return;
    ::pinMode(g, mode);
}

void PCF8574::digitalWrite(uint8_t pin, uint8_t value) {
    int8_t g = gpioFor(pin);
    if (g < 0) return;
    ::digitalWrite(g, value);
}

uint8_t PCF8574::digitalRead(uint8_t pin) {
    int8_t g = gpioFor(pin);
    if (g < 0) return 1;  // unconnected pin reads HIGH (button not pressed)
    return ::digitalRead(g);
}
