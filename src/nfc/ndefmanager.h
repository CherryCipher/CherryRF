#pragma once

#include <Arduino.h>
#include <vector>

/**
 * @enum NDEFRecordType
 * @brief Supported high-level NDEF record types.
 */
enum class NDEFRecordType
{
    UNKNOWN,
    TEXT,
    URI
};

/**
 * @struct NDEFRecord
 * @brief Represents a decoded NDEF record.
 */
struct NDEFRecord
{
    NDEFRecordType type = NDEFRecordType::UNKNOWN;

    String value;
    String language;

    uint8_t tnf = 0;
    String rawType;

    std::vector<uint8_t> payload;
};

/**
 * @struct NDEFMessage
 * @brief Represents a complete NDEF message.
 */
struct NDEFMessage
{
    std::vector<NDEFRecord> records;

    void clear()
    {
        records.clear();
    }

    bool empty() const
    {
        return records.empty();
    }

    size_t size() const
    {
        return records.size();
    }
};

/**
 * @class NDEFManager
 * @brief Encodes and decodes NFC Data Exchange Format messages.
 *
 * NDEFManager contains no PN532 or tag-specific code.
 *
 * It converts raw NDEF byte streams into structured NDEF messages
 * and creates valid NDEF byte streams from application data.
 */
class NDEFManager
{
public:

    /**
     * @brief Decodes a complete NDEF message.
     *
     * @param data Raw NDEF message.
     * @param length Number of bytes.
     * @param message Destination message.
     *
     * @return true if a valid NDEF message was decoded.
     */
    static bool decode(const uint8_t* data, size_t length, NDEFMessage& message);

    /**
     * @brief Creates an NDEF Text record.
     *
     * @param text Text payload.
     * @param output Destination byte vector.
     * @param language ISO language code.
     *
     * @return true on success.
     */
    static bool createText(
        const String& text,
        std::vector<uint8_t>& output,
        const String& language = "en"
    );

    /**
     * @brief Creates an NDEF URI record.
     *
     * Common URI prefixes are compressed automatically.
     *
     * @param uri URI to encode.
     * @param output Destination byte vector.
     *
     * @return true on success.
     */
    static bool createURI(
        const String& uri,
        std::vector<uint8_t>& output
    );

    /**
     * @brief Returns a human-readable record type.
     */
    static String getTypeName(NDEFRecordType type);

private:

    static bool decodeRecord(
        const uint8_t* data,
        size_t length,
        size_t& offset,
        NDEFRecord& record,
        bool& messageEnd
    );

    static void decodeText(NDEFRecord& record);
    static void decodeURI(NDEFRecord& record);

    static uint8_t getURIPrefix(const String& uri, String& remainder);
    static String getURIPrefixString(uint8_t prefix);
};