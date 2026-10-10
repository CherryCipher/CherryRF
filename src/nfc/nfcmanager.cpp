#include "NFCManager.h"

#include "NDEFManager.h"
#include "handlers/Type2TagHandler.h"
#include "handlers/MifareClassicHandler.h"
#include "formats/Type2TagFormat.h"

NFCManager::NFCManager(uint8_t sda, uint8_t scl)
    : sda(sda), scl(scl), nfc(-1, -1)
{
}

bool NFCManager::start()
{
    if (running) return true;

    Serial.println("[NFC] Starting PN532...");

    Wire.begin(sda, scl);
    Wire.setClock(100000);

    nfc.begin();

    uint32_t version = nfc.getFirmwareVersion();

    if (!version)
    {
        Serial.println("[NFC] PN532 not found.");
        return false;
    }

    uint8_t chip = (version >> 24) & 0xFF;
    uint8_t firmwareMajor = (version >> 16) & 0xFF;
    uint8_t firmwareMinor = (version >> 8) & 0xFF;

    Serial.print("[NFC] PN5");
    Serial.println(chip, HEX);

    Serial.print("[NFC] Firmware ");
    Serial.print(firmwareMajor);
    Serial.print(".");
    Serial.println(firmwareMinor);

    if (!nfc.SAMConfig())
    {
        Serial.println("[NFC] SAMConfig failed.");
        return false;
    }

    running = true;

    Serial.println("[NFC] Ready.");

    return true;
}

bool NFCManager::resetReader()
{
    if (!running) return false;

    delay(10);

    if (!nfc.SAMConfig())
    {
        Serial.println("[NFC] Reader reset failed.");
        return false;
    }

    delay(5);

    return true;
}

bool NFCManager::scan(NFCTag& tag, uint16_t timeout)
{
    if (!running) return false;

    tag.clear();

    uint8_t uid[10] = {0};
    uint8_t uidLength = 0;

    bool found = nfc.readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        uid,
        &uidLength,
        timeout
    );

    if (!found) return false;
    if (uidLength == 0 || uidLength > sizeof(tag.uid)) return false;

    memcpy(tag.uid, uid, uidLength);
    tag.uidLength = uidLength;

    detectTagType(tag);

    return true;
}

void NFCManager::detectTagType(NFCTag& tag)
{
    tag.type = NFCTagType::UNKNOWN;
    tag.capacity = 0;
    tag.readable = false;
    tag.writable = false;
    tag.ndefCapable = false;

    if (tag.uidLength == 4 && detectClassic(tag))
    {
        tag.type = NFCTagType::MIFARE_CLASSIC;
        tag.capacity = 47 * 16;
        tag.readable = true;
        tag.writable = true;
        return;
    }

    if (detectNTAG())
    {
        tag.type = NFCTagType::NTAG_ULTRALIGHT;

        uint8_t cc[4] = {0};

        if (readPage(3, cc))
        {
            Type2TagInfo info;

            if (Type2TagFormat::parseCapabilityContainer(cc, info))
            {
                tag.capacity = info.dataAreaSize;
                tag.readable = info.readable;
                tag.writable = info.writable;
                tag.ndefCapable = true;
                return;
            }
        }

        tag.readable = true;
    }
}

bool NFCManager::detectNTAG()
{
    uint8_t page[4] = {0};

    return nfc.ntag2xx_ReadPage(4, page);
}

bool NFCManager::detectClassic(const NFCTag& tag)
{
    uint8_t key[6] =
    {
        0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF
    };

    return nfc.mifareclassic_AuthenticateBlock(
        const_cast<uint8_t*>(tag.uid),
        tag.uidLength,
        4,
        0,
        key
    );
}

NFCResult NFCManager::readTag(NFCTagData& data, uint16_t timeout)
{
    NFCTag tag;

    if (!scan(tag, timeout))
        return NFCResult::NO_TAG;

    NFCResult result = readDetectedTag(tag, data);

    resetReader();

    return result;
}

NFCResult NFCManager::readDetectedTag(const NFCTag& tag, NFCTagData& data)
{
    data.clear();
    data.tag = tag;

    if (tag.type == NFCTagType::NTAG_ULTRALIGHT)
    {
        Type2TagHandler handler(*this, tag);

        if (!handler.isReadable())
            return NFCResult::READ_FAILED;

        if (!handler.read(data.rawData))
            return NFCResult::READ_FAILED;

        Type2TagInfo info;
        std::vector<uint8_t> ndefBytes;

        if (!handler.getInfo(info))
            return NFCResult::INVALID_FORMAT;

        if (!Type2TagFormat::findNDEF(data.rawData, info, ndefBytes))
            return NFCResult::INVALID_FORMAT;

        data.hasNDEF = info.hasNDEF && !ndefBytes.empty();

        if (data.hasNDEF)
        {
            if (!NDEFManager::decode(ndefBytes.data(), ndefBytes.size(), data.ndef))
            {
                data.hasNDEF = false;
                data.ndef.clear();
            }
        }

        return NFCResult::OK;
    }

    if (tag.type == NFCTagType::MIFARE_CLASSIC)
    {
        MifareClassicHandler handler(*this, tag);

        if (!handler.read(data.rawData))
            return NFCResult::AUTH_FAILED;

        parseClassicData(data);

        return NFCResult::OK;
    }

    return NFCResult::UNSUPPORTED_TAG;
}

bool NFCManager::parseClassicData(NFCTagData& data)
{
    data.hasCherryRFData = false;
    data.cherryRFType = CherryRFDataType::NONE;
    data.cherryRFValue = "";

    /*
     * CherryRF Classic format:
     *
     * Byte 0-3 : "CRF1"
     * Byte 4   : type
     *            0x01 = Text
     *            0x02 = URI
     * Byte 5-6 : payload length, big endian
     * Byte 7   : reserved
     * Byte 8.. : payload
     */
    static constexpr size_t HEADER_SIZE = 8;

    if (data.rawData.size() < HEADER_SIZE)
        return false;

    if (data.rawData[0] != 'C' ||
        data.rawData[1] != 'R' ||
        data.rawData[2] != 'F' ||
        data.rawData[3] != '1')
    {
        return false;
    }

    uint8_t type = data.rawData[4];

    size_t length =
        ((size_t)data.rawData[5] << 8) |
        (size_t)data.rawData[6];

    if (length > data.rawData.size() - HEADER_SIZE)
        return false;

    switch (type)
    {
        case 0x01:
            data.cherryRFType = CherryRFDataType::TEXT;
            break;

        case 0x02:
            data.cherryRFType = CherryRFDataType::URI;
            break;

        default:
            return false;
    }

    String value;
    value.reserve(length);

    for (size_t i = 0; i < length; i++)
        value += (char)data.rawData[HEADER_SIZE + i];

    data.cherryRFValue = value;
    data.hasCherryRFData = true;

    return true;
}

NFCResult NFCManager::writeText(const String& text, uint16_t timeout)
{
    NFCTag tag;

    if (!scan(tag, timeout))
        return NFCResult::NO_TAG;

    NFCResult result = writeText(tag, text);

    resetReader();

    return result;
}

NFCResult NFCManager::writeText(const NFCTag& tag, const String& text)
{
    if (!tag.writable)
        return NFCResult::READ_ONLY;

    if (tag.type == NFCTagType::NTAG_ULTRALIGHT)
    {
        std::vector<uint8_t> ndef;

        if (!NDEFManager::createText(text, ndef))
            return NFCResult::INVALID_FORMAT;

        return writeNDEFToType2(tag, ndef);
    }

    if (tag.type == NFCTagType::MIFARE_CLASSIC)
        return writeClassicData(tag, CherryRFDataType::TEXT, text);

    return NFCResult::INCOMPATIBLE_TAG;
}

NFCResult NFCManager::writeURI(const String& uri, uint16_t timeout)
{
    NFCTag tag;

    if (!scan(tag, timeout))
        return NFCResult::NO_TAG;

    NFCResult result = writeURI(tag, uri);

    resetReader();

    return result;
}

NFCResult NFCManager::writeURI(const NFCTag& tag, const String& uri)
{
    if (!tag.writable)
        return NFCResult::READ_ONLY;

    if (tag.type == NFCTagType::NTAG_ULTRALIGHT)
    {
        std::vector<uint8_t> ndef;

        if (!NDEFManager::createURI(uri, ndef))
            return NFCResult::INVALID_FORMAT;

        return writeNDEFToType2(tag, ndef);
    }

    if (tag.type == NFCTagType::MIFARE_CLASSIC)
        return writeClassicData(tag, CherryRFDataType::URI, uri);

    return NFCResult::INCOMPATIBLE_TAG;
}

NFCResult NFCManager::writeClassicData(const NFCTag& tag, CherryRFDataType type, const String& value)
{
    MifareClassicHandler handler(*this, tag);

    if (!handler.isWritable())
        return NFCResult::READ_ONLY;

    static constexpr size_t HEADER_SIZE = 8;

    size_t valueLength = value.length();

    if (valueLength > handler.capacity() - HEADER_SIZE)
        return NFCResult::NOT_ENOUGH_SPACE;

    uint8_t typeByte = 0x00;

    switch (type)
    {
        case CherryRFDataType::TEXT:
            typeByte = 0x01;
            break;

        case CherryRFDataType::URI:
            typeByte = 0x02;
            break;

        default:
            return NFCResult::INVALID_FORMAT;
    }

    std::vector<uint8_t> output;
    output.reserve(HEADER_SIZE + valueLength);

    output.push_back('C');
    output.push_back('R');
    output.push_back('F');
    output.push_back('1');

    output.push_back(typeByte);

    output.push_back((valueLength >> 8) & 0xFF);
    output.push_back(valueLength & 0xFF);

    output.push_back(0x00);

    for (size_t i = 0; i < valueLength; i++)
        output.push_back((uint8_t)value[i]);

    if (!handler.write(output))
        return NFCResult::WRITE_FAILED;

    if (!verifyClassicData(tag, output))
        return NFCResult::VERIFY_FAILED;

    return NFCResult::OK;
}

bool NFCManager::verifyClassicData(const NFCTag& tag, const std::vector<uint8_t>& expected)
{
    MifareClassicHandler handler(*this, tag);

    std::vector<uint8_t> actual;

    if (!handler.read(actual))
        return false;

    if (actual.size() < expected.size())
        return false;

    for (size_t i = 0; i < expected.size(); i++)
    {
        if (actual[i] != expected[i])
            return false;
    }

    return true;
}

NFCResult NFCManager::writeNDEFToType2(const NFCTag& tag, const std::vector<uint8_t>& ndef)
{
    Type2TagHandler handler(*this, tag);

    if (!handler.isWritable())
        return NFCResult::READ_ONLY;

    std::vector<uint8_t> current;

    if (!handler.read(current))
        return NFCResult::READ_FAILED;

    std::vector<uint8_t> output;

    if (!Type2TagFormat::buildNDEFDataArea(current, ndef, output))
        return NFCResult::NOT_ENOUGH_SPACE;

    if (!handler.write(output))
        return NFCResult::WRITE_FAILED;

    return verifyType2NDEF(tag, ndef);
}

NFCResult NFCManager::verifyType2NDEF(const NFCTag& tag, const std::vector<uint8_t>& expected)
{
    if (tag.type != NFCTagType::NTAG_ULTRALIGHT)
        return NFCResult::VERIFY_FAILED;

    Type2TagHandler handler(*this, tag);

    std::vector<uint8_t> actual;

    if (!handler.read(actual))
        return NFCResult::VERIFY_FAILED;

    Type2TagInfo info;
    std::vector<uint8_t> ndef;

    if (!handler.getInfo(info))
        return NFCResult::VERIFY_FAILED;

    if (!Type2TagFormat::findNDEF(actual, info, ndef))
        return NFCResult::VERIFY_FAILED;

    if (ndef != expected)
        return NFCResult::VERIFY_FAILED;

    return NFCResult::OK;
}

NFCResult NFCManager::writeTagData(const NFCTagData& source, uint16_t timeout)
{
    NFCTag target;

    if (!scan(target, timeout))
        return NFCResult::NO_TAG;

    NFCResult result = writeTagData(target, source);

    resetReader();

    return result;
}

NFCResult NFCManager::writeTagData(const NFCTag& target, const NFCTagData& source)
{
    if (source.tag.type != target.type)
        return NFCResult::INCOMPATIBLE_TAG;

    if (target.type == NFCTagType::NTAG_ULTRALIGHT)
    {
        if (!source.hasNDEF)
            return NFCResult::INVALID_FORMAT;

        if (source.ndef.empty())
            return NFCResult::INVALID_FORMAT;

        std::vector<uint8_t> ndef;

        const NDEFRecord& record = source.ndef.records[0];

        if (record.type == NDEFRecordType::TEXT)
        {
            if (!NDEFManager::createText(record.value, ndef, record.language))
                return NFCResult::INVALID_FORMAT;
        }
        else if (record.type == NDEFRecordType::URI)
        {
            if (!NDEFManager::createURI(record.value, ndef))
                return NFCResult::INVALID_FORMAT;
        }
        else
        {
            return NFCResult::UNSUPPORTED_TAG;
        }

        return writeNDEFToType2(target, ndef);
    }

    if (target.type == NFCTagType::MIFARE_CLASSIC)
    {
        MifareClassicHandler handler(*this, target);

        if (source.rawData.size() > handler.capacity())
            return NFCResult::NOT_ENOUGH_SPACE;

        if (!handler.write(source.rawData))
            return NFCResult::WRITE_FAILED;

        NFCTagData verify;

        NFCResult result = readDetectedTag(target, verify);

        if (result != NFCResult::OK)
            return NFCResult::VERIFY_FAILED;

        if (verify.rawData != source.rawData)
            return NFCResult::VERIFY_FAILED;

        return NFCResult::OK;
    }

    return NFCResult::UNSUPPORTED_TAG;
}

bool NFCManager::readPage(uint8_t page, uint8_t* data)
{
    if (!running || !data) return false;

    return nfc.ntag2xx_ReadPage(page, data);
}

bool NFCManager::writePage(uint8_t page, const uint8_t* data)
{
    if (!running || !data) return false;

    return nfc.ntag2xx_WritePage(
        page,
        const_cast<uint8_t*>(data)
    );
}

bool NFCManager::authenticateClassic(const NFCTag& tag, uint8_t block, const uint8_t* key, bool keyB)
{
    if (!running || !key) return false;
    if (!tag.valid()) return false;
    if (tag.type != NFCTagType::MIFARE_CLASSIC) return false;

    return nfc.mifareclassic_AuthenticateBlock(
        const_cast<uint8_t*>(tag.uid),
        tag.uidLength,
        block,
        keyB ? 1 : 0,
        const_cast<uint8_t*>(key)
    );
}

bool NFCManager::readClassicBlock(uint8_t block, uint8_t* data)
{
    if (!running || !data) return false;

    return nfc.mifareclassic_ReadDataBlock(block, data);
}

bool NFCManager::writeClassicBlock(uint8_t block, const uint8_t* data)
{
    if (!running || !data) return false;

    return nfc.mifareclassic_WriteDataBlock(
        block,
        const_cast<uint8_t*>(data)
    );
}

bool NFCManager::isRunning() const
{
    return running;
}