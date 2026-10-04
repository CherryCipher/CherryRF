#pragma once

class NFCManager;
class DisplayManager;
class ButtonManager;
class ScreenManager;
class WebAppManager;

struct AppContext
{
    NFCManager* nfc = nullptr;
    DisplayManager* display = nullptr;
    ButtonManager* buttons = nullptr;
    ScreenManager* screens = nullptr;
    WebAppManager* webApp = nullptr;
};