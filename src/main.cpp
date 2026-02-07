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

// --- PIN DEFINITIONS ---
#define PIN_ONE_WIRE_BUS 4
#define PIN_SSR 15

// --- OBJECTS ---
OneWire oneWire(PIN_ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1, sensor2;

SafetySystem safety;
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
unsigned long lastTelegramCheck = 0;
const int telegramInterval = 2000; // Check every 2s (avoids rate limit)

// SSR Time Proportional Control
unsigned long windowStartTime;
const int WindowSize = 2000; // 2000ms window for SSR PWM

// --- FUNCTIONS ---

void handleRoot() {
    server.send_P(200, "text/html", index_html);
}

void handleStatus() {
    SafetyStatus status = safety.getStatus();
    JsonDocument doc;
    doc["current"] = status.currentTemp;
    doc["target"] = Setpoint;
    doc["running"] = isRunning;
    doc["heater"] = (Output > 0); // Is PID outputting?
    if (status.emergencyStop) {
        doc["error"] = status.reason;
    }
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void handleSet() {
    if (server.hasArg("target")) {
        Setpoint = server.arg("target").toDouble();
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
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
        for (int i = 0; i < numNewMessages; i++) {
            String chat_id = String(bot.messages[i].chat_id);
            String text = bot.messages[i].text;

            // Simple Auth (Save Chat ID)
            if (text == "/start") {
                // In a real app, require a password. 
                // Here we just save the ID if not saved.
                preferences.putString("chat_id", chat_id);
                bot.sendMessage(chat_id, "Welcome to Sous Vide! Commands: /status, /start_cook, /stop, /set 55", "");
            }
            
            if (text == "/status") {
                SafetyStatus s = safety.getStatus();
                String msg = "Temp: " + String(s.currentTemp, 1) + "C\n";
                msg += "Target: " + String(Setpoint, 1) + "C\n";
                msg += "State: " + String(isRunning ? "RUNNING" : "IDLE");
                if (s.emergencyStop) msg += "\nERROR: " + s.reason;
                bot.sendMessage(chat_id, msg, "");
            }
            
            if (text == "/stop") {
                isRunning = false;
                bot.sendMessage(chat_id, "Stopped.", "");
            }
            
             // Rudimentary parsing for "/set 60"
            if (text.startsWith("/set ")) {
                float t = text.substring(5).toFloat();
                if (t > 20 && t < 90) {
                    Setpoint = t;
                    bot.sendMessage(chat_id, "Target set to " + String(Setpoint, 1), "");
                }
            }
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
}

void setup() {
    Serial.begin(115200);
    preferences.begin("sousvide", false); // Namespace "sousvide"
    
    pinMode(PIN_SSR, OUTPUT);
    digitalWrite(PIN_SSR, LOW);

    // Sensors
    sensors.begin();
    if(sensors.getDeviceCount() < 2) Serial.println("WARNING: < 2 Sensors found!");
    sensors.getAddress(sensor1, 0);
    sensors.getAddress(sensor2, 1);

    // WiFi via WiFiManager
    WiFiManager wifiManager;
    // Tries to connect to last known settings.
    // If it fails, it starts an AP "SousVide_Config_AP" for calibration/setup.
    // We set a timeout (e.g., 180s) so if no one configures it, we can fall back to 
    // a distinct "Offline Mode" AP where the control interface lives.
    wifiManager.setConfigPortalTimeout(180); 

    if (!wifiManager.autoConnect("SousVide_Config_AP")) {
        Serial.println("WiFi Manager Timeout - Starting Offline AP for Control");
        WiFi.softAP("SousVide_Offline");
        // In this mode, we have NO Internet, so Telegram won't work.
    } else {
        Serial.println("WiFi Connected");
    }

    // Telegram Cert
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
    myPID.SetOutputLimits(0, WindowSize); // 0 to 2000ms
    myPID.SetMode(AUTOMATIC);
    windowStartTime = millis();
}

unsigned long lastTempRequest = 0;
const int tempInterval = 1000; // Read every 1s
const int conversionDelay = 750; // 12-bit
bool isConversionStarted = false;

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
        float t2 = sensors.getTempC(sensor2);
        
        safety.loop(t1, t2);
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

}
