#pragma once

#include <Arduino.h>

namespace Config
{
    // PN532 - primary I2C bus
    static constexpr uint8_t NFC_SDA = 8;
    static constexpr uint8_t NFC_SCL = 9;

    // OLED - secondary / LP I2C bus
    static constexpr uint8_t OLED_SDA = 2;
    static constexpr uint8_t OLED_SCL = 3;

    // Buttons
    static constexpr uint8_t BUTTON_UP = 26;
    static constexpr uint8_t BUTTON_DOWN = 25;
    static constexpr uint8_t BUTTON_SELECT = 10;
    static constexpr uint8_t BUTTON_BACK = 5;

    // OLED
    static constexpr uint8_t OLED_ADDRESS = 0x3C;
    static constexpr uint16_t OLED_WIDTH = 128;
    static constexpr uint16_t OLED_HEIGHT = 64;

    // Webapp
    static constexpr char WEBAPP_SSID[] = "CherryRF";
    static constexpr char WEBAPP_PASSWORD[] = "CherryRF123";
}