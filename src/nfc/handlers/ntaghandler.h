#pragma once

#include <Arduino.h>
#include <vector>

#include "NFCTagHandler.h"
#include "../NFCManager.h"

class NTAGHandler : public NFCTagHandler
{
public:
    NTAGHandler(NFCManager& nfc, const NFCTag& tag);

    bool read(std::vector<uint8_t>& data) override;
    bool write(const std::vector<uint8_t>& data) override;

    size_t capacity() const override;

    bool isReadable() const override;
    bool isWritable() const override;

    bool supports(const NFCTag& tag) const override;

private:
    void detectCapacity();

private:
    NFCManager& nfc;
    const NFCTag& tag;

    size_t detectedCapacity = 0;

    static constexpr uint8_t FIRST_USER_PAGE = 4;
};