#pragma once

#include <Arduino.h>

#include "Screen.h"
#include "../core/AppContext.h"

enum class ScreenID
{
    HOME,
    MENU,
    READ,
    WRITE,
    COPY,
    WEBAPP
};

class ScreenManager
{
public:
    ScreenManager(AppContext& context);
    ~ScreenManager();

    void begin();
    void update();

    void show(ScreenID id);
    void home();

    ScreenID getCurrentID() const;

private:
    Screen* create(ScreenID id);

    void destroyCurrent();
    void applyPendingSwitch();

private:
    AppContext& context;

    Screen* current = nullptr;
    ScreenID currentID = ScreenID::HOME;

    bool switchPending = false;
    ScreenID pendingID = ScreenID::HOME;
};