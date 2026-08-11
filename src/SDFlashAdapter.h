/*
 * SDFlashAdapter.h
 *
 * Drop-in shim that exposes the SD library API used by ESP32-DIV but stores
 * everything in the ESP32's internal flash via LittleFS.
 *
 * The original project includes <SD.h> and calls SD.begin(cs), SD.open(path),
 * SD.remove(path), SD.exists(path), SD.mkdir(path), SD.rmdir(path), and uses
 * a File class with read/write/seek/position/size/available/close/isDirectory/
 * openNextFile/name/readdir semantics. We provide all of those here.
 *
 * Files are stored under LittleFS root ("/"). When the project passes a path
 * like "/logs/foo.txt" or "foo.txt", both work.
 */
#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <FS.h>

// Bring fs::File into the global namespace so existing project code that uses
// `File f = SD.open(...)` compiles unchanged (mirrors what <SD.h> does).
using fs::File;

// --- SD card type constants used by the project (we lie; there's no card) ---
#define CARD_NONE      0
#define CARD_MMC       1
#define CARD_SD         2
#define CARD_SDHC       3

// Re-export the File type from FS.h so existing source compiles unchanged.
// (The SD library's File inherits from fs::File too, so the API matches.)
class SDClass {
public:
    bool begin(uint8_t csPin = 0, SPIClass &spi = SPI, uint32_t cfg = 4000000);
    void end();

    bool exists(const char *path);
    inline bool exists(const String &path) { return exists(path.c_str()); }

    // NOTE: In the Arduino-ESP32 core, FILE_READ / FILE_WRITE / FILE_APPEND are
    // defined in FS.h as string literals ("r", "w", "a"). They are therefore
    // `const char*`, not integers — so the classic SD library `uint8_t mode`
    // overload cannot be implemented here. All project call-sites already pass
    // FILE_READ / FILE_WRITE / FILE_APPEND, which route to the `const char*`
    // overload below.
    fs::File open(const char *path, const char *mode = FILE_READ);
    inline fs::File open(const String &path, const char *mode = FILE_READ) {
        return open(path.c_str(), mode);
    }

    bool remove(const char *path);
    inline bool remove(const String &path) { return remove(path.c_str()); }

    bool mkdir(const char *path);
    inline bool mkdir(const String &path) { return mkdir(path.c_str()); }

    bool rmdir(const char *path);
    inline bool rmdir(const String &path) { return rmdir(path.c_str()); }

    uint8_t cardType();
    uint64_t cardSize();    // bytes
    uint64_t totalBytes();  // bytes
    uint64_t usedBytes();   // bytes

    // Returns a fake SD card info map (some project paths print this to serial).
    void infoPrint();

private:
    bool _mounted;
    bool ensureMounted();
};

extern SDClass SD;

// Helper aliases / functions used by the project that aren't on the FS::File class.
// (e.g. SD.open returns SDLib::File in the original — we use fs::File from LittleFS.)
// Nothing else to declare here; the FS.h File class already has all the methods
// the project calls (read, write, available, close, seek, position, size, name,
// isDirectory, openNextFile, peek, flush, etc.).
