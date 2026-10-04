#include "ScreenHome.h"

#include "../display/DisplayManager.h"
#include "../nfc/NFCManager.h"
#include "ScreenManager.h"

ScreenHome::ScreenHome(AppContext& context)
    : context(context)
{
}

void ScreenHome::enter()
{
    tag.clear();
    tagPresent = false;
    currentUID = "";
    lastScan = 0;
}

void ScreenHome::update()
{
    if (millis() - lastScan < SCAN_INTERVAL) return;

    lastScan = millis();

    NFCTag scanned;

    bool found = context.nfc->scan(scanned, 50);

    if (!found)
    {
        if (tagPresent)
        {
            tagPresent = false;
            currentUID = "";
            tag.clear();
            render();
        }

        return;
    }

    String uid = scanned.getUID();

    if (!tagPresent || uid != currentUID)
    {
        tag = scanned;
        currentUID = uid;
        tagPresent = true;
        render();
    }
}

void ScreenHome::handleButton(Button button)
{
    if (button == Button::SELECT)
        context.screens->show(ScreenID::MENU);
}

void ScreenHome::render()
{
    DisplayManager& display = *context.display;

    display.clear();
    display.header("CherryRF");

    if (!tagPresent)
    {
        display.centered(25, "PRESENT TAG");
    }
    else
    {
        display.centered(14, "TAG FOUND");
        display.centered(27, tag.getTypeName());
        display.centered(40, tag.getUID());
    }

    display.footer("", "* MENU");
    display.show();
}