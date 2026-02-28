#include "Display.h"

Display::Display() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

void Display::begin() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        return; // Don't crash the whole system if display fails, just log it.
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println(F("Sous Vide Booting..."));
    display.display();
}

void Display::update(float currentTemp, float setpoint, bool isRunning, bool isHeaterOn, const String& statusMsg, IPAddress ip) {
    display.clearDisplay();
    
    // Row 1: Status and IP (y=0)
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(statusMsg);
    
    // IP on right side of row 1
    display.setCursor(75, 0);
    display.print(ip);
    
    // Row 2: Current Temperature - Large (y=12, using size 2 = 16px tall, fits in 32px display)
    display.setTextSize(2);
    display.setCursor(0, 12);
    display.print(currentTemp, 1);
    display.setTextSize(1);
    display.print("C");
    
    // Row 3: Setpoint and Heater Indicator (y=26)
    display.setCursor(0, 26);
    display.setTextSize(1);
    display.print("Set:");
    display.print(setpoint, 1);
    display.print("C");
    
    // Heater indicator on right
    if (isRunning) {
        if (isHeaterOn) {
            display.setCursor(85, 26);
            display.print("HEAT");
        } else {
            display.setCursor(85, 26);
            display.print("HOLD");
        }
    } else {
        display.setCursor(85, 26);
        display.print("IDLE");
    }

    display.display();
}

void Display::showMessage(const String& msg) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println(msg);
    display.display();
}
