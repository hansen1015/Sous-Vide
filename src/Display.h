#ifndef SOUS_VIDE_DISPLAY_H
#define SOUS_VIDE_DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IPAddress.h>

// Display configuration
static constexpr int kScreenWidth = 128;
static constexpr int kScreenHeight = 32;
static constexpr int kOledReset = -1;
static constexpr uint8_t kScreenAddress = 0x3C;

class Display {
public:
    Display();
    bool begin();
    void update(float currentTemp, float setpoint, bool isRunning, 
                bool isHeaterOn, const String& statusMsg, IPAddress ip);
    void showMessage(const String& msg);
    bool isConnected() const { return _isConnected; }

private:
    Adafruit_SSD1306 display;
    bool _isConnected = false;
    
    // For dirty flag tracking
    float _lastCurrentTemp = -999.0f;
    float _lastSetpoint = -999.0f;
    bool _lastIsRunning = false;
    bool _lastIsHeaterOn = false;
    String _lastStatusMsg;
    IPAddress _lastIp;
};

#endif
