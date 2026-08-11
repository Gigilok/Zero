#pragma once

/*
 * BleCompat.h
 *
 * Aliases the original Arduino BLE API names (BLEDevice, BLEServer, BLEScan,
 * BLEAdvertising, BLEAdvertisementData, BLEScanResults, BLEAdvertisedDevice,
 * BLEAdvertisedDeviceCallbacks, BLEAddress, BLEUUID) to their NimBLE
 * counterparts. This lets the project's bluetooth.cpp / gps.cpp / ducky.cpp
 * compile unmodified after switching from the deprecated built-in Arduino BLE
 * stack to NimBLE-Arduino (which uses far less RAM).
 *
 * REQUIRED LIBRARY: h2zero/NimBLE-Arduino  (>= 1.4.0)
 *   - Arduino IDE:    Library Manager → search "NimBLE-Arduino"
 *   - PlatformIO:     lib_deps = h2zero/NimBLE-Arduino
 *                     (already declared in platformio.ini)
 *
 * The project uses BLE in many namespaces (BleJammer, BleSpoofer, SourApple,
 * AirTagSpoofer, AirTagSniffer, BleSkimmer, BleScan, BleSniffer, MouseJack,
 * MouseJackInject, ProtoKill, EsbSniffer, EsbReplay). All of them depend on
 * NimBLE, so this header MUST compile. If you intentionally want to disable
 * BLE, define ESP32DIV_NO_BLE and the project's BLE entry points will be
 * stubbed (see config.h) — but you should not need this; just install
 * NimBLE-Arduino.
 */

#if defined(ESP32DIV_NO_BLE)
  /* BLE intentionally disabled. Provide stub typedefs so callers still compile. */
  #include <Arduino.h>
  namespace NimBLECompatStubs {
    class StubBLE { public: static void init(const String&) {} static void deinit() {} };
  }
  using BLEDevice                    = NimBLECompatStubs::StubBLE;
  using BLEServer                    = void;
  using BLEAdvertising               = void;
  using BLEAdvertisementData         = void;
  using BLEScan                      = void;
  using BLEScanResults               = void;
  using BLEAdvertisedDevice          = void;
  using BLEAdvertisedDeviceCallbacks = void;
  using BLEAddress                   = void;
  using BLEUUID                      = void;
#else
  #if __has_include(<NimBLEDevice.h>)
    #include <NimBLEDevice.h>

    using BLEDevice                 = NimBLEDevice;
    using BLEServer                 = NimBLEServer;
    using BLEAdvertising            = NimBLEAdvertising;
    using BLEAdvertisementData      = NimBLEAdvertisementData;
    using BLEScan                   = NimBLEScan;
    using BLEScanResults            = NimBLEScanResults;
    using BLEAdvertisedDevice       = NimBLEAdvertisedDevice;
    using BLEAdvertisedDeviceCallbacks = NimBLEAdvertisedDeviceCallbacks;
    using BLEAddress                = NimBLEAddress;
    using BLEUUID                   = NimBLEUUID;
  #else
    #error "NimBLEDevice.h not found. Install NimBLE-Arduino (h2zero/NimBLE-Arduino). " \
           "Arduino IDE: Sketch > Include Library > Manage Libraries > search 'NimBLE-Arduino'. " \
           "PlatformIO:  add 'lib_deps = h2zero/NimBLE-Arduino' to platformio.ini " \
           "(already declared in the platformio.ini shipped with this project — " \
           "run 'pio lib install' or open the project fresh so deps are fetched)."
  #endif
#endif
