#include "MifareClassicHandler.h"

MifareClassicHandler::MifareClassicHandler(
    NFCManager& nfc,
    const NFCTag& tag)
    : nfc(nfc), tag(tag)
{
}

bool MifareClassicHandler::supports(const NFCTag& tag) const
{
    return tag.type == NFCTagType::MIFARE_CLASSIC;
}

bool MifareClassicHandler::isReadable() const
{
    return supports(tag) && tag.readable;
}

bool MifareClassicHandler::isWritable() const
{
    return supports(tag) && tag.writable;
}

void MifareClassicHandler::setKey(const uint8_t* newKey, bool keyB)
{
    if (!newKey) return;

    memcpy(key, newKey, sizeof(key));

    useKeyB = keyB;
}

bool MifareClassicHandler::authenticate(uint8_t block)
{
    return nfc.authenticateClassic(
        tag,
        block,
        key,
        useKeyB
    );
}

bool MifareClassicHandler::isSectorTrailer(uint8_t block) const
{
    return (block % 4) == 3;
}

size_t MifareClassicHandler::capacity() const
{
    /*
     * MIFARE Classic 1K:
     *
     * 64 blocks total
     * - block 0 manufacturer block
     * - 16 sector trailers
     *
     * 47 normal data blocks remain.
     */

    return 47 * BLOCK_SIZE;
}

bool MifareClassicHandler::read(std::vector<uint8_t>& data)
{
    data.clear();

    if (!isReadable()) return false;

    data.reserve(capacity());

    for (uint8_t block = FIRST_DATA_BLOCK; block < TOTAL_BLOCKS; block++)
    {
        if (isSectorTrailer(block))
            continue;

        if (!authenticate(block))
        {
            data.clear();
            return false;
        }

        uint8_t blockData[BLOCK_SIZE] = {0};

        if (!nfc.readClassicBlock(block, blockData))
        {
            data.clear();
            return false;
        }

        data.insert(
            data.end(),
            blockData,
            blockData + BLOCK_SIZE
        );
    }

    return true;
}

bool MifareClassicHandler::write(const std::vector<uint8_t>& data)
{
    if (!isWritable()) return false;
    if (data.size() > capacity()) return false;

    size_t offset = 0;

    for (uint8_t block = FIRST_DATA_BLOCK; block < TOTAL_BLOCKS; block++)
    {
        if (isSectorTrailer(block))
            continue;

        if (offset >= data.size())
            break;

        if (!authenticate(block))
            return false;

        uint8_t blockData[BLOCK_SIZE] = {0};

        size_t remaining = data.size() - offset;
        size_t bytesToCopy = remaining >= BLOCK_SIZE
            ? BLOCK_SIZE
            : remaining;

        memcpy(
            blockData,
            data.data() + offset,
            bytesToCopy
        );

        if (!nfc.writeClassicBlock(block, blockData))
            return false;

        offset += bytesToCopy;
    }

    return true;
}