#pragma once

#include "Screen.h"
#include "../core/AppContext.h"

class ScreenMenu : public Screen
{
public:
    explicit ScreenMenu(AppContext& context);

    void enter() override;
    void update() override;
    void handleButton(Button button) override;
    void render() override;

private:
    AppContext& context;

    uint8_t selected = 0;

    static constexpr uint8_t ITEM_COUNT = 4;

    const char* items[ITEM_COUNT] =
    {
        "READ",
        "WRITE",
        "COPY",
        "WEBAPP"
    };
};