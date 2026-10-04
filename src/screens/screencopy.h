#pragma once

#include "Screen.h"
#include "../core/AppContext.h"
#include "../nfc/NFCTagData.h"

class ScreenCopy : public Screen
{
public:
    explicit ScreenCopy(AppContext& context);

    void enter() override;
    void update() override;
    void handleButton(Button button) override;
    void render() override;

private:
    enum class State
    {
        WAIT_SOURCE,
        READING_SOURCE,
        WAIT_SOURCE_REMOVAL,
        WAIT_TARGET,
        WRITING_TARGET,
        SUCCESS,
        ERROR
    };

    AppContext& context;

    State state = State::WAIT_SOURCE;

    NFCTagData source;

    String sourceUID;
    String error;

    unsigned long lastScan = 0;
};