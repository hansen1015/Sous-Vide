#include "Safety.h"

SafetySystem::SafetySystem() 
    : _lastCheckTime(0)
    , _lastTemp(0.0f)
    , _startTime(0)  // Will be set when system starts running
    , _isInitialized(false)
    , _isRunning(false) {
}

void SafetySystem::loop(float t1) {
    unsigned long currentMillis = millis();
    
    // Update current temperature reading
    _status.temp1 = t1;
    _status.currentTemp = t1;
    
    // 0. Invalid Sensor Check (DS18B20 returns -127 or NaN on error)
    if (std::isnan(t1) || t1 < -100.0f) {
        handleInvalidSensor(t1);
        return;
    }
    
    // 1. Absolute Max Temp
    if (t1 > kAbsoluteMaxTemperatureCelsius) {
        handleOverTemp(t1);
        return;
    }
    
    // 2. Absolute Min Temp (Sensor Error Detection)
    if (t1 < kAbsoluteMinTemperatureCelsius) {
        handleUnderTemp(t1);
        return;
    }
    
    // 3. Dry Run Protection (Slope Check)
    // Initialize on first valid reading - MUST have valid temperature first
    if (!_isInitialized) {
        _lastTemp = t1;
        _lastCheckTime = currentMillis;
        _isInitialized = true;
        Serial.println("[SAFETY] Initialized with valid temperature reading");
    } else {
        // Calculate time delta with protection against zero division
        unsigned long timeDiff = currentMillis - _lastCheckTime;
        float dt = timeDiff / 1000.0f;
        
        // Only check slope if enough time has passed AND dt is non-zero (protect against division by zero)
        if (timeDiff >= kMinCheckIntervalMs && dt > 0.001f) {
            float slope = (t1 - _lastTemp) / dt;
            if (slope > kMaxTemperatureSlopePerSecond && t1 > kDryRunSlopeCheckThreshold) {
                handleDryRun(t1, slope);
                return;
            }
            _lastTemp = t1;
            _lastCheckTime = currentMillis;
        }
    }
    
    // 4. Watchdog Timer (Max Run Time) - only check when running
    if (_isRunning && _startTime > 0) {
        // Handle millis() overflow gracefully
        // When millis() overflows, currentMillis becomes small while _startTime is large
        // The subtraction still works correctly due to unsigned arithmetic wraparound
        unsigned long elapsed = currentMillis - _startTime;
        if (elapsed > kMaxRuntimeMilliseconds) {
            handleTimeout();
            return;
        }
    }
    
    // Clear any previous error if all checks pass
    if (_status.emergencyStop) {
        _status.emergencyStop = false;
        _status.reason = "";
        _status.errorCode = 0;
    }
}

void SafetySystem::handleInvalidSensor(float temp) {
    _status.emergencyStop = true;
    _status.reason = "Invalid Sensor Reading";
    _status.errorCode = kErrInvalidSensor;
    _status.currentTemp = 0.0f;
    Serial.printf("[SAFETY] Invalid sensor: %.1f\n", temp);
}

void SafetySystem::handleOverTemp(float temp) {
    _status.emergencyStop = true;
    _status.reason = "Over Temperature > 90C";
    _status.errorCode = kErrOverTemp;
    Serial.printf("[SAFETY] Over temp: %.1f°C\n", temp);
}

void SafetySystem::handleUnderTemp(float temp) {
    _status.emergencyStop = true;
    _status.reason = "Under Temperature < -50C";
    _status.errorCode = kErrUnderTemp;
    Serial.printf("[SAFETY] Under temp: %.1f°C\n", temp);
}

void SafetySystem::handleDryRun(float temp, float slope) {
    _status.emergencyStop = true;
    _status.reason = "Dry Run Detected (Slope > 0.6C/s)";
    _status.errorCode = kErrDryRun;
    Serial.printf("[SAFETY] Dry run: %.2f°C/s at %.1f°C\n", slope, temp);
}

void SafetySystem::handleTimeout() {
    _status.emergencyStop = true;
    _status.reason = "24H Timeout Reached";
    _status.errorCode = kErrTimeout;
    Serial.println("[SAFETY] 24-hour timeout reached");
}

void SafetySystem::reset(bool hardReset) {
    _status.emergencyStop = false;
    _status.reason = "";
    _status.errorCode = 0;
    if (hardReset) {
        _startTime = millis();
        _isInitialized = false;
        _lastTemp = 0.0f;
        _lastCheckTime = 0;
        _isRunning = true;  // Mark as running when hard reset
    }
    Serial.println("[SAFETY] System reset");
}

void SafetySystem::start() {
    if (!_isRunning) {
        _startTime = millis();
        _isRunning = true;
        _isInitialized = false;  // Re-initialize slope detection
        Serial.println("[SAFETY] System started");
    }
}

void SafetySystem::stop() {
    _isRunning = false;
    Serial.println("[SAFETY] System stopped");
}
