#pragma once

#include "../input/ButtonManager.h"

class Screen
{
public:
    virtual ~Screen() = default;

    virtual void enter() = 0;
    virtual void update() = 0;
    virtual void handleButton(Button button) = 0;
    virtual void render() = 0;
};