#include "Type2TagFormat.h"

bool Type2TagFormat::parseCapabilityContainer(const uint8_t* page3, Type2TagInfo& info)
{
    info = Type2TagInfo();

    if (!page3) return false;
    if (page3[0] != 0xE1) return false;

    info.mappingVersion = page3[1];
    info.dataAreaSize = (size_t)page3[2] * 8;

    uint8_t readAccess = (page3[3] >> 4) & 0x0F;
    uint8_t writeAccess = page3[3] & 0x0F;

    info.readable = readAccess == 0x00;
    info.writable = writeAccess == 0x00;

    info.valid = info.dataAreaSize > 0;

    return info.valid;
}

bool Type2TagFormat::readLength(const std::vector<uint8_t>& data, size_t& offset, size_t& length)
{
    if (offset >= data.size()) return false;

    uint8_t value = data[offset++];

    if (value != 0xFF)
    {
        length = value;
        return true;
    }

    if (offset + 2 > data.size()) return false;

    length = ((size_t)data[offset] << 8) | data[offset + 1];
    offset += 2;

    return true;
}

bool Type2TagFormat::findNDEF(
    const std::vector<uint8_t>& dataArea,
    Type2TagInfo& info,
    std::vector<uint8_t>& ndef)
{
    ndef.clear();

    size_t offset = 0;

    while (offset < dataArea.size())
    {
        uint8_t type = dataArea[offset++];

        if (type == 0x00) continue;
        if (type == 0xFE) break;

        size_t length = 0;

        if (!readLength(dataArea, offset, length)) return false;
        if (offset + length > dataArea.size()) return false;

        if (type == 0x03)
        {
            info.hasNDEF = true;
            info.ndefOffset = offset;
            info.ndefLength = length;

            ndef.assign(
                dataArea.begin() + offset,
                dataArea.begin() + offset + length
            );

            return true;
        }

        offset += length;
    }

    return true;
}

bool Type2TagFormat::buildNDEFDataArea(
    const std::vector<uint8_t>& current,
    const std::vector<uint8_t>& ndef,
    std::vector<uint8_t>& output)
{
    output = current;

    if (output.empty()) return false;

    /*
     * CherryRF only rebuilds the NFC Forum NDEF TLV area when it can do so
     * without overwriting another non-padding TLV.
     *
     * Blank Type-2 tags normally contain:
     *
     * 03 00 FE ...
     *
     * Existing NDEF tags contain:
     *
     * 03 LEN <NDEF> FE
     */

    size_t ndefTLV = SIZE_MAX;
    size_t terminator = SIZE_MAX;
    size_t offset = 0;

    while (offset < current.size())
    {
        size_t tlvStart = offset;
        uint8_t type = current[offset++];

        if (type == 0x00) continue;

        if (type == 0xFE)
        {
            terminator = tlvStart;
            break;
        }

        size_t length = 0;

        if (!readLength(current, offset, length)) return false;
        if (offset + length > current.size()) return false;

        if (type == 0x03)
        {
            ndefTLV = tlvStart;
            break;
        }

        /*
         * Lock Control and Memory Control TLVs describe reserved areas.
         * We preserve them and only place NDEF after them.
         */
        offset += length;
    }

    size_t start = ndefTLV;

    if (start == SIZE_MAX)
    {
        start = terminator != SIZE_MAX ? terminator : offset;
    }

    size_t lengthBytes = ndef.size() < 0xFF ? 1 : 3;
    size_t required = 1 + lengthBytes + ndef.size() + 1;

    if (start + required > output.size()) return false;

    /*
     * Never destroy TLVs preceding the NDEF TLV.
     * Everything from the NDEF position onward becomes the new NDEF TLV,
     * terminator and padding.
     */
    std::fill(output.begin() + start, output.end(), 0x00);

    size_t write = start;

    output[write++] = 0x03;

    if (ndef.size() < 0xFF)
    {
        output[write++] = (uint8_t)ndef.size();
    }
    else
    {
        output[write++] = 0xFF;
        output[write++] = (ndef.size() >> 8) & 0xFF;
        output[write++] = ndef.size() & 0xFF;
    }

    memcpy(output.data() + write, ndef.data(), ndef.size());
    write += ndef.size();

    output[write] = 0xFE;

    return true;
}