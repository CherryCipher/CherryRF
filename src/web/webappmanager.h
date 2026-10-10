#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include <logger/Logger.h>
#include <wifi/WiFiManager.h>
#include <wifi/WiFiConfig.h>

#include "../nfc/NFCTagData.h"

class NFCManager;

class WebAppManager
{
public:
    explicit WebAppManager(NFCManager& nfc);

    bool start();
    void stop();
    void update();

    bool isRunning() const;

    String getSSID() const;
    String getIP() const;

private:
    void registerRoutes();

    void handleIndex();
    void handleStatus();
    void handleRead();
    void handleWrite();
    void handleCopyRead();
    void handleCopyWrite();
    void handleNotFound();

    void sendError(const String& message, int code = 400);
    void sendTagData(const NFCTagData& data);

    String escapeJSON(const String& value) const;
    String bytesToHex(const std::vector<uint8_t>& data) const;
    String describeTagData(const NFCTagData& data) const;

private:
    NFCManager& nfc;

    Logger logger;
    WiFiManager wifi;
    WebServer server{80};

    NFCTagData copySource;
    bool copyReady = false;

    bool running = false;
};