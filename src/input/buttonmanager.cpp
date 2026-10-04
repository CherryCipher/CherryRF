#include "ButtonManager.h"

ButtonManager::ButtonManager(uint8_t upPin, uint8_t downPin, uint8_t selectPin, uint8_t backPin)
    : up{upPin}, down{downPin}, select{selectPin}, back{backPin}
{
}

void ButtonManager::begin()
{
    pinMode(up.pin, INPUT_PULLUP);
    pinMode(down.pin, INPUT_PULLUP);
    pinMode(select.pin, INPUT_PULLUP);
    pinMode(back.pin, INPUT_PULLUP);

    up.stableState = up.lastReading = digitalRead(up.pin);
    down.stableState = down.lastReading = digitalRead(down.pin);
    select.stableState = select.lastReading = digitalRead(select.pin);
    back.stableState = back.lastReading = digitalRead(back.pin);
}

void ButtonManager::updateButton(ButtonState& button)
{
    bool reading = digitalRead(button.pin);

    if (reading != button.lastReading)
    {
        button.lastReading = reading;
        button.changedAt = millis();
    }

    if (millis() - button.changedAt < DEBOUNCE_MS) return;
    if (reading == button.stableState) return;

    button.stableState = reading;

    if (button.stableState == LOW) button.pressedEvent = true;
}

void ButtonManager::update()
{
    updateButton(up);
    updateButton(down);
    updateButton(select);
    updateButton(back);
}

Button ButtonManager::getPressed()
{
    if (up.pressedEvent)
    {
        up.pressedEvent = false;
        return Button::UP;
    }

    if (down.pressedEvent)
    {
        down.pressedEvent = false;
        return Button::DOWN;
    }

    if (select.pressedEvent)
    {
        select.pressedEvent = false;
        return Button::SELECT;
    }

    if (back.pressedEvent)
    {
        back.pressedEvent = false;
        return Button::BACK;
    }

    return Button::NONE;
}