#include "Type2TagHandler.h"

Type2TagHandler::Type2TagHandler(NFCManager& nfc, const NFCTag& tag)
    : nfc(nfc), tag(tag)
{
    loadInfo();
}

bool Type2TagHandler::supports(const NFCTag& tag) const
{
    return tag.type == NFCTagType::NTAG_ULTRALIGHT;
}

bool Type2TagHandler::loadInfo()
{
    uint8_t cc[4] = {0};

    if (!nfc.readPage(CC_PAGE, cc)) return false;

    return Type2TagFormat::parseCapabilityContainer(cc, info);
}

bool Type2TagHandler::getInfo(Type2TagInfo& result)
{
    if (!info.valid && !loadInfo()) return false;

    result = info;
    return true;
}

bool Type2TagHandler::isReadable() const
{
    return supports(tag) && info.valid && info.readable;
}

bool Type2TagHandler::isWritable() const
{
    return supports(tag) && info.valid && info.writable;
}

size_t Type2TagHandler::capacity() const
{
    return info.valid ? info.dataAreaSize : 0;
}

bool Type2TagHandler::read(std::vector<uint8_t>& data)
{
    data.clear();

    if (!isReadable()) return false;

    data.reserve(info.dataAreaSize);

    size_t pageCount = (info.dataAreaSize + 3) / 4;

    for (size_t i = 0; i < pageCount; i++)
    {
        uint8_t page[4] = {0};

        if (!nfc.readPage(FIRST_DATA_PAGE + i, page))
        {
            data.clear();
            return false;
        }

        size_t remaining = info.dataAreaSize - data.size();
        size_t count = remaining >= 4 ? 4 : remaining;

        data.insert(data.end(), page, page + count);
    }

    return true;
}

bool Type2TagHandler::write(const std::vector<uint8_t>& data)
{
    if (!isWritable()) return false;
    if (data.size() != info.dataAreaSize) return false;

    size_t pageCount = (info.dataAreaSize + 3) / 4;

    for (size_t i = 0; i < pageCount; i++)
    {
        uint8_t page[4] = {0};

        size_t offset = i * 4;
        size_t remaining = data.size() - offset;
        size_t count = remaining >= 4 ? 4 : remaining;

        /*
         * Type-2 data areas described by the CC are page aligned for the
         * tags supported here. Preserve any unused bytes if that ever isn't
         * the case.
         */
        if (count < 4)
        {
            if (!nfc.readPage(FIRST_DATA_PAGE + i, page)) return false;
        }

        memcpy(page, data.data() + offset, count);

        if (!nfc.writePage(FIRST_DATA_PAGE + i, page)) return false;
    }

    return true;
}