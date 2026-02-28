#include "Safety.h"

SafetySystem::SafetySystem() {
    _status.emergencyStop = false;
    _status.reason = "";
    _status.errorCode = 0;
    _status.currentTemp = 0.0;
    _status.temp1 = 0.0;
    _lastCheckTime = millis();
    _lastTemp = 0.0;
    _startTime = millis();
}

void SafetySystem::loop(float t1) {
    unsigned long currentMillis = millis();
    float dt = (currentMillis - _lastCheckTime) / 1000.0; // Time delta in seconds

    // Update internal state
    _status.temp1 = t1;
    
    // 0. Invalid Sensor Check (DS18B20 returns -127 or NaN on error)
    if (isnan(t1) || t1 < -100) {
        _status.emergencyStop = true;
        _status.reason = "Invalid Sensor Reading";
        _status.errorCode = ERR_INVALID_SENSOR;
        _status.currentTemp = 0.0;
        return;
    }
    
    // Use single sensor temp for safety checks
    _status.currentTemp = t1;

    // 1. Absolute Max Temp
    if (t1 > ABSOLUTE_MAX_TEMP) {
        _status.emergencyStop = true;
        _status.reason = "Over Temperature > 90C";
        _status.errorCode = ERR_OVER_TEMP;
        return;
    }
    
    // 2. Absolute Min Temp (Sensor Error Detection)
    if (t1 < ABSOLUTE_MIN_TEMP) {
        _status.emergencyStop = true;
        _status.reason = "Under Temperature < -50C";
        _status.errorCode = ERR_UNDER_TEMP;
        return;
    }

    // 3. Dry Run Protection (Slope Check)
    // Only check if we have a valid previous temp and minimal time has passed (e.g., 2s) to avoid noise
    if (dt > 2.0) {
        float slope = (t1 - _lastTemp) / dt;
        if (slope > MAX_SLOPE && t1 > 40.0) { // Only checking slope if above 40C to avoid false positives at start
             _status.emergencyStop = true;
             _status.reason = "Dry Run Detected (Slope > 0.6C/s)";
             _status.errorCode = ERR_DRY_RUN;
             return;
        }
        _lastTemp = t1;
        _lastCheckTime = currentMillis;
    }

    // 4. Watchdog Timer (Max Run Time)
    if (currentMillis - _startTime > MAX_RUN_TIME) {
        _status.emergencyStop = true;
        _status.reason = "24H Timeout Reached";
        _status.errorCode = ERR_TIMEOUT;
        return;
    }
}

SafetyStatus SafetySystem::getStatus() {
    return _status;
}

void SafetySystem::reset() {
    _status.emergencyStop = false;
    _status.reason = "";
    _startTime = millis(); // Reset timer? Or maybe just stop flag. Let's restart timer.
}
