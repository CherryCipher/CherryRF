#pragma once

#include <Arduino.h>
#include <vector>

#include "NFCTagHandler.h"
#include "../NFCManager.h"
#include "../formats/Type2TagFormat.h"

class Type2TagHandler : public NFCTagHandler
{
public:
    Type2TagHandler(NFCManager& nfc, const NFCTag& tag);

    bool read(std::vector<uint8_t>& data) override;
    bool write(const std::vector<uint8_t>& data) override;

    size_t capacity() const override;

    bool isReadable() const override;
    bool isWritable() const override;

    bool supports(const NFCTag& tag) const override;

    bool getInfo(Type2TagInfo& info);

private:
    bool loadInfo();

    NFCManager& nfc;
    const NFCTag& tag;

    Type2TagInfo info;

    static constexpr uint8_t CC_PAGE = 3;
    static constexpr uint8_t FIRST_DATA_PAGE = 4;
};