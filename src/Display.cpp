#include "Display.h"

Display::Display() 
    : display(kScreenWidth, kScreenHeight, &Wire, kOledReset)
    , _isConnected(false)
    , _lastCurrentTemp(-999.0f)
    , _lastSetpoint(-999.0f)
    , _lastIsRunning(false)
    , _lastIsHeaterOn(false)
    , _lastIp() {
}

bool Display::begin() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, kScreenAddress)) {
        Serial.println(F("SSD1306 allocation failed"));
        _isConnected = false;
        return false;
    }
    _isConnected = true;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("Sous Vide Booting..."));
    display.display();
    return true;
}

void Display::update(float currentTemp, float setpoint, bool isRunning,
                     bool isHeaterOn, const String& statusMsg, IPAddress ip) {
    // Check if display is connected
    if (!_isConnected) {
        return;
    }
    
    // Dirty flag checking - only update if values have changed
    bool tempChanged = (abs(currentTemp - _lastCurrentTemp) > 0.05f);
    bool setpointChanged = (abs(setpoint - _lastSetpoint) > 0.05f);
    bool runningChanged = (isRunning != _lastIsRunning);
    bool heaterChanged = (isHeaterOn != _lastIsHeaterOn);
    bool statusChanged = (statusMsg != _lastStatusMsg);
    // Fixed: Compare IP addresses as uint32_t for reliable comparison
    bool ipChanged = (ip != _lastIp);
    
    bool needsFullRedraw = tempChanged || setpointChanged || runningChanged || 
                           heaterChanged || statusChanged || ipChanged;
    
    if (!needsFullRedraw) {
        return;  // No changes, skip update to preserve OLED lifespan
    }
    
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
    
    // Update cached values for dirty flag comparison
    _lastCurrentTemp = currentTemp;
    _lastSetpoint = setpoint;
    _lastIsRunning = isRunning;
    _lastIsHeaterOn = isHeaterOn;
    _lastStatusMsg = statusMsg;
    _lastIp = ip;
}

void Display::showMessage(const String& msg) {
    if (!_isConnected) {
        return;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(msg);
    display.display();
}
