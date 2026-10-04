#pragma once

#include <Arduino.h>

#include <logger/Logger.h>
#include <wifi/WiFiManager.h>
#include <wifi/WiFiConfig.h>
#include <webserver/WebServerManager.h>

class WebAppManager
{
public:
    WebAppManager();

    bool start();
    void stop();
    void update();

    bool isRunning() const;

    String getSSID() const;
    String getIP() const;

private:
    Logger logger;
    WiFiManager wifi;
    WebServerManager web;

    bool running = false;
};