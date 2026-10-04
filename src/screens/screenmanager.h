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
    explicit ScreenManager(AppContext& context);
    ~ScreenManager();

    void begin();
    void update();

    void show(ScreenID id);
    void home();

private:
    void destroyCurrent();
    Screen* create(ScreenID id);

    AppContext& context;

    Screen* current = nullptr;
    ScreenID currentID = ScreenID::HOME;
};