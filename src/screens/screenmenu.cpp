#include "ScreenMenu.h"

#include "../display/DisplayManager.h"
#include "ScreenManager.h"

ScreenMenu::ScreenMenu(AppContext& context)
    : context(context)
{
}

void ScreenMenu::enter()
{
    selected = 0;
}

void ScreenMenu::update()
{
}

void ScreenMenu::handleButton(Button button)
{
    if (button == Button::UP)
    {
        selected = selected == 0 ? ITEM_COUNT - 1 : selected - 1;
        render();
        return;
    }

    if (button == Button::DOWN)
    {
        selected = (selected + 1) % ITEM_COUNT;
        render();
        return;
    }

    if (button == Button::BACK)
    {
        context.screens->home();
        return;
    }

    if (button != Button::SELECT) return;

    switch (selected)
    {
        case 0: context.screens->show(ScreenID::READ); break;
        case 1: context.screens->show(ScreenID::WRITE); break;
        case 2: context.screens->show(ScreenID::COPY); break;
        case 3: context.screens->show(ScreenID::WEBAPP); break;
    }
}

void ScreenMenu::render()
{
    DisplayManager& display = *context.display;

    display.clear();
    display.header("MENU");

    for (uint8_t i = 0; i < ITEM_COUNT; i++)
    {
        int y = 13 + (i * 10);

        display.text(0, y, i == selected ? ">" : " ");
        display.text(10, y, items[i]);
    }

    display.footer("< BACK", "SELECT");
    display.show();
}