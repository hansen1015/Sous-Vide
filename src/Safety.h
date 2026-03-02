#ifndef SAFETY_H
#define SAFETY_H

#include <Arduino.h>
#include <cmath>

// Safety thresholds (based on food safety guidelines and hardware limits)
static constexpr float kMaxTemperatureSlopePerSecond = 0.6f;    // °C/s - dry run detection
static constexpr float kAbsoluteMaxTemperatureCelsius = 90.0f;  // °C - safety margin below boiling
static constexpr float kAbsoluteMinTemperatureCelsius = -50.0f; // °C - sensor error threshold
static constexpr unsigned long kMaxRuntimeMilliseconds = 24UL * 3600UL * 1000UL; // 24 hours
static constexpr float kDryRunSlopeCheckThreshold = 40.0f;      // °C - only check slope above this
static constexpr unsigned long kMinCheckIntervalMs = 2000UL;    // Minimum time between slope checks

// Error Codes
static constexpr int kErrSensorMismatch = 1;
static constexpr int kErrOverTemp = 2;
static constexpr int kErrDryRun = 3;
static constexpr int kErrTimeout = 4;
static constexpr int kErrUnderTemp = 5;
static constexpr int kErrInvalidSensor = 6;

struct SafetyStatus {
    bool emergencyStop;
    String reason;
    int errorCode;
    float currentTemp;
    float temp1;
    
    SafetyStatus() : emergencyStop(false), errorCode(0), currentTemp(0.0f), temp1(0.0f) {}
};

class SafetySystem {
public:
    SafetySystem();
    void loop(float t1);
    SafetyStatus getStatus() const { return _status; }
    void reset(bool hardReset = false);
    bool hasError() const { return _status.emergencyStop; }
    int getErrorCode() const { return _status.errorCode; }
    
    // New methods for proper lifecycle management
    void start();  // Call when cooking starts
    void stop();   // Call when cooking stops

private:
    SafetyStatus _status;
    unsigned long _lastCheckTime;
    float _lastTemp;
    unsigned long _startTime;
    bool _isInitialized;
    bool _isRunning;  // Track if system is actively running
    
    void handleInvalidSensor(float temp);
    void handleOverTemp(float temp);
    void handleUnderTemp(float temp);
    void handleDryRun(float temp, float slope);
    void handleTimeout();
};

#endif
