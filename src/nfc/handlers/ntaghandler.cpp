#include "NTAGHandler.h"

NTAGHandler::NTAGHandler(NFCManager& nfc, const NFCTag& tag)
    : nfc(nfc), tag(tag)
{
    detectCapacity();
}

bool NTAGHandler::supports(const NFCTag& tag) const
{
    return tag.type == NFCTagType::NTAG_ULTRALIGHT;
}

bool NTAGHandler::isReadable() const
{
    return supports(tag) && tag.readable;
}

bool NTAGHandler::isWritable() const
{
    return supports(tag) && tag.writable;
}

void NTAGHandler::detectCapacity()
{
    detectedCapacity = 0;

    uint8_t data[4] = {0};

    for (uint16_t page = FIRST_USER_PAGE; page <= 255; page++)
    {
        if (!nfc.readPage((uint8_t)page, data))
            break;

        detectedCapacity += 4;
    }
}

size_t NTAGHandler::capacity() const
{
    return detectedCapacity;
}

bool NTAGHandler::read(std::vector<uint8_t>& data)
{
    data.clear();

    if (!isReadable()) return false;

    if (detectedCapacity == 0) return false;

    data.reserve(detectedCapacity);

    uint8_t pageData[4] = {0};

    size_t pageCount = detectedCapacity / 4;

    for (size_t i = 0; i < pageCount; i++)
    {
        uint8_t page = FIRST_USER_PAGE + i;

        if (!nfc.readPage(page, pageData))
        {
            data.clear();
            return false;
        }

        data.insert(
            data.end(),
            pageData,
            pageData + 4
        );
    }

    return true;
}

bool NTAGHandler::write(const std::vector<uint8_t>& data)
{
    if (!isWritable()) return false;
    if (detectedCapacity == 0) return false;
    if (data.size() > detectedCapacity) return false;

    size_t offset = 0;
    uint8_t page = FIRST_USER_PAGE;

    while (offset < data.size())
    {
        uint8_t pageData[4] = {0};

        size_t remaining = data.size() - offset;
        size_t bytesToCopy = remaining >= 4 ? 4 : remaining;

        memcpy(
            pageData,
            data.data() + offset,
            bytesToCopy
        );

        if (!nfc.writePage(page, pageData))
            return false;

        offset += bytesToCopy;
        page++;
    }

    return true;
}