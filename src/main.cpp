#include <Arduino.h>
#include <vector>

#include "nfc/NFCManager.h"
#include "nfc/NFCTag.h"

#include "nfc/handlers/NTAGHandler.h"
#include "nfc/handlers/MifareClassicHandler.h"

static constexpr uint8_t NFC_SDA = 8;
static constexpr uint8_t NFC_SCL = 9;

NFCManager nfc(NFC_SDA, NFC_SCL);

String lastUID = "";

void printHex(const std::vector<uint8_t>& data)
{
    for (size_t i = 0; i < data.size(); i++)
    {
        if (data[i] < 0x10)
            Serial.print("0");

        Serial.print(data[i], HEX);
        Serial.print(" ");

        if ((i + 1) % 16 == 0)
            Serial.println();
    }

    if (data.size() % 16 != 0)
        Serial.println();
}

void testNTAG(const NFCTag& tag)
{
    Serial.println();
    Serial.println("=== NTAG / ULTRALIGHT HANDLER ===");

    NTAGHandler handler(nfc, tag);

    Serial.print("Readable: ");
    Serial.println(handler.isReadable() ? "YES" : "NO");

    Serial.print("Writable: ");
    Serial.println(handler.isWritable() ? "YES" : "NO");

    Serial.print("Detected capacity: ");
    Serial.print(handler.capacity());
    Serial.println(" bytes");

    std::vector<uint8_t> data;

    Serial.println();
    Serial.println("Reading user memory...");

    if (!handler.read(data))
    {
        Serial.println("READ FAILED");
        return;
    }

    Serial.print("Read ");
    Serial.print(data.size());
    Serial.println(" bytes:");

    printHex(data);
}

void testClassic(const NFCTag& tag)
{
    Serial.println();
    Serial.println("=== MIFARE CLASSIC HANDLER ===");

    MifareClassicHandler handler(nfc, tag);

    Serial.print("Readable: ");
    Serial.println(handler.isReadable() ? "YES" : "NO");

    Serial.print("Writable: ");
    Serial.println(handler.isWritable() ? "YES" : "NO");

    Serial.print("User capacity: ");
    Serial.print(handler.capacity());
    Serial.println(" bytes");

    std::vector<uint8_t> data;

    Serial.println();
    Serial.println("Reading user memory...");

    if (!handler.read(data))
    {
        Serial.println("READ FAILED");
        return;
    }

    Serial.print("Read ");
    Serial.print(data.size());
    Serial.println(" bytes:");

    printHex(data);
}

void testTag(const NFCTag& tag)
{
    Serial.println();
    Serial.println("============================");
    Serial.println("         TAG FOUND");
    Serial.println("============================");

    Serial.print("UID: ");
    Serial.println(tag.getUID());

    Serial.print("Type: ");
    Serial.println(tag.getTypeName());

    Serial.print("Readable: ");
    Serial.println(tag.readable ? "YES" : "NO");

    Serial.print("Writable: ");
    Serial.println(tag.writable ? "YES" : "NO");

    Serial.print("NDEF: ");
    Serial.println(tag.ndefCapable ? "YES" : "NO");

    switch (tag.type)
    {
        case NFCTagType::NTAG_ULTRALIGHT:
            testNTAG(tag);
            break;

        case NFCTagType::MIFARE_CLASSIC:
            testClassic(tag);
            break;

        default:
            Serial.println();
            Serial.println("No handler available.");
            break;
    }

    Serial.println();
    Serial.println("============================");
}

void setup()
{
    Serial.begin(115200);

    delay(2000);

    Serial.println();
    Serial.println("============================");
    Serial.println("       NFC LAB TEST");
    Serial.println("============================");

    if (!nfc.start())
    {
        Serial.println("PN532 START FAILED");

        while (true)
            delay(1000);
    }

    Serial.println("PN532 READY");
    Serial.println("Present a tag...");
}

void loop()
{
    NFCTag tag;

    if (!nfc.scan(tag, 100))
    {
        lastUID = "";
        delay(50);
        return;
    }

    String uid = tag.getUID();

    if (uid == lastUID)
    {
        delay(100);
        return;
    }

    lastUID = uid;

    testTag(tag);

    delay(250);
}