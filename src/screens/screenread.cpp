#include "ScreenRead.h"

#include "../display/DisplayManager.h"
#include "../nfc/NFCManager.h"
#include "../nfc/NFCResult.h"
#include "ScreenManager.h"

ScreenRead::ScreenRead(AppContext& context)
    : context(context)
{
}

void ScreenRead::enter()
{
    state = State::WAITING;
    data.clear();
    lines.clear();
    scroll = 0;
    error = "";
    lastScan = 0;
}

void ScreenRead::update()
{
    if (state != State::WAITING) return;
    if (millis() - lastScan < 200) return;

    lastScan = millis();

    NFCTag tag;

    if (!context.nfc->scan(tag, 50)) return;

    state = State::READING;
    render();

    NFCResult result = context.nfc->readDetectedTag(tag, data);

    context.nfc->resetReader();

    if (result != NFCResult::OK)
    {
        state = State::ERROR;
        error = getNFCResultName(result);
        render();
        return;
    }

    buildLines();

    state = State::RESULT;
    render();
}

void ScreenRead::buildLines()
{
    lines.clear();

    lines.push_back("TYPE:");
    lines.push_back(data.tag.getTypeName());

    lines.push_back("UID:");
    lines.push_back(data.tag.getUID());

    lines.push_back("SIZE:");
    lines.push_back(String(data.rawData.size()) + " BYTES");

    if (data.hasNDEF)
    {
        for (size_t i = 0; i < data.ndef.records.size(); i++)
        {
            const NDEFRecord& record = data.ndef.records[i];

            lines.push_back("NDEF " + String(i + 1) + ":");
            lines.push_back(NDEFManager::getTypeName(record.type));

            if (record.value.length())
                lines.push_back(record.value);
        }
    }
    else if (data.hasCherryRFData)
    {
        lines.push_back("CHERRYRF:");
        lines.push_back(data.getCherryRFTypeName());

        if (data.cherryRFValue.length())
            lines.push_back(data.cherryRFValue);
    }
    else
    {
        lines.push_back("DATA:");
        lines.push_back("NONE");
    }
}
void ScreenRead::handleButton(Button button)
{
    if (button == Button::BACK)
    {
        context.screens->show(ScreenID::MENU);
        return;
    }

    if (state != State::RESULT) return;

    if (button == Button::UP && scroll > 0)
    {
        scroll--;
        render();
    }

    if (button == Button::DOWN && scroll + 4 < lines.size())
    {
        scroll++;
        render();
    }
}

void ScreenRead::render()
{
    DisplayManager& display = *context.display;

    display.clear();
    display.header("READ");

    if (state == State::WAITING)
    {
        display.centered(27, "PRESENT TAG");
    }
    else if (state == State::READING)
    {
        display.centered(27, "READING...");
    }
    else if (state == State::ERROR)
    {
        display.centered(20, "READ FAILED");
        display.centered(34, error);
    }
    else
    {
        for (size_t i = 0; i < 4 && scroll + i < lines.size(); i++)
            display.text(0, 13 + (i * 10), lines[scroll + i]);
    }

    display.footer("< BACK", state == State::RESULT ? "UP/DOWN" : "");
    display.show();
}