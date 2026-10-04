#include "DisplayManager.h"

DisplayManager::DisplayManager(uint16_t width, uint16_t height, uint8_t address, TwoWire& wire)
    : width(width),
      height(height),
      address(address),
      wire(wire),
      display(width, height, &wire, -1)
{
}

bool DisplayManager::begin()
{
    // I2C is already initialized in main.cpp.
    // periphBegin=false prevents SSD1306 from calling begin() again.
    if (!display.begin(SSD1306_SWITCHCAPVCC, address, true, false)) return false;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextWrap(false);
    display.display();

    return true;
}

void DisplayManager::clear()
{
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
}

void DisplayManager::show()
{
    display.display();
}

void DisplayManager::text(int16_t x, int16_t y, const String& value, uint8_t size)
{
    display.setTextSize(size);
    display.setCursor(x, y);
    display.print(value);
}

void DisplayManager::centered(int16_t y, const String& value, uint8_t size)
{
    display.setTextSize(size);

    int16_t x1;
    int16_t y1;
    uint16_t w;
    uint16_t h;

    display.getTextBounds(value, 0, y, &x1, &y1, &w, &h);

    display.setCursor((width - w) / 2, y);
    display.print(value);
}

void DisplayManager::header(const String& title)
{
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(title);

    display.drawLine(0, 9, width - 1, 9, SSD1306_WHITE);
}

void DisplayManager::footer(const String& left, const String& right)
{
    display.drawLine(0, 54, width - 1, 54, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print(left);

    if (!right.length()) return;

    int16_t x1;
    int16_t y1;
    uint16_t w;
    uint16_t h;

    display.getTextBounds(right, 0, 56, &x1, &y1, &w, &h);

    display.setCursor(width - w, 56);
    display.print(right);
}

Adafruit_SSD1306& DisplayManager::raw()
{
    return display;
}