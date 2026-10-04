#include <Arduino.h>
#include <Wire.h>

#include "Config.h"

#include "core/AppContext.h"

#include "input/ButtonManager.h"
#include "display/DisplayManager.h"

#include "nfc/NFCManager.h"

#include "screens/ScreenManager.h"

#include "web/WebAppManager.h"

ButtonManager buttons(
    Config::BUTTON_UP,
    Config::BUTTON_DOWN,
    Config::BUTTON_SELECT,
    Config::BUTTON_BACK
);

DisplayManager display(
    Config::OLED_WIDTH,
    Config::OLED_HEIGHT,
    Config::OLED_ADDRESS
);

NFCManager nfc(
    Config::I2C_SDA,
    Config::I2C_SCL
);

WebAppManager webApp;

AppContext context;

ScreenManager screens(context);

void setup()
{
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("============================");
    Serial.println("          CherryRF");
    Serial.println("============================");

    /*
     * OLED and PN532 share the same I2C bus.
     * NFCManager currently initializes Wire using the known-working
     * PN532 setup, so do not initialize another I2C instance.
     */

    buttons.begin();

    if (!display.begin())
    {
        Serial.println("[CherryRF] OLED initialization failed.");
    }

    if (!nfc.start())
    {
        Serial.println("[CherryRF] PN532 initialization failed.");

        display.clear();
        display.header("CherryRF");
        display.centered(20, "PN532 ERROR");
        display.centered(34, "CHECK WIRING");
        display.show();

        while (true)
            delay(1000);
    }

    context.nfc = &nfc;
    context.display = &display;
    context.buttons = &buttons;
    context.screens = &screens;
    context.webApp = &webApp;

    screens.begin();

    Serial.println("[CherryRF] Ready.");
}

void loop()
{
    buttons.update();
    screens.update();
    webApp.update();

    delay(2);
}