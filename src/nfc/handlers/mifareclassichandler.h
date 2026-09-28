#pragma once

#include <Arduino.h>
#include <vector>

#include "NFCTagHandler.h"
#include "../NFCManager.h"

class MifareClassicHandler : public NFCTagHandler
{
public:
    MifareClassicHandler(NFCManager& nfc, const NFCTag& tag);

    bool read(std::vector<uint8_t>& data) override;
    bool write(const std::vector<uint8_t>& data) override;

    size_t capacity() const override;

    bool isReadable() const override;
    bool isWritable() const override;

    bool supports(const NFCTag& tag) const override;

    void setKey(const uint8_t* key, bool keyB = false);

private:
    bool authenticate(uint8_t block);
    bool isSectorTrailer(uint8_t block) const;

private:
    NFCManager& nfc;
    const NFCTag& tag;

    uint8_t key[6] =
    {
        0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF
    };

    bool useKeyB = false;

    static constexpr uint8_t TOTAL_BLOCKS = 64;
    static constexpr uint8_t BLOCK_SIZE = 16;
    static constexpr uint8_t FIRST_DATA_BLOCK = 1;
};