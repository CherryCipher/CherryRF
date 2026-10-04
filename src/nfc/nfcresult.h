#pragma once

#include <Arduino.h>

enum class NFCResult
{
    OK,
    NO_TAG,
    UNSUPPORTED_TAG,
    READ_FAILED,
    WRITE_FAILED,
    AUTH_FAILED,
    READ_ONLY,
    NOT_ENOUGH_SPACE,
    INCOMPATIBLE_TAG,
    INVALID_FORMAT,
    VERIFY_FAILED
};

inline String getNFCResultName(NFCResult result)
{
    switch (result)
    {
        case NFCResult::OK:               return "OK";
        case NFCResult::NO_TAG:           return "NO TAG";
        case NFCResult::UNSUPPORTED_TAG:   return "UNSUPPORTED TAG";
        case NFCResult::READ_FAILED:       return "READ FAILED";
        case NFCResult::WRITE_FAILED:      return "WRITE FAILED";
        case NFCResult::AUTH_FAILED:       return "AUTH FAILED";
        case NFCResult::READ_ONLY:         return "READ ONLY";
        case NFCResult::NOT_ENOUGH_SPACE:  return "TAG TOO SMALL";
        case NFCResult::INCOMPATIBLE_TAG:  return "WRONG TAG";
        case NFCResult::INVALID_FORMAT:    return "INVALID FORMAT";
        case NFCResult::VERIFY_FAILED:     return "VERIFY FAILED";
        default:                           return "UNKNOWN";
    }
}