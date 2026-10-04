#pragma once

#include "Screen.h"
#include "../core/AppContext.h"
#include "../nfc/NFCTag.h"

class ScreenHome : public Screen
{
public:
    explicit ScreenHome(AppContext& context);

    void enter() override;
    void update() override;
    void handleButton(Button button) override;
    void render() override;

private:
    AppContext& context;

    NFCTag tag;

    bool tagPresent = false;
    String currentUID;

    unsigned long lastScan = 0;

    static constexpr unsigned long SCAN_INTERVAL = 200;
};