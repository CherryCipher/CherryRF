#include <Arduino.h>
#include <Wire.h>

#include "Config.h"

#include "core/AppContext.h"

#include "input/ButtonManager.h"
#include "display/DisplayManager.h"

#include "nfc/NFCManager.h"

#include "screens/ScreenManager.h"

#include "web/WebAppManager.h"

/*
 * ESP32-C5 I2C buses:
 *
 * Wire:
 *   PN532
 *   SDA GPIO8
 *   SCL GPIO9
 *
 * Wire1:
 *   OLED
 *   SDA GPIO2
 *   SCL GPIO3
 */
TwoWire OLEDWire(1);

ButtonManager buttons(
    Config::BUTTON_UP,
    Config::BUTTON_DOWN,
    Config::BUTTON_SELECT,
    Config::BUTTON_BACK
);

DisplayManager display(
    Config::OLED_WIDTH,
    Config::OLED_HEIGHT,
    Config::OLED_ADDRESS,
    OLEDWire
);

NFCManager nfc(
    Config::NFC_SDA,
    Config::NFC_SCL
);

/*
 * WebAppManager uses the existing NFCManager.
 */
WebAppManager webApp(nfc);

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

    buttons.begin();

    /*
     * Start OLED on the ESP32-C5 secondary / LP I2C bus.
     */
    Serial.println("[CherryRF] Starting OLED I2C...");

    if (!OLEDWire.begin(
        Config::OLED_SDA,
        Config::OLED_SCL,
        100000))
    {
        Serial.println(
            "[CherryRF] OLED I2C initialization failed."
        );

        while (true)
            delay(1000);
    }

    Serial.println(
        "[CherryRF] OLED I2C started."
    );

    if (!display.begin())
    {
        Serial.println(
            "[CherryRF] OLED initialization failed."
        );

        while (true)
            delay(1000);
    }

    Serial.println(
        "[CherryRF] OLED started."
    );

    /*
     * Start PN532 on the primary I2C bus.
     *
     * NFCManager initializes Wire using GPIO8/GPIO9.
     */
    Serial.println(
        "[CherryRF] Starting PN532..."
    );

    if (!nfc.start())
    {
        Serial.println(
            "[CherryRF] PN532 initialization failed."
        );

        display.clear();
        display.header("CherryRF");
        display.centered(
            20,
            "PN532 ERROR"
        );
        display.centered(
            34,
            "CHECK WIRING"
        );
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

    Serial.println(
        "[CherryRF] Ready."
    );
}

void loop()
{
    buttons.update();
    screens.update();

    /*
     * Only does work while WebApp mode is active.
     */
    webApp.update();

    delay(2);
}