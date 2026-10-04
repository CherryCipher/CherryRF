#include "ScreenWebApp.h"

#include "../display/DisplayManager.h"
#include "../web/WebAppManager.h"
#include "ScreenManager.h"

ScreenWebApp::ScreenWebApp(AppContext& context)
    : context(context)
{
}

void ScreenWebApp::enter()
{
    state = State::STARTING;
    render();

    if (context.webApp->start())
        state = State::RUNNING;
    else
        state = State::ERROR;

    render();
}

void ScreenWebApp::update()
{
    context.webApp->update();
}

void ScreenWebApp::handleButton(Button button)
{
    if (button != Button::BACK) return;

    context.webApp->stop();
    context.screens->show(ScreenID::MENU);
}

void ScreenWebApp::render()
{
    DisplayManager& display = *context.display;

    display.clear();
    display.header("WEBAPP");

    if (state == State::STARTING)
    {
        display.centered(25, "STARTING WIFI...");
    }
    else if (state == State::RUNNING)
    {
        display.text(0, 14, "SSID:");
        display.text(0, 24, context.webApp->getSSID());

        display.text(0, 36, "IP:");
        display.text(0, 46, context.webApp->getIP());
    }
    else
    {
        display.centered(25, "START FAILED");
    }

    display.footer("< STOP");
    display.show();
}