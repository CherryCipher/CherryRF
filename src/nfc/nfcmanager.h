#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

#include "NFCTag.h"
#include "NFCTagData.h"
#include "NFCResult.h"

class NFCManager
{
public:
    NFCManager(uint8_t sda, uint8_t scl);

    bool start();
    bool isRunning() const;

    bool scan(NFCTag& tag, uint16_t timeout = 50);

    NFCResult readTag(NFCTagData& data, uint16_t timeout = 250);
    NFCResult readDetectedTag(const NFCTag& tag, NFCTagData& data);

    NFCResult writeText(const String& text, uint16_t timeout = 250);
    NFCResult writeTagData(const NFCTagData& source, uint16_t timeout = 250);

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

private:
    void detectTagType(NFCTag& tag);

    bool detectNTAG();
    bool detectClassic(const NFCTag& tag);

    NFCResult writeNDEFToType2(
        const NFCTag& tag,
        const std::vector<uint8_t>& ndef
    );

    NFCResult verifyType2NDEF(
        const std::vector<uint8_t>& expected
    );

private:
    uint8_t sda;
    uint8_t scl;

    Adafruit_PN532 nfc;

    bool running = false;
};