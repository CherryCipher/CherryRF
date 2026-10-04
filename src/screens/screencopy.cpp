#include "ScreenCopy.h"

#include "../display/DisplayManager.h"
#include "../nfc/NFCManager.h"
#include "../nfc/NFCResult.h"
#include "ScreenManager.h"

ScreenCopy::ScreenCopy(AppContext& context)
    : context(context)
{
}

void ScreenCopy::enter()
{
    state = State::WAIT_SOURCE;
    source.clear();
    sourceUID = "";
    error = "";
    lastScan = 0;
}

void ScreenCopy::update()
{
    if (state == State::SUCCESS || state == State::ERROR) return;
    if (millis() - lastScan < 200) return;

    lastScan = millis();

    if (state == State::WAIT_SOURCE)
    {
        NFCTag tag;

        if (!context.nfc->scan(tag, 50)) return;

        state = State::READING_SOURCE;
        render();

        NFCResult result = context.nfc->readTag(source, 250);

        if (result != NFCResult::OK)
        {
            state = State::ERROR;
            error = getNFCResultName(result);
            render();
            return;
        }

        sourceUID = source.tag.getUID();
        state = State::WAIT_SOURCE_REMOVAL;
        render();
        return;
    }

    if (state == State::WAIT_SOURCE_REMOVAL)
    {
        NFCTag tag;

        if (!context.nfc->scan(tag, 50))
        {
            state = State::WAIT_TARGET;
            render();
        }

        return;
    }

    if (state == State::WAIT_TARGET)
    {
        NFCTag target;

        if (!context.nfc->scan(target, 50)) return;

        if (target.getUID() == sourceUID) return;

        state = State::WRITING_TARGET;
        render();

        NFCResult result = context.nfc->writeTagData(source, 250);

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
}

void ScreenCopy::handleButton(Button button)
{
    if (button == Button::BACK)
        context.screens->show(ScreenID::MENU);
}

void ScreenCopy::render()
{
    DisplayManager& display = *context.display;

    display.clear();
    display.header("COPY");

    switch (state)
    {
        case State::WAIT_SOURCE:
            display.centered(25, "PRESENT SOURCE");
            break;

        case State::READING_SOURCE:
            display.centered(25, "READING...");
            break;

        case State::WAIT_SOURCE_REMOVAL:
            display.centered(18, "COPIED");
            display.centered(34, "REMOVE TAG");
            break;

        case State::WAIT_TARGET:
            display.centered(18, "PRESENT");
            display.centered(34, "NEW TAG");
            break;

        case State::WRITING_TARGET:
            display.centered(25, "WRITING...");
            break;

        case State::SUCCESS:
            display.centered(25, "COPY OK");
            break;

        case State::ERROR:
            display.centered(18, "COPY FAILED");
            display.centered(34, error);
            break;
    }

    display.footer("< CANCEL");
    display.show();
}