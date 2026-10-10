#include "WebAppManager.h"

#include "../Config.h"
#include "../nfc/NFCManager.h"
#include "../nfc/NFCResult.h"
#include "../nfc/NDEFManager.h"
#include "WebPages.h"

WebAppManager::WebAppManager(NFCManager& nfc)
    : nfc(nfc), wifi(logger)
{
}

bool WebAppManager::start()
{
    if (running) return true;

    Serial.println("[WebApp] Starting...");

    if (!wifi.start())
    {
        Serial.println("[WebApp] WiFi start failed.");
        return false;
    }

    WiFiAPConfig config;
    config.ssid = Config::WEBAPP_SSID;
    config.password = Config::WEBAPP_PASSWORD;

    if (!wifi.startAP(config))
    {
        Serial.println("[WebApp] AP start failed.");
        wifi.stop();
        return false;
    }

    registerRoutes();

    server.begin();

    copySource.clear();
    copyReady = false;
    running = true;

    Serial.println("[WebApp] Ready.");
    Serial.print("[WebApp] SSID: ");
    Serial.println(getSSID());
    Serial.print("[WebApp] IP: ");
    Serial.println(getIP());

    return true;
}

void WebAppManager::stop()
{
    if (!running) return;

    server.stop();

    wifi.stopAP();
    wifi.stop();

    copySource.clear();
    copyReady = false;
    running = false;

    Serial.println("[WebApp] Stopped.");
}

void WebAppManager::update()
{
    if (!running) return;

    server.handleClient();
}

bool WebAppManager::isRunning() const
{
    return running;
}

String WebAppManager::getSSID() const
{
    return Config::WEBAPP_SSID;
}

String WebAppManager::getIP() const
{
    return wifi.getAPIP().toString();
}

void WebAppManager::registerRoutes()
{
    server.on("/", HTTP_GET, [this]()
    {
        handleIndex();
    });

    server.on("/api/status", HTTP_GET, [this]()
    {
        handleStatus();
    });

    server.on("/api/read", HTTP_GET, [this]()
    {
        handleRead();
    });

    server.on("/api/write", HTTP_POST, [this]()
    {
        handleWrite();
    });

    server.on("/api/copy/read", HTTP_POST, [this]()
    {
        handleCopyRead();
    });

    server.on("/api/copy/write", HTTP_POST, [this]()
    {
        handleCopyWrite();
    });

    server.onNotFound([this]()
    {
        handleNotFound();
    });
}

void WebAppManager::handleIndex()
{
    server.send(200, "text/html", WebPages::INDEX);
}

void WebAppManager::handleStatus()
{
    server.send(
        200,
        "application/json",
        R"({"status":"ready","device":"CherryRF"})"
    );
}

void WebAppManager::handleRead()
{
    NFCTag tag;

    if (!nfc.scan(tag, 750))
    {
        sendError("NO TAG", 404);
        return;
    }

    NFCTagData data;
    NFCResult result = nfc.readDetectedTag(tag, data);

    nfc.resetReader();

    if (result != NFCResult::OK)
    {
        sendError(getNFCResultName(result));
        return;
    }

    sendTagData(data);
}

void WebAppManager::handleWrite()
{
    if (!server.hasArg("type") || !server.hasArg("value"))
    {
        sendError("MISSING DATA");
        return;
    }

    String type = server.arg("type");
    String value = server.arg("value");

    value.trim();

    if (value.length() == 0)
    {
        sendError("EMPTY VALUE");
        return;
    }

    NFCTag tag;

    if (!nfc.scan(tag, 750))
    {
        sendError("NO TAG", 404);
        return;
    }

    NFCResult result;

    if (type == "text")
        result = nfc.writeText(tag, value);
    else if (type == "uri")
        result = nfc.writeURI(tag, value);
    else
        result = NFCResult::INVALID_FORMAT;

    nfc.resetReader();

    if (result != NFCResult::OK)
    {
        sendError(getNFCResultName(result));
        return;
    }

    String json = "{\"ok\":true,\"uid\":\"";
    json += escapeJSON(tag.getUID());
    json += "\"}";

    server.send(200, "application/json", json);
}

void WebAppManager::handleCopyRead()
{
    NFCTag tag;

    if (!nfc.scan(tag, 750))
    {
        sendError("NO TAG", 404);
        return;
    }

    copySource.clear();

    NFCResult result = nfc.readDetectedTag(tag, copySource);

    nfc.resetReader();

    if (result != NFCResult::OK)
    {
        copyReady = false;
        sendError(getNFCResultName(result));
        return;
    }

    copyReady = true;

    String json = "{";
    json += "\"ok\":true,";
    json += "\"uid\":\"" + escapeJSON(copySource.tag.getUID()) + "\",";
    json += "\"type\":\"" + escapeJSON(copySource.tag.getTypeName()) + "\",";
    json += "\"description\":\"" + escapeJSON(describeTagData(copySource)) + "\"";
    json += "}";

    server.send(200, "application/json", json);
}

void WebAppManager::handleCopyWrite()
{
    if (!copyReady)
    {
        sendError("NO SOURCE");
        return;
    }

    NFCTag target;

    if (!nfc.scan(target, 750))
    {
        sendError("NO TAG", 404);
        return;
    }

    if (target.getUID() == copySource.tag.getUID())
    {
        nfc.resetReader();
        sendError("REMOVE SOURCE TAG");
        return;
    }

    NFCResult result = nfc.writeTagData(target, copySource);

    nfc.resetReader();

    if (result != NFCResult::OK)
    {
        sendError(getNFCResultName(result));
        return;
    }

    String json = "{\"ok\":true,\"uid\":\"";
    json += escapeJSON(target.getUID());
    json += "\"}";

    copySource.clear();
    copyReady = false;

    server.send(200, "application/json", json);
}

void WebAppManager::handleNotFound()
{
    server.send(
        404,
        "application/json",
        R"({"ok":false,"error":"NOT FOUND"})"
    );
}

void WebAppManager::sendError(const String& message, int code)
{
    String json = "{\"ok\":false,\"error\":\"";
    json += escapeJSON(message);
    json += "\"}";

    server.send(code, "application/json", json);
}

void WebAppManager::sendTagData(const NFCTagData& data)
{
    String json = "{";

    json += "\"ok\":true,";
    json += "\"uid\":\"" + escapeJSON(data.tag.getUID()) + "\",";
    json += "\"type\":\"" + escapeJSON(data.tag.getTypeName()) + "\",";
    json += "\"capacity\":" + String(data.tag.capacity) + ",";

    json += "\"readable\":";
    json += data.tag.readable ? "true," : "false,";

    json += "\"writable\":";
    json += data.tag.writable ? "true," : "false,";

    json += "\"hasNDEF\":";
    json += data.hasNDEF ? "true," : "false,";

    json += "\"hasCherryRFData\":";
    json += data.hasCherryRFData ? "true," : "false,";

    json += "\"cherryRFType\":\"";
    json += escapeJSON(data.getCherryRFTypeName());
    json += "\",";

    json += "\"cherryRFValue\":\"";
    json += escapeJSON(data.cherryRFValue);
    json += "\",";

    json += "\"records\":[";

    for (size_t i = 0; i < data.ndef.records.size(); i++)
    {
        if (i > 0) json += ",";

        const NDEFRecord& record = data.ndef.records[i];

        json += "{";
        json += "\"type\":\"";
        json += escapeJSON(NDEFManager::getTypeName(record.type));
        json += "\",";
        json += "\"value\":\"";
        json += escapeJSON(record.value);
        json += "\"";
        json += "}";
    }

    json += "],";

    json += "\"raw\":\"";
    json += escapeJSON(bytesToHex(data.rawData));
    json += "\"";

    json += "}";

    server.send(200, "application/json", json);
}

String WebAppManager::escapeJSON(const String& value) const
{
    String output;
    output.reserve(value.length() + 8);

    for (size_t i = 0; i < value.length(); i++)
    {
        char c = value[i];

        switch (c)
        {
            case '"':  output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;

            default:
                if ((uint8_t)c >= 0x20) output += c;
                break;
        }
    }

    return output;
}

String WebAppManager::bytesToHex(const std::vector<uint8_t>& data) const
{
    if (data.empty()) return "";

    static const char hex[] = "0123456789ABCDEF";

    String output;
    output.reserve(data.size() * 3);

    for (size_t i = 0; i < data.size(); i++)
    {
        if (i > 0)
        {
            if (i % 16 == 0)
                output += "\n";
            else
                output += " ";
        }

        output += hex[(data[i] >> 4) & 0x0F];
        output += hex[data[i] & 0x0F];
    }

    return output;
}

String WebAppManager::describeTagData(const NFCTagData& data) const
{
    if (data.hasNDEF && !data.ndef.empty())
    {
        const NDEFRecord& record = data.ndef.records[0];

        return NDEFManager::getTypeName(record.type) + ": " + record.value;
    }

    if (data.hasCherryRFData)
        return data.getCherryRFTypeName() + ": " + data.cherryRFValue;

    return String(data.rawData.size()) + " raw bytes";
}