#ifndef SAFETY_H
#define SAFETY_H

#include <Arduino.h>

struct SafetyStatus {
    bool emergencyStop;
    String reason;
    int errorCode;  // Error code for diagnostics
    float currentTemp;
    float temp1;  // Single sensor temperature
};

class SafetySystem {
public:
    SafetySystem();
    void loop(float t1); // Call in main loop with single sensor reading
    SafetyStatus getStatus();
    void reset();

private:
    SafetyStatus _status;
    unsigned long _lastCheckTime;
    float _lastTemp;
    unsigned long _startTime;
    
    // Constants
    const float MAX_TEMP_DIFF = 3.0; // deg C
    const float MAX_SLOPE = 0.6; // deg C per sec
    const float ABSOLUTE_MAX_TEMP = 90.0; // deg C
    const float ABSOLUTE_MIN_TEMP = -50.0; // deg C
    const unsigned long MAX_RUN_TIME = 24 * 3600 * 1000; // 24 hours in ms
    
    // Error Codes
    const int ERR_SENSOR_MISMATCH = 1;  // E01: Sensor readings differ > 3°C
    const int ERR_OVER_TEMP = 2;         // E02: Temperature > 90°C
    const int ERR_DRY_RUN = 3;           // E03: Rapid temperature rise (dry run)
    const int ERR_TIMEOUT = 4;           // E04: 24h runtime exceeded
    const int ERR_UNDER_TEMP = 5;        // E05: Temperature < -50°C (invalid sensor)
    const int ERR_INVALID_SENSOR = 6;    // E06: Sensor reading is NaN or -127°C
};

#endif
