/*
 * SDFlashAdapter.cpp
 *
 * SD.h shim backed by LittleFS on the ESP32's internal flash.
 * See SDFlashAdapter.h for behavior notes.
 */
#include "SDFlashAdapter.h"

SDClass SD;

bool SDClass::begin(uint8_t /*csPin*/, SPIClass & /*spi*/, uint32_t /*cfg*/) {
    if (_mounted) return true;
    if (!LittleFS.begin(true)) {
        Serial.println(F("[SDFlashAdapter] LittleFS mount failed"));
        _mounted = false;
        return false;
    }
    _mounted = true;
    Serial.println(F("[SDFlashAdapter] LittleFS mounted (flash storage ready)"));

    // Create the standard directories the project expects, ignoring errors if
    // they already exist.
    LittleFS.mkdir("/logs");
    LittleFS.mkdir("/captures");
    LittleFS.mkdir("/config");
    return true;
}

void SDClass::end() {
    if (_mounted) {
        LittleFS.end();
        _mounted = false;
    }
}

bool SDClass::ensureMounted() {
    if (_mounted) return true;
    return begin();
}

bool SDClass::exists(const char *path) {
    if (!ensureMounted()) return false;
    if (!path) return false;
    // LittleFS does not consider "/" to exist as a file but treats it as the root.
    if (strcmp(path, "/") == 0) return true;
    return LittleFS.exists(path);
}

fs::File SDClass::open(const char *path, const char *mode) {
    if (!ensureMounted()) return fs::File();
    if (!path) return fs::File();
    // The project sometimes opens paths like "foo.txt" (no leading slash).
    // LittleFS requires absolute paths starting with "/". Normalize.
    String p = path;
    if (!p.startsWith("/")) p = "/" + p;
    return LittleFS.open(p, mode);
}

bool SDClass::remove(const char *path) {
    if (!ensureMounted()) return false;
    if (!path) return false;
    String p = path;
    if (!p.startsWith("/")) p = "/" + p;
    return LittleFS.remove(p);
}

bool SDClass::mkdir(const char *path) {
    if (!ensureMounted()) return false;
    if (!path) return false;
    String p = path;
    if (!p.startsWith("/")) p = "/" + p;
    return LittleFS.mkdir(p);
}

bool SDClass::rmdir(const char *path) {
    if (!ensureMounted()) return false;
    if (!path) return false;
    String p = path;
    if (!p.startsWith("/")) p = "/" + p;
    return LittleFS.rmdir(p);
}

uint8_t SDClass::cardType() {
    // Pretend to be an SDHC card so callers that check `cardType() != CARD_NONE`
    // see "yes, storage is present".
    return CARD_SDHC;
}

uint64_t SDClass::cardSize() {
    return totalBytes();
}

uint64_t SDClass::totalBytes() {
    if (!ensureMounted()) return 0;
    return LittleFS.totalBytes();
}

uint64_t SDClass::usedBytes() {
    if (!ensureMounted()) return 0;
    return LittleFS.usedBytes();
}

void SDClass::infoPrint() {
    Serial.printf("[SDFlashAdapter] total=%llu used=%llu free=%llu\r\n",
                  (unsigned long long)totalBytes(),
                  (unsigned long long)usedBytes(),
                  (unsigned long long)(totalBytes() - usedBytes()));
}
