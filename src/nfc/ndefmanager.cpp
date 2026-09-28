#include "NDEFManager.h"

bool NDEFManager::decode(const uint8_t* data, size_t length, NDEFMessage& message)
{
    message.clear();

    if (!data || length == 0) return false;

    size_t offset = 0;
    bool messageEnd = false;

    while (offset < length && !messageEnd)
    {
        NDEFRecord record;

        if (!decodeRecord(data, length, offset, record, messageEnd))
        {
            message.clear();
            return false;
        }

        message.records.push_back(record);
    }

    return !message.empty();
}

bool NDEFManager::decodeRecord(
    const uint8_t* data,
    size_t length,
    size_t& offset,
    NDEFRecord& record,
    bool& messageEnd)
{
    if (offset >= length) return false;

    uint8_t header = data[offset++];

    bool me = header & 0x40;
    bool cf = header & 0x20;
    bool sr = header & 0x10;
    bool il = header & 0x08;

    uint8_t tnf = header & 0x07;

    /*
     * Chunked records are deliberately rejected here.
     * They require record reassembly rather than being treated
     * as independent NDEF records.
     */
    if (cf) return false;

    if (offset >= length) return false;

    uint8_t typeLength = data[offset++];

    uint32_t payloadLength = 0;

    if (sr)
    {
        if (offset >= length) return false;

        payloadLength = data[offset++];
    }
    else
    {
        if (offset + 4 > length) return false;

        payloadLength =
            ((uint32_t)data[offset] << 24) |
            ((uint32_t)data[offset + 1] << 16) |
            ((uint32_t)data[offset + 2] << 8) |
            ((uint32_t)data[offset + 3]);

        offset += 4;
    }

    uint8_t idLength = 0;

    if (il)
    {
        if (offset >= length) return false;

        idLength = data[offset++];
    }

    if (offset + typeLength + idLength + payloadLength > length)
        return false;

    record.tnf = tnf;

    record.rawType = "";

    for (uint8_t i = 0; i < typeLength; i++)
        record.rawType += (char)data[offset + i];

    offset += typeLength;

    /*
     * ID is currently not exposed by our high-level model,
     * but must still be skipped correctly.
     */
    offset += idLength;

    record.payload.assign(
        data + offset,
        data + offset + payloadLength
    );

    offset += payloadLength;

    /*
     * TNF 0x01 = NFC Forum Well Known Type.
     */
    if (tnf == 0x01 && record.rawType == "T")
    {
        record.type = NDEFRecordType::TEXT;
        decodeText(record);
    }
    else if (tnf == 0x01 && record.rawType == "U")
    {
        record.type = NDEFRecordType::URI;
        decodeURI(record);
    }
    else
    {
        record.type = NDEFRecordType::UNKNOWN;
    }

    messageEnd = me;

    return true;
}

void NDEFManager::decodeText(NDEFRecord& record)
{
    if (record.payload.empty()) return;

    uint8_t status = record.payload[0];

    bool utf16 = status & 0x80;
    uint8_t languageLength = status & 0x3F;

    if (record.payload.size() < 1 + languageLength) return;

    record.language = "";

    for (uint8_t i = 0; i < languageLength; i++)
        record.language += (char)record.payload[1 + i];

    /*
     * Our high-level String representation currently exposes
     * UTF-8 text. UTF-16 records remain available through payload.
     */
    if (utf16) return;

    record.value = "";

    for (size_t i = 1 + languageLength; i < record.payload.size(); i++)
        record.value += (char)record.payload[i];
}

void NDEFManager::decodeURI(NDEFRecord& record)
{
    if (record.payload.empty()) return;

    record.value = getURIPrefixString(record.payload[0]);

    for (size_t i = 1; i < record.payload.size(); i++)
        record.value += (char)record.payload[i];
}

bool NDEFManager::createText(
    const String& text,
    std::vector<uint8_t>& output,
    const String& language)
{
    output.clear();

    if (language.length() > 63) return false;

    size_t payloadLength =
        1 +
        language.length() +
        text.length();

    /*
     * Use Short Record when payload fits in one byte.
     */
    bool shortRecord = payloadLength <= 255;

    uint8_t header = 0x80 | 0x40 | 0x01;

    if (shortRecord) header |= 0x10;

    output.push_back(header);

    // Type length
    output.push_back(1);

    if (shortRecord)
    {
        output.push_back((uint8_t)payloadLength);
    }
    else
    {
        output.push_back((payloadLength >> 24) & 0xFF);
        output.push_back((payloadLength >> 16) & 0xFF);
        output.push_back((payloadLength >> 8) & 0xFF);
        output.push_back(payloadLength & 0xFF);
    }

    // NFC Forum Text type
    output.push_back('T');

    // UTF-8 + language length
    output.push_back((uint8_t)language.length());

    for (size_t i = 0; i < language.length(); i++)
        output.push_back(language[i]);

    for (size_t i = 0; i < text.length(); i++)
        output.push_back(text[i]);

    return true;
}

bool NDEFManager::createURI(
    const String& uri,
    std::vector<uint8_t>& output)
{
    output.clear();

    String remainder;

    uint8_t prefix = getURIPrefix(uri, remainder);

    size_t payloadLength = 1 + remainder.length();

    bool shortRecord = payloadLength <= 255;

    uint8_t header = 0x80 | 0x40 | 0x01;

    if (shortRecord) header |= 0x10;

    output.push_back(header);

    // Type length
    output.push_back(1);

    if (shortRecord)
    {
        output.push_back((uint8_t)payloadLength);
    }
    else
    {
        output.push_back((payloadLength >> 24) & 0xFF);
        output.push_back((payloadLength >> 16) & 0xFF);
        output.push_back((payloadLength >> 8) & 0xFF);
        output.push_back(payloadLength & 0xFF);
    }

    // NFC Forum URI type
    output.push_back('U');

    // URI prefix compression byte
    output.push_back(prefix);

    for (size_t i = 0; i < remainder.length(); i++)
        output.push_back(remainder[i]);

    return true;
}

uint8_t NDEFManager::getURIPrefix(const String& uri, String& remainder)
{
    struct Prefix
    {
        uint8_t code;
        const char* value;
    };

    static const Prefix prefixes[] =
    {
        {0x01, "http://www."},
        {0x02, "https://www."},
        {0x03, "http://"},
        {0x04, "https://"},
        {0x05, "tel:"},
        {0x06, "mailto:"},
        {0x07, "ftp://anonymous:anonymous@"},
        {0x08, "ftp://ftp."},
        {0x09, "ftps://"},
        {0x0A, "sftp://"},
        {0x0B, "smb://"},
        {0x0C, "nfs://"},
        {0x0D, "ftp://"},
        {0x0E, "dav://"},
        {0x0F, "news:"},
        {0x10, "telnet://"},
        {0x11, "imap:"},
        {0x12, "rtsp://"},
        {0x13, "urn:"},
        {0x14, "pop:"},
        {0x15, "sip:"},
        {0x16, "sips:"},
        {0x17, "tftp:"},
        {0x18, "btspp://"},
        {0x19, "btl2cap://"},
        {0x1A, "btgoep://"},
        {0x1B, "tcpobex://"},
        {0x1C, "irdaobex://"},
        {0x1D, "file://"},
        {0x1E, "urn:epc:id:"},
        {0x1F, "urn:epc:tag:"},
        {0x20, "urn:epc:pat:"},
        {0x21, "urn:epc:raw:"},
        {0x22, "urn:epc:"},
        {0x23, "urn:nfc:"}
    };

    for (const Prefix& item : prefixes)
    {
        String prefix = item.value;

        if (uri.startsWith(prefix))
        {
            remainder = uri.substring(prefix.length());

            return item.code;
        }
    }

    remainder = uri;

    return 0x00;
}

String NDEFManager::getURIPrefixString(uint8_t prefix)
{
    switch (prefix)
    {
        case 0x01: return "http://www.";
        case 0x02: return "https://www.";
        case 0x03: return "http://";
        case 0x04: return "https://";
        case 0x05: return "tel:";
        case 0x06: return "mailto:";
        case 0x07: return "ftp://anonymous:anonymous@";
        case 0x08: return "ftp://ftp.";
        case 0x09: return "ftps://";
        case 0x0A: return "sftp://";
        case 0x0B: return "smb://";
        case 0x0C: return "nfs://";
        case 0x0D: return "ftp://";
        case 0x0E: return "dav://";
        case 0x0F: return "news:";
        case 0x10: return "telnet://";
        case 0x11: return "imap:";
        case 0x12: return "rtsp://";
        case 0x13: return "urn:";
        case 0x14: return "pop:";
        case 0x15: return "sip:";
        case 0x16: return "sips:";
        case 0x17: return "tftp:";
        case 0x18: return "btspp://";
        case 0x19: return "btl2cap://";
        case 0x1A: return "btgoep://";
        case 0x1B: return "tcpobex://";
        case 0x1C: return "irdaobex://";
        case 0x1D: return "file://";
        case 0x1E: return "urn:epc:id:";
        case 0x1F: return "urn:epc:tag:";
        case 0x20: return "urn:epc:pat:";
        case 0x21: return "urn:epc:raw:";
        case 0x22: return "urn:epc:";
        case 0x23: return "urn:nfc:";
        default:   return "";
    }
}

String NDEFManager::getTypeName(NDEFRecordType type)
{
    switch (type)
    {
        case NDEFRecordType::TEXT:
            return "Text";

        case NDEFRecordType::URI:
            return "URI";

        default:
            return "Unknown";
    }
}