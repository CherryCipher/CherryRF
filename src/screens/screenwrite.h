#pragma once

#include "Screen.h"
#include "../core/AppContext.h"

class ScreenWrite : public Screen
{
public:
    explicit ScreenWrite(AppContext& context);

    void enter() override;
    void update() override;
    void handleButton(Button button) override;
    void render() override;

private:
    enum class State
    {
        WAITING,
        WRITING,
        SUCCESS,
        ERROR
    };

    String generateCode();

    AppContext& context;

    State state = State::WAITING;

    String code;
    String error;

    unsigned long lastAttempt = 0;
};