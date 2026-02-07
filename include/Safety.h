#ifndef SAFETY_H
#define SAFETY_H

#include <Arduino.h>

struct SafetyStatus {
    bool emergencyStop;
    String reason;
    float currentTemp;
    float temp1;
    float temp2;
};

class SafetySystem {
public:
    SafetySystem();
    void loop(float t1, float t2); // Call in main loop with sensor readings
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
    const unsigned long MAX_RUN_TIME = 24 * 3600 * 1000; // 24 hours in ms
};

#endif
