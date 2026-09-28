#include "NFCManager.h"

NFCManager::NFCManager(uint8_t sda, uint8_t scl)
    : sda(sda), scl(scl), nfc(-1, -1)
{
}

bool NFCManager::start()
{
    if (running) return true;

    Wire.begin(sda, scl);

    nfc.begin();

    uint32_t version = nfc.getFirmwareVersion();

    if (!version)
    {
        Serial.println("[NFCManager] PN532 not found.");
        return false;
    }

    uint8_t chip = (version >> 24) & 0xFF;
    uint8_t firmwareMajor = (version >> 16) & 0xFF;
    uint8_t firmwareMinor = (version >> 8) & 0xFF;

    Serial.print("[NFCManager] PN5");
    Serial.println(chip, HEX);

    Serial.print("[NFCManager] Firmware ");
    Serial.print(firmwareMajor);
    Serial.print(".");
    Serial.println(firmwareMinor);

    if (!nfc.SAMConfig())
    {
        Serial.println("[NFCManager] SAMConfig failed.");
        return false;
    }

    running = true;

    Serial.println("[NFCManager] Started.");

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

    if (uidLength == 0 || uidLength > sizeof(tag.uid))
    {
        Serial.println("[NFCManager] Invalid UID length.");
        return false;
    }

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
        tag.readable = true;
        tag.writable = true;
        tag.ndefCapable = false;

        Serial.println("[NFCManager] MIFARE Classic detected.");

        return;
    }

    if (detectNTAG())
    {
        tag.type = NFCTagType::NTAG_ULTRALIGHT;
        tag.readable = true;
        tag.writable = true;
        tag.ndefCapable = false;

        Serial.println("[NFCManager] NTAG / Ultralight detected.");

        return;
    }

    Serial.println("[NFCManager] Unknown ISO14443A tag.");
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

bool NFCManager::authenticateClassic(
    const NFCTag& tag,
    uint8_t block,
    const uint8_t* key,
    bool keyB)
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