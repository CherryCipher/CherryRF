#pragma once

#include <Arduino.h>

enum class Button
{
    NONE,
    UP,
    DOWN,
    SELECT,
    BACK
};

class ButtonManager
{
public:
    ButtonManager(uint8_t up, uint8_t down, uint8_t select, uint8_t back);

    void begin();
    void update();

    Button getPressed();

private:
    struct ButtonState
    {
        uint8_t pin;
        bool stableState = HIGH;
        bool lastReading = HIGH;
        unsigned long changedAt = 0;
        bool pressedEvent = false;
    };

    void updateButton(ButtonState& button);

    ButtonState up;
    ButtonState down;
    ButtonState select;
    ButtonState back;

    static constexpr unsigned long DEBOUNCE_MS = 30;
};