#pragma once

#include <Arduino.h>

namespace Config
{
    static constexpr uint8_t I2C_SDA = 8;
    static constexpr uint8_t I2C_SCL = 9;

    static constexpr uint8_t BUTTON_UP = 2;
    static constexpr uint8_t BUTTON_DOWN = 3;
    static constexpr uint8_t BUTTON_SELECT = 4;
    static constexpr uint8_t BUTTON_BACK = 5;

    static constexpr uint8_t OLED_ADDRESS = 0x3C;
    static constexpr uint16_t OLED_WIDTH = 128;
    static constexpr uint16_t OLED_HEIGHT = 64;

    static constexpr char WEBAPP_SSID[] = "CherryRF";
    static constexpr char WEBAPP_PASSWORD[] = "CherryRF123";
}