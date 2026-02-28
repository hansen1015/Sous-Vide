#ifndef SOUS_VIDE_DISPLAY_H
#define SOUS_VIDE_DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IPAddress.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels (128x32 display)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< 0x3D for 128x64, 0x3C for 128x32

class Display {
public:
    Display();
    void begin();
    void update(float currentTemp, float setpoint, bool isRunning, bool isHeaterOn, const String& statusMsg, IPAddress ip);
    void showMessage(const String& msg);

private:
    Adafruit_SSD1306 display;
};

#endif
