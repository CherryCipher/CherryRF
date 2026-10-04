#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager
{
public:
    DisplayManager(uint16_t width, uint16_t height, uint8_t address);

    bool begin();

    void clear();
    void show();

    void text(int16_t x, int16_t y, const String& value, uint8_t size = 1);
    void centered(int16_t y, const String& value, uint8_t size = 1);

    void header(const String& title);
    void footer(const String& left, const String& right = "");

    Adafruit_SSD1306& raw();

private:
    uint16_t width;
    uint16_t height;
    uint8_t address;

    Adafruit_SSD1306 display;
};