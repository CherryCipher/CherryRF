#include "ScreenManager.h"

#include "ScreenHome.h"
#include "ScreenMenu.h"
#include "ScreenRead.h"
#include "ScreenWrite.h"
#include "ScreenCopy.h"
#include "ScreenWebApp.h"

ScreenManager::ScreenManager(AppContext& context)
    : context(context)
{
}

ScreenManager::~ScreenManager()
{
    destroyCurrent();
}

void ScreenManager::begin()
{
    show(ScreenID::HOME);
}

void ScreenManager::destroyCurrent()
{
    if (!current) return;

    delete current;
    current = nullptr;
}

Screen* ScreenManager::create(ScreenID id)
{
    switch (id)
    {
        case ScreenID::HOME:   return new ScreenHome(context);
        case ScreenID::MENU:   return new ScreenMenu(context);
        case ScreenID::READ:   return new ScreenRead(context);
        case ScreenID::WRITE:  return new ScreenWrite(context);
        case ScreenID::COPY:   return new ScreenCopy(context);
        case ScreenID::WEBAPP: return new ScreenWebApp(context);
        default:               return nullptr;
    }
}

void ScreenManager::show(ScreenID id)
{
    destroyCurrent();

    currentID = id;
    current = create(id);

    if (!current) return;

    current->enter();
    current->render();
}

void ScreenManager::home()
{
    show(ScreenID::HOME);
}

void ScreenManager::update()
{
    if (!current) return;

    current->update();

    Button button = context.buttons->getPressed();

    if (button != Button::NONE)
        current->handleButton(button);
}