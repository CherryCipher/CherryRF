#include "WebAppManager.h"

#include "../Config.h"

WebAppManager::WebAppManager()
    : wifi(logger)
{
}

bool WebAppManager::start()
{
    if (running) return true;

    if (!wifi.start()) return false;

    WiFiAPConfig config;

    config.ssid = Config::WEBAPP_SSID;
    config.password = Config::WEBAPP_PASSWORD;

    if (!wifi.startAP(config))
    {
        wifi.stop();
        return false;
    }

    if (!web.start())
    {
        wifi.stopAP();
        wifi.stop();
        return false;
    }

    running = true;

    return true;
}

void WebAppManager::stop()
{
    if (!running) return;

    web.stop();
    wifi.stopAP();
    wifi.stop();

    running = false;
}

void WebAppManager::update()
{
    if (!running) return;

    web.handleClients();
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