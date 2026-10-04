#pragma once

#include <Arduino.h>
#include <vector>

struct Type2TagInfo
{
    bool valid = false;
    bool readable = false;
    bool writable = false;

    uint8_t mappingVersion = 0;

    size_t dataAreaSize = 0;

    size_t ndefOffset = 0;
    size_t ndefLength = 0;

    bool hasNDEF = false;
};

class Type2TagFormat
{
public:
    static bool parseCapabilityContainer(const uint8_t* page3, Type2TagInfo& info);

    static bool findNDEF(
        const std::vector<uint8_t>& dataArea,
        Type2TagInfo& info,
        std::vector<uint8_t>& ndef
    );

    static bool buildNDEFDataArea(
        const std::vector<uint8_t>& current,
        const std::vector<uint8_t>& ndef,
        std::vector<uint8_t>& output
    );

private:
    static bool readLength(
        const std::vector<uint8_t>& data,
        size_t& offset,
        size_t& length
    );
};