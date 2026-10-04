#include "ScreenWrite.h"

#include "../display/DisplayManager.h"
#include "../nfc/NFCManager.h"
#include "../nfc/NFCResult.h"
#include "ScreenManager.h"

ScreenWrite::ScreenWrite(AppContext& context)
    : context(context)
{
}

String ScreenWrite::generateCode()
{
    uint32_t value = esp_random() & 0xFFFFFF;

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "CRF-%06lX", (unsigned long)value);

    return String(buffer);
}

void ScreenWrite::enter()
{
    code = generateCode();
    error = "";
    state = State::WAITING;
    lastAttempt = 0;
}

void ScreenWrite::update()
{
    if (state != State::WAITING) return;
    if (millis() - lastAttempt < 250) return;

    lastAttempt = millis();

    NFCTag tag;

    if (!context.nfc->scan(tag, 50)) return;

    state = State::WRITING;
    render();

    NFCResult result = context.nfc->writeText(code, 250);

    if (result == NFCResult::OK)
    {
        state = State::SUCCESS;
    }
    else
    {
        state = State::ERROR;
        error = getNFCResultName(result);
    }

    render();
}

void ScreenWrite::handleButton(Button button)
{
    if (button == Button::BACK)
        context.screens->show(ScreenID::MENU);
}

void ScreenWrite::render()
{
    DisplayManager& display = *context.display;

    display.clear();
    display.header("WRITE");

    if (state == State::WAITING)
    {
        display.centered(15, code);
        display.centered(32, "PRESENT TAG");
    }
    else if (state == State::WRITING)
    {
        display.centered(27, "WRITING...");
    }
    else if (state == State::SUCCESS)
    {
        display.centered(16, "WRITE OK");
        display.centered(32, code);
    }
    else
    {
        display.centered(18, "WRITE FAILED");
        display.centered(34, error);
    }

    display.footer("< BACK");
    display.show();
}