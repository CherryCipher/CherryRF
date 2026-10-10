#include "ScreenWebApp.h"

#include "../display/DisplayManager.h"
#include "../web/WebAppManager.h"
#include "../Config.h"
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
    // WebAppManager is updated globally from main.cpp.
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

    switch (state)
    {
        case State::STARTING:
            display.centered(25, "STARTING WIFI...");
            break;

        case State::RUNNING:
            display.text(0, 13, "SSID:");
            display.text(34, 13, context.webApp->getSSID());

            display.text(0, 25, "PW:");
            display.text(22, 25, Config::WEBAPP_PASSWORD);

            display.text(0, 37, "IP:");
            display.text(22, 37, context.webApp->getIP());
            break;

        case State::ERROR:
            display.centered(20, "START FAILED");
            display.centered(34, "CHECK SERIAL");
            break;
    }

    display.footer("< BACK");
    display.show();
}