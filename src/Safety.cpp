#include "Safety.h"

SafetySystem::SafetySystem() {
    _status.emergencyStop = false;
    _status.reason = "";
    _status.currentTemp = 0.0;
    _status.temp1 = 0.0;
    _status.temp2 = 0.0;
    _lastCheckTime = millis();
    _lastTemp = 0.0;
    _startTime = millis();
}

void SafetySystem::loop(float t1, float t2) {
    unsigned long currentMillis = millis();
    float dt = (currentMillis - _lastCheckTime) / 1000.0; // Time delta in seconds

    // Update internal state
    _status.temp1 = t1;
    _status.temp2 = t2;
    // Use highest temp for safety checks (conservative approach)
    float maxCurrentTemp = max(t1, t2);
    _status.currentTemp = maxCurrentTemp;

    // 1. Sensor Redundancy Check
    if (abs(t1 - t2) > MAX_TEMP_DIFF) {
        _status.emergencyStop = true;
        _status.reason = "Sensor Mismatch > 3C";
        return;
    }

    // 2. Absolute Max Temp
    if (maxCurrentTemp > ABSOLUTE_MAX_TEMP) {
        _status.emergencyStop = true;
        _status.reason = "Over Temperature > 90C";
        return;
    }

    // 3. Dry Run Protection (Slope Check)
    // Only check if we have a valid previous temp and minimal time has passed (e.g., 2s) to avoid noise
    if (dt > 2.0) {
        float slope = (maxCurrentTemp - _lastTemp) / dt;
        if (slope > MAX_SLOPE && maxCurrentTemp > 40.0) { // Only checking slope if above 40C to avoid false positives at start
             _status.emergencyStop = true;
             _status.reason = "Dry Run Detected (Slope > 0.6C/s)";
             return;
        }
        _lastTemp = maxCurrentTemp; 
        _lastCheckTime = currentMillis;
    }

    // 4. Watchdog Timer (Max Run Time)
    if (currentMillis - _startTime > MAX_RUN_TIME) {
        _status.emergencyStop = true;
        _status.reason = "24H Timeout Reached";
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
