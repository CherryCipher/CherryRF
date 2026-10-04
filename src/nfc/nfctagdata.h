#pragma once

#include <Arduino.h>
#include <vector>

#include "NFCTag.h"
#include "NDEFManager.h"

struct NFCTagData
{
    NFCTag tag;

    std::vector<uint8_t> rawData;

    bool hasNDEF = false;
    NDEFMessage ndef;

    void clear()
    {
        tag.clear();
        rawData.clear();
        hasNDEF = false;
        ndef.clear();
    }
};