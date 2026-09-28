#pragma once

#include <Arduino.h>
#include <vector>

#include "../NFCTag.h"

class NFCTagHandler
{
public:
    virtual ~NFCTagHandler() = default;

    virtual bool read(std::vector<uint8_t>& data) = 0;
    virtual bool write(const std::vector<uint8_t>& data) = 0;

    virtual size_t capacity() const = 0;

    virtual bool isReadable() const = 0;
    virtual bool isWritable() const = 0;

    virtual bool supports(const NFCTag& tag) const = 0;
};