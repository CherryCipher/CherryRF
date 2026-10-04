#pragma once

#include "Screen.h"
#include "../core/AppContext.h"

class ScreenWebApp : public Screen
{
public:
    explicit ScreenWebApp(AppContext& context);

    void enter() override;
    void update() override;
    void handleButton(Button button) override;
    void render() override;

private:
    enum class State
    {
        STARTING,
        RUNNING,
        ERROR
    };

    AppContext& context;

    State state = State::STARTING;
};