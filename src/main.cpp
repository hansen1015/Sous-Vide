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
#include <esp_task_wdt.h>

#include "Safety.h"
#include "Secrets.h"
#include "WebPage.h"
#include "Display.h"

// --- PIN DEFINITIONS ---
// ESP32-C6 Pinout validation
#define PIN_ONE_WIRE_BUS 4
#define PIN_SSR 3

// Compile-time pin validation for ESP32-C6
static_assert(PIN_ONE_WIRE_BUS != 0, "GPIO0 is a strapping pin");
static_assert(PIN_ONE_WIRE_BUS != 1, "GPIO1 is used for USB");
static_assert(PIN_SSR != 0, "GPIO0 is a strapping pin");

// --- CONSTANTS ---
static constexpr int kTelegramIntervalMs = 2000;
static constexpr int kWindowSizeMs = 2000;
static constexpr int kTempIntervalMs = 1000;
static constexpr int kConversionDelayMs = 750;  // 12-bit resolution
static constexpr int kDisplayIntervalMs = 500;
static constexpr int kReconnectIntervalMs = 60000;
static constexpr size_t kMaxPresetNameLength = 32;
static constexpr float kPresetMinTemp = 20.0f;
static constexpr float kPresetMaxTemp = 90.0f;

// --- BUFFER SIZES FOR STRING OPTIMIZATION ---
static constexpr size_t kMsgBufferSize = 320;
static constexpr size_t kSmallBufferSize = 64;

// --- STATIC CONST STRINGS (stored in flash on ESP32 via RODATA) ---
// These are placed in flash automatically by the compiler on ESP32
static const char kMsgWelcome[] = "🌡️ *Sous Vide Controller*\n\n"
    "Welcome! Use the keyboard below or these commands:\n\n"
    "📊 `/status` - Current status\n"
    "📋 `/presets` - List presets\n"
    "⚙️ `/settings` - Configure\n";
static const char kMsgKeyboard[] = "[[\"📊 Status\",\"🔴 Stop\"],[\"📋 Presets\",\"⚙️ Settings\"],[\"🥩 Steak 55°C\",\"🐟 Fish 50°C\"],[\"🥚 Egg 64°C\",\"🍗 Chicken 65°C\"]]";
static const char kMsgNoPresets[] = "📋 *Saved Presets:*\n\n_No presets saved yet._\n\nAdd one with: `/addpreset Steak 55`";
static const char kMsgPresetHeader[] = "📋 *Saved Presets:*\n\n";
static const char kMsgCookingStopped[] = "🛑 Cooking stopped.";
static const char kMsgSensorError[] = "⚠️ Cannot start: Temperature sensor not detected.";
static const char kMsgSensorErrorDisplay[] = "NO SENSOR!";
static const char kMsgUnknownCommand[] = "❓ Unknown command. Use /start to see available commands.";
static const char kMsgInvalidTemp[] = "❌ Invalid temperature (20-90°C)";
static const char kMsgInvalidPower[] = "❌ Invalid power (0-100)";
static const char kMsgPresetNotFound[] = "❌ Preset not found. Use `/presets` to see available presets.";
static const char kMsgPresetUsage[] = "Usage: `/addpreset Steak 55`";
static const char kMsgPresetSaveFailed[] = "❌ Failed to save preset (invalid name or storage full)";
static const char kMsgPresetDeleted[] = "🗑️ Preset *";
static const char kMsgPresetNotFoundShort[] = "❌ Preset not found.";
static const char kMsgSettingsHeader[] = "⚙️ *Settings*\n\nCurrent power limit: ";
static const char kMsgSettingsFooter[] = "%\n\nUse `/setpower <0-100>` to change power limit.";
static const char kMsgStatusHeader[] = "📊 *Current Status*\n\n";
static const char kMsgTemp[] = "🌡️ Temperature: *";
static const char kMsgTarget[] = "🎯 Target: *";
static const char kMsgPWM[] = "⚡ PWM Output: ";
static const char kMsgPower[] = "🔋 Power Limit: ";
static const char kMsgState[] = "🔄 State: *";
static const char kMsgRunning[] = "🟢 RUNNING";
static const char kMsgIdle[] = "⚪ IDLE";
static const char kMsgError[] = "\n🚨 ERROR: ";
static const char kMsgCookingAt[] = "🔥 Cooking *";
static const char kMsgStartedAt[] = "🔥 Started cooking at ";
static const char kMsgTargetSet[] = "🌡️ Target set to ";
static const char kMsgUseCook[] = "°C\n\nUse `/cook` to start.";
static const char kMsgPowerSet[] = "✅ Power limit set to ";
static const char kMsgPresetSaved[] = "✅ Preset *";
static const char kMsgAtTemp[] = "* saved at ";
static const char kMsgSensorStatus[] = "\n⚠️ *Sensor: NOT DETECTED*";

// --- OBJECTS ---
OneWire oneWire(PIN_ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

SafetySystem safety;
Display display;
Preferences preferences;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
WebServer server(80);

// --- CONTROLLER STATE ---
struct ControllerState {
    double setpoint = 55.0;
    double input = 0.0;
    double output = 0.0;
    double kp = 100.0;
    double ki = 0.5;
    double kd = 5.0;
    bool isRunning = false;
    int powerLimit = 100;
    
    PID pid;
    
    ControllerState() 
        : pid(&input, &output, &setpoint, kp, ki, kd, DIRECT) {}
};

ControllerState controller;

// --- TIMING STATE ---
unsigned long lastTelegramCheck = 0;
unsigned long windowStartTime;
unsigned long lastTempRequest = 0;
bool isConversionStarted = false;
unsigned long lastDisplayUpdate = 0;
unsigned long lastReconnectAttempt = 0;

// --- SENSOR STATE ---
bool sensorPresent = false;

// --- HELPER FUNCTIONS FOR STRING OPTIMIZATION ---
// Append string to buffer with length checking
void appendStr(char* buf, size_t bufSize, const char* src) {
    size_t len = strlen(buf);
    if (len < bufSize - 1) {
        strncat(buf, src, bufSize - len - 1);
    }
}

// Append float to buffer
void appendFloat(char* buf, size_t bufSize, float value, int decimals) {
    size_t len = strlen(buf);
    if (len < bufSize - 10) {
        dtostrf(value, 0, decimals, buf + len);
    }
}

// Append int to buffer
void appendInt(char* buf, size_t bufSize, int value) {
    size_t len = strlen(buf);
    if (len < bufSize - 10) {
        snprintf(buf + len, bufSize - len, "%d", value);
    }
}

// Check if sensor is present and valid
bool checkSensorPresent() {
    return sensorPresent && sensors.getDeviceCount() > 0;
}

// --- PRESET MANAGEMENT ---
void getPresetKey(char* buf, size_t bufSize, int index) {
    snprintf(buf, bufSize, "preset_%d", index);
}

bool validatePresetName(const String& name) {
    if (name.length() == 0 || name.length() > kMaxPresetNameLength) {
        return false;
    }
    for (size_t i = 0; i < name.length(); i++) {
        char c = name.charAt(i);
        if (c == ';' || c < 32) {
            return false;
        }
    }
    return true;
}

bool validatePresetTemp(float temp) {
    return (temp >= kPresetMinTemp && temp <= kPresetMaxTemp);
}

bool savePreset(const String& name, float temp) {
    if (!validatePresetName(name) || !validatePresetTemp(temp)) {
        return false;
    }
    
    char key[16];
    int slot = -1;
    for (int i = 0; i < 10; i++) {
        getPresetKey(key, sizeof(key), i);
        if (!preferences.isKey(key)) {
            slot = i;
            break;
        }
        String existing = preferences.getString(key, "");
        if (existing.startsWith(name + ";")) {
            slot = i;
            break;
        }
    }
    
    if (slot != -1) {
        char value[48];
        snprintf(value, sizeof(value), "%s;%.1f", name.c_str(), temp);
        getPresetKey(key, sizeof(key), slot);
        preferences.putString(key, value);
        return true;
    }
    return false;
}

bool deletePreset(const String& name) {
    char key[16], nextKey[16];
    for (int i = 0; i < 10; i++) {
        getPresetKey(key, sizeof(key), i);
        String value = preferences.getString(key, "");
        if (value.startsWith(name + ";")) {
            preferences.remove(key);
            for (int j = i; j < 9; j++) {
                getPresetKey(nextKey, sizeof(nextKey), j + 1);
                String nextValue = preferences.getString(nextKey, "");
                if (nextValue.length() > 0) {
                    getPresetKey(key, sizeof(key), j);
                    preferences.putString(key, nextValue);
                    preferences.remove(nextKey);
                } else {
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

// Optimized listPresets using pre-allocated buffer
void listPresets(char* buf, size_t bufSize) {
    strcpy(buf, kMsgPresetHeader);
    size_t pos = strlen(buf);
    
    bool found = false;
    char key[16];
    for (int i = 0; i < 10 && pos < bufSize - 50; i++) {
        getPresetKey(key, sizeof(key), i);
        String value = preferences.getString(key, "");
        if (value.length() > 0) {
            int sepIndex = value.indexOf(';');
            if (sepIndex > 0) {
                String name = value.substring(0, sepIndex);
                String temp = value.substring(sepIndex + 1);
                pos += snprintf(buf + pos, bufSize - pos, "🔹 `%s` → %s°C\n", name.c_str(), temp.c_str());
                found = true;
            }
        }
    }
    
    if (!found) {
        strcpy(buf, kMsgNoPresets);
    }
}

float findPreset(const String& name) {
    char key[16];
    for (int i = 0; i < 10; i++) {
        getPresetKey(key, sizeof(key), i);
        String value = preferences.getString(key, "");
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
    return -1.0f;
}

// --- WEB SERVER HANDLERS ---
void handleRoot() {
    server.send_P(200, "text/html; charset=utf-8", index_html);
}

void handleStatus() {
    SafetyStatus status = safety.getStatus();
    JsonDocument doc;
    doc["current"] = status.currentTemp;
    doc["t1"] = status.temp1;
    doc["t2"] = status.temp1;  // t2 same as t1 for single-sensor setup
    doc["target"] = controller.setpoint;
    doc["running"] = controller.isRunning;
    doc["pwm"] = (controller.output / kWindowSizeMs) * 100.0;
    doc["heater"] = (controller.output > 0);
    doc["powerLimit"] = controller.powerLimit;
    doc["sensorPresent"] = checkSensorPresent();
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
        controller.setpoint = server.arg("target").toDouble();
    }
    if (server.hasArg("limit")) {
        int l = server.arg("limit").toInt();
        if (l >= 0 && l <= 100) {
            controller.powerLimit = l;
            preferences.putInt("power", controller.powerLimit);
            float powerRatio = static_cast<float>(controller.powerLimit) / 100.0f;
            controller.pid.SetOutputLimits(0, kWindowSizeMs * powerRatio);
        }
    }
    if (server.hasArg("run")) {
        int r = server.arg("run").toInt();
        if (r == 1) {
            // Check sensor before starting
            if (!checkSensorPresent()) {
                display.showMessage(kMsgSensorErrorDisplay);
                server.send(400, "text/plain", "ERROR: No temperature sensor detected");
                return;
            }
            controller.isRunning = true;
            safety.start();
            windowStartTime = millis();
        } else {
            controller.isRunning = false;
            safety.stop();
        }
    }
    server.send(200, "text/plain", "OK");
}

// --- TELEGRAM HANDLER (OPTIMIZED) ---
void handleTelegram() {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    if (numNewMessages <= 0) {
        return;
    }
    
    // Pre-allocate buffers for messages
    static char msgBuf[kMsgBufferSize];
    
    Serial.printf("Got %d messages\n", numNewMessages);
    
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        Serial.printf("Received: %s from %s\n", text.c_str(), chat_id.c_str());

        // --- /start command ---
        if (text == "/start") {
            preferences.putString("chat_id", chat_id);
            bot.sendMessageWithReplyKeyboard(chat_id, kMsgWelcome, "Markdown", kMsgKeyboard);
            Serial.println("Welcome Sent");
        }
        
        // --- Status command ---
        else if (text == "/status" || text == "📊 Status") {
            SafetyStatus s = safety.getStatus();
            float pwm = (controller.output / kWindowSizeMs) * 100.0;
            
            // Build status message using pre-allocated buffer
            // Use snprintf for the first part to initialize the buffer
            snprintf(msgBuf, sizeof(msgBuf), "%s%s%.1f°C*\n%s%.1f°C*\n%s%.0f%%\n%s%d%%\n%s%s*",
                     kMsgStatusHeader,
                     kMsgTemp, s.currentTemp,
                     kMsgTarget, controller.setpoint,
                     kMsgPWM, pwm,
                     kMsgPower, controller.powerLimit,
                     kMsgState, controller.isRunning ? kMsgRunning : kMsgIdle);
            
            if (s.emergencyStop) {
                size_t len = strlen(msgBuf);
                snprintf(msgBuf + len, sizeof(msgBuf) - len, "%s%s", kMsgError, s.reason.c_str());
            }
            
            // Add sensor status
            if (!checkSensorPresent()) {
                size_t len = strlen(msgBuf);
                snprintf(msgBuf + len, sizeof(msgBuf) - len, "%s", kMsgSensorStatus);
            }
            
            bot.sendMessage(chat_id, msgBuf, "Markdown");
            Serial.println("Status Sent");
        }
        
        // --- Stop command ---
        else if (text == "/stop" || text == "🔴 Stop") {
            controller.isRunning = false;
            safety.stop();
            bot.sendMessage(chat_id, kMsgCookingStopped, "");
            Serial.println("Stop Sent");
        }
        
        // --- List Presets ---
        else if (text == "/presets" || text == "📋 Presets") {
            listPresets(msgBuf, sizeof(msgBuf));
            bot.sendMessage(chat_id, msgBuf, "Markdown");
        }
        
        // --- Add Preset ---
        else if (text.startsWith("/addpreset ")) {
            String params = text.substring(11);
            params.trim();
            int spaceIndex = params.lastIndexOf(' ');
            if (spaceIndex > 0) {
                String name = params.substring(0, spaceIndex);
                float temp = params.substring(spaceIndex + 1).toFloat();
                if (validatePresetTemp(temp)) {
                    if (savePreset(name, temp)) {
                        snprintf(msgBuf, sizeof(msgBuf), "%s%s%s%.1f°C", 
                                 kMsgPresetSaved, name.c_str(), 
                                 kMsgAtTemp, temp);
                        bot.sendMessage(chat_id, msgBuf, "Markdown");
                    } else {
                        bot.sendMessage(chat_id, kMsgPresetSaveFailed, "Markdown");
                    }
                } else {
                    bot.sendMessage(chat_id, kMsgInvalidTemp, "");
                }
            } else {
                bot.sendMessage(chat_id, kMsgPresetUsage, "Markdown");
            }
        }
        
        // --- Delete Preset ---
        else if (text.startsWith("/delpreset ")) {
            String name = text.substring(11);
            name.trim();
            if (deletePreset(name)) {
                snprintf(msgBuf, sizeof(msgBuf), "%s%s* deleted.", 
                         kMsgPresetDeleted, name.c_str());
                bot.sendMessage(chat_id, msgBuf, "Markdown");
            } else {
                bot.sendMessage(chat_id, kMsgPresetNotFoundShort, "");
            }
        }
        
        // --- Cook with preset ---
        else if (text.startsWith("/cook ")) {
            String name = text.substring(6);
            name.trim();
            float temp = findPreset(name);
            if (temp > 0) {
                // Check sensor before starting
                if (!checkSensorPresent()) {
                    display.showMessage(kMsgSensorErrorDisplay);
                    bot.sendMessage(chat_id, kMsgSensorError, "");
                    continue;
                }
                controller.setpoint = temp;
                controller.isRunning = true;
                safety.start();
                windowStartTime = millis();
                snprintf(msgBuf, sizeof(msgBuf), "%s%s* at %.1f°C", 
                         kMsgCookingAt, name.c_str(), temp);
                bot.sendMessage(chat_id, msgBuf, "Markdown");
            } else {
                bot.sendMessage(chat_id, kMsgPresetNotFound, "Markdown");
            }
        }
        
        // --- Quick Preset Buttons ---
        else if (text.startsWith("🥩 ") || text.startsWith("🐟 ") || text.startsWith("🥚 ") || text.startsWith("🍗 ")) {
            int tempStart = text.lastIndexOf(' ') + 1;
            int tempEnd = text.indexOf("°C");
            if (tempStart > 0 && tempEnd > tempStart) {
                float temp = text.substring(tempStart, tempEnd).toFloat();
                if (validatePresetTemp(temp)) {
                    // Check sensor before starting
                    if (!checkSensorPresent()) {
                        display.showMessage(kMsgSensorErrorDisplay);
                        bot.sendMessage(chat_id, kMsgSensorError, "");
                        continue;
                    }
                    controller.setpoint = temp;
                    controller.isRunning = true;
                    safety.start();
                    windowStartTime = millis();
                    snprintf(msgBuf, sizeof(msgBuf), "%s%.1f°C", 
                             kMsgStartedAt, temp);
                    bot.sendMessage(chat_id, msgBuf, "");
                }
            }
        }
        
        // --- Settings Menu ---
        else if (text == "⚙️ Settings" || text == "/settings") {
            snprintf(msgBuf, sizeof(msgBuf), "%s%d%s", 
                     kMsgSettingsHeader, controller.powerLimit,
                     kMsgSettingsFooter);
            bot.sendMessage(chat_id, msgBuf, "Markdown");
        }
        
        // --- Set Power Limit ---
        else if (text.startsWith("/setpower ")) {
            int power = text.substring(10).toInt();
            if (power >= 0 && power <= 100) {
                controller.powerLimit = power;
                preferences.putInt("power", controller.powerLimit);
                float powerRatio = static_cast<float>(controller.powerLimit) / 100.0f;
                controller.pid.SetOutputLimits(0, kWindowSizeMs * powerRatio);
                snprintf(msgBuf, sizeof(msgBuf), "%s%d%%", 
                         kMsgPowerSet, controller.powerLimit);
                bot.sendMessage(chat_id, msgBuf, "");
            } else {
                bot.sendMessage(chat_id, kMsgInvalidPower, "");
            }
        }
        
        // --- Set Temperature ---
        else if (text.startsWith("/set ")) {
            float t = text.substring(5).toFloat();
            if (validatePresetTemp(t)) {
                controller.setpoint = t;
                snprintf(msgBuf, sizeof(msgBuf), "%s%.1f%s", 
                         kMsgTargetSet, controller.setpoint,
                         kMsgUseCook);
                bot.sendMessage(chat_id, msgBuf, "Markdown");
            } else {
                bot.sendMessage(chat_id, kMsgInvalidTemp, "");
            }
        }
        
        // --- Unknown command ---
        else if (text.startsWith("/")) {
            bot.sendMessage(chat_id, kMsgUnknownCommand, "");
        }
    }
    
    // Clear processed messages
    bot.getUpdates(bot.last_message_received + 1);
}

// --- WIFI RECONNECTION ---
void tryWiFiReconnect() {
    if (WiFi.status() != WL_CONNECTED && 
        millis() - lastReconnectAttempt >= kReconnectIntervalMs) {
        lastReconnectAttempt = millis();
        Serial.println("Attempting WiFi reconnection...");
        WiFi.reconnect();
    }
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);
    
    // Initialize watchdog timer (10 second timeout)
    // ESP32-C6 uses esp_task_wdt_init with config struct
    esp_task_wdt_config_t twdt_config = {
        10000,      // timeout_ms
        0,          // idle_core_mask (bitmask of idle tasks to watch)
        true        // trigger_panic
    };
    esp_task_wdt_init(&twdt_config);
    esp_task_wdt_add(NULL);
    
    preferences.begin("sousvide", false);
    controller.powerLimit = preferences.getInt("power", 100);
    
    // Display initialization
    display.begin();
    display.showMessage("Connecting WiFi...");

    pinMode(PIN_SSR, OUTPUT);
    digitalWrite(PIN_SSR, LOW);

    // Sensors
    sensors.begin();
    sensorPresent = (sensors.getDeviceCount() > 0);
    if (!sensorPresent) {
        Serial.println("ERROR: No temperature sensor found!");
        display.showMessage("NO SENSOR!");
        delay(2000);  // Show error for 2 seconds
    } else {
        Serial.printf("Found %d sensor(s)\n", sensors.getDeviceCount());
        sensors.getAddress(sensor1, 0);
    }

    // WiFi via WiFiManager with password protection
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(180);
    
    wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
        display.showMessage("Config AP: " + myWiFiManager->getConfigPortalSSID());
    });

    // Use password-protected autoConnect for security
    // AP name: SousVide_Config_AP, Password: from WIFI_AP_PASSWORD
    if (!wifiManager.autoConnect("SousVide_Config_AP", WIFI_AP_PASSWORD)) {
        Serial.println("WiFi Manager Timeout - Starting Offline AP for Control");
        WiFi.softAP("SousVide_Offline");
        display.showMessage("Offline AP Mode");
    } else {
        Serial.println("WiFi Connected");
        display.showMessage("WiFi Connected!");
        
        // Sync time for SSL validation
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

    // Telegram - Insecure mode for development
    secured_client.setInsecure();

    // Web Server with OTA authentication
    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/set", handleSet);
    
    // ElegantOTA with authentication
    ElegantOTA.begin(&server, OTA_USERNAME, OTA_PASSWORD);
    Serial.printf("OTA Update available at /update (user: %s)\n", OTA_USERNAME);
    
    server.begin();

    // Async Temperature Reading
    sensors.setWaitForConversion(false);

    // PID initialization
    float powerRatio = static_cast<float>(controller.powerLimit) / 100.0f;
    controller.pid.SetOutputLimits(0, kWindowSizeMs * powerRatio);
    controller.pid.SetMode(AUTOMATIC);
    windowStartTime = millis();
    
    // Feed watchdog at end of setup
    esp_task_wdt_reset();
}

// --- MAIN LOOP ---
void loop() {
    // Feed watchdog timer
    esp_task_wdt_reset();
    
    server.handleClient();
    ElegantOTA.loop();

    unsigned long currentMillis = millis();

    // 1. Safety Loop (Non-blocking Temp Read)
    if (!isConversionStarted && (currentMillis - lastTempRequest >= kTempIntervalMs)) {
        sensors.requestTemperatures(); 
        isConversionStarted = true;
        lastTempRequest = currentMillis;
    }

    if (isConversionStarted && (currentMillis - lastTempRequest >= kConversionDelayMs)) {
        float t1 = sensors.getTempC(sensor1);
        
        // Check for sensor disconnection during runtime
        if (sensorPresent && (std::isnan(t1) || t1 < -100.0f)) {
            // Sensor was present but now returning invalid readings
            sensorPresent = false;
            Serial.println("ERROR: Temperature sensor disconnected!");
            controller.isRunning = false;
            safety.stop();
        } else if (!sensorPresent && sensors.getDeviceCount() > 0) {
            // Sensor reconnected
            sensorPresent = true;
            sensors.getAddress(sensor1, 0);
            Serial.println("Temperature sensor reconnected.");
        }
        
        safety.loop(t1);
        isConversionStarted = false;
        
        SafetyStatus s = safety.getStatus();
        if (s.emergencyStop) {
            controller.isRunning = false;
        } else if (controller.isRunning) {
            controller.input = s.currentTemp;
            controller.pid.Compute();
        }
    }

    // 2. Control Loop (Fast PWM)
    SafetyStatus s = safety.getStatus();

    if (controller.isRunning && !s.emergencyStop && checkSensorPresent()) {
        // Handle millis() overflow gracefully - subtraction works correctly due to unsigned wraparound
        unsigned long elapsed = currentMillis - windowStartTime;
        if (elapsed > kWindowSizeMs) {
            // Reset window - use subtraction to handle overflow correctly
            windowStartTime = currentMillis - (elapsed % kWindowSizeMs);
        }
        // Compare output (0 to kWindowSizeMs * powerRatio) with elapsed time in window
        // Both values are now properly bounded to avoid issues
        elapsed = currentMillis - windowStartTime;  // Recalculate after potential reset
        if (elapsed < static_cast<unsigned long>(controller.output)) {
            digitalWrite(PIN_SSR, HIGH);
        } else {
            digitalWrite(PIN_SSR, LOW);
        }
    } else {
        digitalWrite(PIN_SSR, LOW);
        controller.output = 0;
        if (!controller.isRunning) {
            safety.stop();  // Stop safety monitoring when not running
        }
    }

    // 3. Telegram (Only if connected to Internet)
    if (WiFi.status() == WL_CONNECTED && (currentMillis - lastTelegramCheck > kTelegramIntervalMs)) {
        lastTelegramCheck = currentMillis;
        handleTelegram(); 
    }
    
    // 4. WiFi Reconnection Attempt (if disconnected)
    if (WiFi.status() != WL_CONNECTED) {
        tryWiFiReconnect();
    }

    // 5. Update Display
    if (currentMillis - lastDisplayUpdate > kDisplayIntervalMs) {
        lastDisplayUpdate = currentMillis;
        
        // Build status message efficiently
        static char displayStatus[24];
        if (!checkSensorPresent()) {
            strcpy(displayStatus, "NO SENSOR!");
        } else if (s.emergencyStop) {
            snprintf(displayStatus, sizeof(displayStatus), "ERR: %.12s", s.reason.c_str());
        } else if (controller.isRunning) {
            strcpy(displayStatus, "RUNNING");
        } else {
            strcpy(displayStatus, "IDLE");
        }
        
        bool heaterOn = (controller.output > 0) && controller.isRunning && checkSensorPresent();
        IPAddress ip = WiFi.localIP();
        if (WiFi.getMode() == WIFI_AP) {
            ip = WiFi.softAPIP();
        }
        display.update(s.currentTemp, controller.setpoint, controller.isRunning, heaterOn, displayStatus, ip);
    }
}
