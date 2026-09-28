#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

#include "NFCTag.h"

class NFCManager
{
public:
    NFCManager(uint8_t sda, uint8_t scl);

    bool start();
    bool scan(NFCTag& tag, uint16_t timeout = 50);

    bool readPage(uint8_t page, uint8_t* data);
    bool writePage(uint8_t page, const uint8_t* data);

    bool authenticateClassic(
        const NFCTag& tag,
        uint8_t block,
        const uint8_t* key,
        bool keyB = false
    );

    bool readClassicBlock(uint8_t block, uint8_t* data);
    bool writeClassicBlock(uint8_t block, const uint8_t* data);

    bool isRunning() const;

private:
    void detectTagType(NFCTag& tag);

    bool detectNTAG();
    bool detectClassic(const NFCTag& tag);

private:
    uint8_t sda;
    uint8_t scl;

    bool running = false;

    Adafruit_PN532 nfc;
};