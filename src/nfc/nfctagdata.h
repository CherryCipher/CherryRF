#pragma once

#include <Arduino.h>
#include <vector>

#include "NFCTag.h"
#include "NDEFManager.h"

enum class CherryRFDataType
{
    NONE,
    TEXT,
    URI
};

struct NFCTagData
{
    NFCTag tag;

    std::vector<uint8_t> rawData;

    bool hasNDEF = false;
    NDEFMessage ndef;

    bool hasCherryRFData = false;
    CherryRFDataType cherryRFType = CherryRFDataType::NONE;
    String cherryRFValue;

    void clear()
    {
        tag.clear();
        rawData.clear();

        hasNDEF = false;
        ndef.clear();

        hasCherryRFData = false;
        cherryRFType = CherryRFDataType::NONE;
        cherryRFValue = "";
    }

    String getCherryRFTypeName() const
    {
        switch (cherryRFType)
        {
            case CherryRFDataType::TEXT:
                return "Text";

            case CherryRFDataType::URI:
                return "URI";

            default:
                return "Unknown";
        }
    }
};