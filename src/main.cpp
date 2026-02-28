#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include <WebServer.h>
#include <WiFiManager.h>

#include "Safety.h"
#include "Secrets.h" // User must edit this file with Bot Token
#include "WebPage.h" // Contains index_html
#include "Display.h" // Added Display

// --- PIN DEFINITIONS ---
// ESP32-C6 Pinout:
// OneWire: GPIO 4
// SSR: GPIO 3 
// I2C SDA: Default (usually GPIO 6 on C6 DevKitM-1)
// I2C SCL: Default (usually GPIO 7 on C6 DevKitM-1)
#define PIN_ONE_WIRE_BUS 4
#define PIN_SSR 3

// --- OBJECTS ---
OneWire oneWire(PIN_ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;  // Single temperature sensor

SafetySystem safety;
Display display; // Added Display Instance
Preferences preferences;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
WebServer server(80);

// --- GLOBAL STATE ---
double Setpoint = 55.0;
double Input, Output;
// PID Tuning (KP, Ki, Kd)
double Kp = 100, Ki = 0.5, Kd = 5; 
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

bool isRunning = false;
int PowerLimit = 100; // Default 100%

unsigned long lastTelegramCheck = 0;
const int telegramInterval = 2000; // Check every 2s (avoids rate limit)

// SSR Time Proportional Control
unsigned long windowStartTime;
const int WindowSize = 2000; // 2000ms window for SSR PWM

// --- FUNCTIONS ---

// Preset Management Functions
struct Preset {
    String name;
    float temp;
};

String getPresetKey(int index) {
    return "preset_" + String(index);
}

void savePreset(const String& name, float temp) {
    // Find first empty slot or existing preset
    int slot = -1;
    for (int i = 0; i < 10; i++) {  // Max 10 presets
        String key = getPresetKey(i);
        if (!preferences.isKey(key.c_str())) {
            slot = i;
            break;
        }
        // Check if preset with same name exists
        String existing = preferences.getString(key.c_str(), "");
        if (existing.startsWith(name + ":")) {
            slot = i; // Overwrite
            break;
        }
    }
    
    if (slot != -1) {
        String value = name + ";" + String(temp, 1);
        preferences.putString(getPresetKey(slot).c_str(), value);
    }
}

bool deletePreset(const String& name) {
    for (int i = 0; i < 10; i++) {
        String key = getPresetKey(i);
        String value = preferences.getString(key.c_str(), "");
        if (value.startsWith(name + ";")) {
            preferences.remove(key.c_str());
            // Compact the array
            for (int j = i; j < 9; j++) {
                String nextKey = getPresetKey(j + 1);
                String nextValue = preferences.getString(nextKey.c_str(), "");
                if (nextValue.length() > 0) {
                    preferences.putString(getPresetKey(j).c_str(), nextValue);
                    preferences.remove(nextKey.c_str());
                } else {
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

String listPresets() {
    String result = "📋 *Saved Presets:*\n\n";
    bool found = false;
    for (int i = 0; i < 10; i++) {
        String key = getPresetKey(i);
        String value = preferences.getString(key.c_str(), "");
        if (value.length() > 0) {
            int sepIndex = value.indexOf(';');
            if (sepIndex > 0) {
                String name = value.substring(0, sepIndex);
                String temp = value.substring(sepIndex + 1);
                result += "🔹 `" + name + "` → " + temp + "°C\n";
                found = true;
            }
        }
    }
    if (!found) {
        result += "_No presets saved yet._\n\n";
        result += "Add one with: `/addpreset Steak 55`";
    }
    return result;
}

float findPreset(const String& name) {
    for (int i = 0; i < 10; i++) {
        String key = getPresetKey(i);
        String value = preferences.getString(key.c_str(), "");
        if (value.length() > 0) {
            int sepIndex = value.indexOf(';');
            if (sepIndex > 0) {
                String presetName = value.substring(0, sepIndex);
                if (presetName.equalsIgnoreCase(name)) {
                    return value.substring(sepIndex + 1).toFloat();
                }
            }
        }
    }
    return -1.0; // Not found
}

String getCustomKeyboard() {
    String keyboard = "[[\"📊 Status\",\"🔴 Stop\"]";
    keyboard += ",[\"📋 Presets\",\"⚙️ Settings\"]";
    keyboard += ",[\"🥩 Steak 55°C\",\"🐟 Fish 50°C\"]";
    keyboard += ",[\"🥚 Egg 64°C\",\"🍗 Chicken 65°C\"]]";
    return keyboard;
}

void handleRoot() {
    server.send_P(200, "text/html; charset=utf-8", index_html);
}

void handleStatus() {
    SafetyStatus status = safety.getStatus();
    JsonDocument doc;
    doc["current"] = status.currentTemp;
    doc["t1"] = status.temp1;
    doc["target"] = Setpoint;
    doc["running"] = isRunning;
    doc["pwm"] = (Output / WindowSize) * 100.0;
    doc["heater"] = (Output > 0);
    doc["powerLimit"] = PowerLimit;
    if (status.emergencyStop) {
        doc["error"] = status.reason;
        doc["errorCode"] = status.errorCode;
    }
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void handleSet() {
    if (server.hasArg("target")) {
        Setpoint = server.arg("target").toDouble();
    }
    if (server.hasArg("limit")) {
        int l = server.arg("limit").toInt();
        if (l >= 0 && l <= 100) {
            PowerLimit = l;
            preferences.putInt("power", PowerLimit);
            // Re-apply limit to PID
             myPID.SetOutputLimits(0, WindowSize * (PowerLimit / 100.0));
        }
    }
    if (server.hasArg("run")) {
        int r = server.arg("run").toInt();
        isRunning = (r == 1);
        if (isRunning) {
            safety.reset();
            windowStartTime = millis();
        }
    }
    server.send(200, "text/plain", "OK");
}

void handleTelegram() {
    Serial.println("Checking Telegram...");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    if (numNewMessages > 0) {
        Serial.println("Got " + String(numNewMessages) + " messages");
        for (int i = 0; i < numNewMessages; i++) {
            String chat_id = String(bot.messages[i].chat_id);
            String text = bot.messages[i].text;
            Serial.println("Received: " + text + " from " + chat_id);

            // --- /start command with custom keyboard ---
            if (text == "/start") {
                preferences.putString("chat_id", chat_id);
                String welcome = "🌡️ *Sous Vide Controller*\n\n";
                welcome += "Welcome! Use the keyboard below or these commands:\n\n";
                welcome += "📊 `/status` - Current status\n";
                welcome += "📋 `/presets` - List presets\n";
                welcome += "⚙️ `/settings` - Configure\n";
                
                String keyboard = "[[\"📊 Status\",\"🔴 Stop\"],[\"📋 Presets\",\"⚙️ Settings\"],[\"🥩 Steak 55°C\",\"🐟 Fish 50°C\"],[\"🥚 Egg 64°C\",\"🍗 Chicken 65°C\"]]";
                
                bool sent = bot.sendMessageWithReplyKeyboard(chat_id, welcome, "Markdown", keyboard);
                Serial.println("Welcome Sent: " + String(sent));
            }
            
            // --- Status command ---
            else if (text == "/status" || text == "📊 Status") {
                SafetyStatus s = safety.getStatus();
                float pwm = (Output / WindowSize) * 100.0;
                
                String msg = "📊 *Current Status*\n\n";
                msg += "🌡️ Temperature: *" + String(s.currentTemp, 1) + "°C*\n";
                msg += "🎯 Target: *" + String(Setpoint, 1) + "°C*\n";
                msg += "⚡ PWM Output: " + String(pwm, 0) + "%\n";
                msg += "🔋 Power Limit: " + String(PowerLimit) + "%\n";
                msg += "🔄 State: *" + String(isRunning ? "🟢 RUNNING" : "⚪ IDLE") + "*";
                if (s.emergencyStop) msg += "\n🚨 ERROR: " + s.reason;
                
                bool sent = bot.sendMessage(chat_id, msg, "Markdown");
                Serial.println("Status Sent: " + String(sent));
            }
            
            // --- Stop command ---
            else if (text == "/stop" || text == "🔴 Stop") {
                isRunning = false;
                bool sent = bot.sendMessage(chat_id, "🛑 Cooking stopped.", "");
                Serial.println("Stop Sent: " + String(sent));
            }
            
            // --- List Presets ---
            else if (text == "/presets" || text == "📋 Presets") {
                bot.sendMessage(chat_id, listPresets(), "Markdown");
            }
            
            // --- Add Preset: /addpreset Steak 55 ---
            else if (text.startsWith("/addpreset ")) {
                String params = text.substring(11);
                params.trim();
                int spaceIndex = params.lastIndexOf(' ');
                if (spaceIndex > 0) {
                    String name = params.substring(0, spaceIndex);
                    float temp = params.substring(spaceIndex + 1).toFloat();
                    if (temp > 20 && temp < 90) {
                        savePreset(name, temp);
                        bot.sendMessage(chat_id, "✅ Preset *" + name + "* saved at " + String(temp, 1) + "°C", "Markdown");
                    } else {
                        bot.sendMessage(chat_id, "❌ Invalid temperature (20-90°C)", "");
                    }
                } else {
                    bot.sendMessage(chat_id, "Usage: `/addpreset Steak 55`", "Markdown");
                }
            }
            
            // --- Delete Preset: /delpreset Steak ---
            else if (text.startsWith("/delpreset ")) {
                String name = text.substring(11);
                name.trim();
                if (deletePreset(name)) {
                    bot.sendMessage(chat_id, "🗑️ Preset *" + name + "* deleted.", "Markdown");
                } else {
                    bot.sendMessage(chat_id, "❌ Preset not found.", "");
                }
            }
            
            // --- Cook with preset: /cook Steak ---
            else if (text.startsWith("/cook ")) {
                String name = text.substring(6);
                name.trim();
                float temp = findPreset(name);
                if (temp > 0) {
                    Setpoint = temp;
                    isRunning = true;
                    safety.reset();
                    windowStartTime = millis();
                    bot.sendMessage(chat_id, "🔥 Cooking *" + name + "* at " + String(temp, 1) + "°C", "Markdown");
                } else {
                    bot.sendMessage(chat_id, "❌ Preset not found. Use `/presets` to see available presets.", "Markdown");
                }
            }
            
            // --- Quick Preset Buttons ---
            else if (text.startsWith("🥩 ") || text.startsWith("🐟 ") || text.startsWith("🥚 ") || text.startsWith("🍗 ")) {
                // Extract temperature from button text like "🥩 Steak 55°C"
                int tempStart = text.lastIndexOf(' ') + 1;
                int tempEnd = text.indexOf("°C");
                if (tempStart > 0 && tempEnd > tempStart) {
                    float temp = text.substring(tempStart, tempEnd).toFloat();
                    if (temp > 20 && temp < 90) {
                        Setpoint = temp;
                        isRunning = true;
                        safety.reset();
                        windowStartTime = millis();
                        bot.sendMessage(chat_id, "🔥 Started cooking at " + String(temp, 1) + "°C", "");
                    }
                }
            }
            
            // --- Settings Menu ---
            else if (text == "⚙️ Settings" || text == "/settings") {
                String msg = "⚙️ *Settings*\n\n";
                msg += "Current power limit: " + String(PowerLimit) + "%\n\n";
                msg += "Use `/setpower <0-100>` to change power limit.";
                bot.sendMessage(chat_id, msg, "Markdown");
            }
            
            // --- Set Power Limit ---
            else if (text.startsWith("/setpower ")) {
                int power = text.substring(10).toInt();
                if (power >= 0 && power <= 100) {
                    PowerLimit = power;
                    preferences.putInt("power", PowerLimit);
                    myPID.SetOutputLimits(0, WindowSize * (PowerLimit / 100.0));
                    bot.sendMessage(chat_id, "✅ Power limit set to " + String(PowerLimit) + "%", "");
                } else {
                    bot.sendMessage(chat_id, "❌ Invalid power (0-100)", "");
                }
            }
            
            // --- Set Temperature: /set 60 ---
            else if (text.startsWith("/set ")) {
                float t = text.substring(5).toFloat();
                if (t > 20 && t < 90) {
                    Setpoint = t;
                    bot.sendMessage(chat_id, "🌡️ Target set to " + String(Setpoint, 1) + "°C\n\nUse `/cook` to start.", "Markdown");
                } else {
                    bot.sendMessage(chat_id, "❌ Invalid temperature (20-90°C)", "");
                }
            }
            
            // --- Unknown command ---
            else if (text.startsWith("/")) {
                bot.sendMessage(chat_id, "❓ Unknown command. Use /start to see available commands.", "");
            }
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
}

void setup() {
    Serial.begin(115200);
    preferences.begin("sousvide", false); // Namespace "sousvide"
    PowerLimit = preferences.getInt("power", 100);
    
    // Display Init
    display.begin();
    display.showMessage("Connecting WiFi...");

    pinMode(PIN_SSR, OUTPUT);
    digitalWrite(PIN_SSR, LOW);

    // Sensors
    sensors.begin();
    if(sensors.getDeviceCount() < 1) Serial.println("WARNING: No sensor found!");
    sensors.getAddress(sensor1, 0);

    // WiFi via WiFiManager
    WiFiManager wifiManager;
    // Tries to connect to last known settings.
    // If it fails, it starts an AP "SousVide_Config_AP" for calibration/setup.
    // We set a timeout (e.g., 180s) so if no one configures it, we can fall back to 
    // a distinct "Offline Mode" AP where the control interface lives.
    wifiManager.setConfigPortalTimeout(180); 
    
    // Callback to update display during config portal? Optional but nice.
    wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
        display.showMessage("Config AP: " + myWiFiManager->getConfigPortalSSID());
    });

    if (!wifiManager.autoConnect("SousVide_Config_AP")) {
        Serial.println("WiFi Manager Timeout - Starting Offline AP for Control");
        WiFi.softAP("SousVide_Offline");
        display.showMessage("Offline AP Mode");
        // In this mode, we have NO Internet, so Telegram won't work.
    } else {
        Serial.println("WiFi Connected");
        display.showMessage("WiFi Connected!");
        
        // Sync time for SSL validation (Required for Telegram)
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");
        Serial.print("Syncing Time");
        time_t now = time(nullptr);
        while (now < 100000) {
            delay(100);
            Serial.print(".");
            now = time(nullptr);
        }
        Serial.println("\nTime Synced");
    }

    // Telegram Cert - Insecure mode effectively disables validation, 
    // but correct time is still good practice and sometimes required.
    secured_client.setInsecure();

    // Web Server
    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/set", handleSet);
    ElegantOTA.begin(&server);
    server.begin();

    // Async Temp Reading
    sensors.setWaitForConversion(false);

    // PID
    myPID.SetOutputLimits(0, WindowSize * (PowerLimit / 100.0)); // Apply Limit
    myPID.SetMode(AUTOMATIC);
    windowStartTime = millis();
}

unsigned long lastTempRequest = 0;
const int tempInterval = 1000; // Read every 1s
const int conversionDelay = 750; // 12-bit
bool isConversionStarted = false;
unsigned long lastDisplayUpdate = 0;
const int displayInterval = 500; // Update display every 500ms

void loop() {
    server.handleClient();
    ElegantOTA.loop();

    unsigned long currentMillis = millis();

    // 1. Safety Loop (Non-blocking Temp Read)
    if (!isConversionStarted && (currentMillis - lastTempRequest >= tempInterval)) {
        sensors.requestTemperatures(); 
        isConversionStarted = true;
        lastTempRequest = currentMillis;
    }

    if (isConversionStarted && (currentMillis - lastTempRequest >= conversionDelay)) {
        float t1 = sensors.getTempC(sensor1);
        
        safety.loop(t1);
        isConversionStarted = false;
        
        // Only run PID compute when we have new data
        SafetyStatus s = safety.getStatus();
        if (s.emergencyStop) {
            isRunning = false;
            // Digital write handled in fast loop below
        } else if (isRunning) {
            Input = s.currentTemp;
            myPID.Compute();
        }
    }

    // 2. Control Loop (Fast PWM)
    SafetyStatus s = safety.getStatus();

    if (isRunning && !s.emergencyStop) {
        // Time Proportional PWM for SSR
        if (currentMillis - windowStartTime > WindowSize) {
            windowStartTime += WindowSize;
        }
        if (Output > (currentMillis - windowStartTime)) digitalWrite(PIN_SSR, HIGH);
        else digitalWrite(PIN_SSR, LOW);
    } else {
        digitalWrite(PIN_SSR, LOW);
        Output = 0; // Reset output
    }

    // 3. Telegram (Only if connected to Internet)
    if (WiFi.status() == WL_CONNECTED && (currentMillis - lastTelegramCheck > telegramInterval)) {
        lastTelegramCheck = currentMillis;
        handleTelegram(); 
    }

    // 4. Update Display
    if (currentMillis - lastDisplayUpdate > displayInterval) {
        lastDisplayUpdate = currentMillis;
        String statusMsg = isRunning ? "RUNNING" : "IDLE";
        if (s.emergencyStop) {
             statusMsg = "ERR: " + s.reason;
        }
        bool heaterOn = (Output > 0) && isRunning;
        IPAddress ip = WiFi.localIP();
        if (WiFi.getMode() == WIFI_AP) {
            ip = WiFi.softAPIP();
        }
        display.update(s.currentTemp, Setpoint, isRunning, heaterOn, statusMsg, ip);
    }

}
