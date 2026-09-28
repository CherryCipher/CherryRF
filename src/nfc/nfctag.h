#pragma once

#include <Arduino.h>

enum class NFCTagType
{
    NONE,
    UNKNOWN,
    NTAG_ULTRALIGHT,
    MIFARE_CLASSIC
};

struct NFCTag
{
    NFCTagType type = NFCTagType::NONE;

    uint8_t uid[10] = {0};
    uint8_t uidLength = 0;

    size_t capacity = 0;

    bool readable = false;
    bool writable = false;
    bool ndefCapable = false;

    void clear()
    {
        type = NFCTagType::NONE;

        memset(uid, 0, sizeof(uid));
        uidLength = 0;

        capacity = 0;

        readable = false;
        writable = false;
        ndefCapable = false;
    }

    bool valid() const
    {
        return uidLength > 0 && type != NFCTagType::NONE;
    }

    String getUID() const
    {
        String result;

        for (uint8_t i = 0; i < uidLength; i++)
        {
            if (i > 0) result += ":";

            if (uid[i] < 0x10) result += "0";

            result += String(uid[i], HEX);
        }

        result.toUpperCase();

        return result;
    }

    String getTypeName() const
    {
        switch (type)
        {
            case NFCTagType::NTAG_ULTRALIGHT:
                return "NTAG / Ultralight";

            case NFCTagType::MIFARE_CLASSIC:
                return "MIFARE Classic";

            case NFCTagType::UNKNOWN:
                return "Unknown";

            default:
                return "None";
        }
    }
};