#pragma once

#include <vector>

#include "Screen.h"
#include "../core/AppContext.h"
#include "../nfc/NFCTagData.h"

class ScreenRead : public Screen
{
public:
    explicit ScreenRead(AppContext& context);

    void enter() override;
    void update() override;
    void handleButton(Button button) override;
    void render() override;

private:
    enum class State
    {
        WAITING,
        READING,
        RESULT,
        ERROR
    };

    void buildLines();

    AppContext& context;

    State state = State::WAITING;

    NFCTagData data;

    std::vector<String> lines;

    size_t scroll = 0;

    String error;

    unsigned long lastScan = 0;
};