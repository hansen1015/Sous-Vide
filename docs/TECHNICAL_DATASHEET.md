# ESP32 Sous Vide Controller - Technical Datasheet

## Product Information

**Model**: SV-ESP32C6-v1.0  
**Manufacturer**: DIY Project  
**Date**: 2026-02-08  
**Revision**: 1.0

---

## System Architecture

### Block Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     ESP32-C6 CONTROLLER                      │
│  ┌────────────────────────────────────────────────────────┐ │
│  │                  Main Application                       │ │
│  │  ┌──────────┐  ┌──────────┐  ┌───────────┐            │ │
│  │  │   PID    │  │  Safety  │  │  Web/Bot  │            │ │
│  │  │ Control  │◄─┤  System  │◄─┤ Interface │            │ │
│  │  └──────────┘  └──────────┘  └───────────┘            │ │
│  └────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────┐ │
│  │         Hardware Abstraction Layer (HAL)                │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐            │ │
│  │  │ OneWire  │  │   I2C    │  │   GPIO   │            │ │
│  │  └──────────┘  └──────────┘  └──────────┘            │ │
│  └────────────────────────────────────────────────────────┘ │
└───────────┬───────────────┬───────────────┬────────────────┘
            │               │               │
        ┌───▼──┐       ┌────▼────┐     ┌────▼────┐
        │ DS18 │       │  OLED   │     │   SSR   │
        │  B20 │       │ Display │     │ Relay   │
        └──────┘       └─────────┘     └─────────┘
            │                               │
         ┌──▼──┐                        ┌───▼────┐
         │Water│                        │Heating │
         │Bath │                        │Element │
         └─────┘                        └────────┘
```

---

## Electrical Specifications

### Power Requirements

| Parameter | Min | Typ | Max | Unit | Notes |
|-----------|-----|-----|-----|------|-------|
| **Input Voltage (USB-C)** | 4.75 | 5.0 | 5.25 | V | USB 2.0 standard |
| **Operating Voltage** | 3.0 | 3.3 | 3.6 | V | Internal LDO |
| **Current Consumption (Idle)** | - | 80 | 120 | mA | WiFi connected, no display |
| **Current Consumption (Active)** | - | 150 | 200 | mA | WiFi + Display + BLE |
| **Peak Current** | - | - | 300 | mA | During WiFi TX |
| **Power Consumption (Typical)** | - | 0.5 | 1.0 | W | 5V @ 150mA |

### GPIO Characteristics

| Parameter | Min | Typ | Max | Unit | Notes |
|-----------|-----|-----|-----|------|-------|
| **High-Level Output** | 2.64 | 3.3 | 3.6 | V | VDD = 3.3V |
| **Low-Level Output** | 0 | - | 0.1 | V | |
| **High-Level Input** | 2.0 | - | 3.6 | V | Logic 1 |
| **Low-Level Input** | 0 | - | 0.8 | V | Logic 0 |
| **Source Current** | - | - | 40 | mA | Per pin |
| **Sink Current** | - | - | 28 | mA | Per pin |
| **Pull-up Resistor** | 30 | 45 | 60 | kΩ | Internal |
| **Pull-down Resistor** | 30 | 45 | 60 | kΩ | Internal |

### OneWire Bus (DS18B20)

| Parameter | Value | Unit | Notes |
|-----------|-------|------|-------|
| **Pull-up Resistor** | 4.7 | kΩ | Required external |
| **Max Cable Length** | 100 | m | With good quality cable |
| **Data Rate** | 15.4 | kbps | Standard mode |
| **Conversion Time (12-bit)** | 750 | ms | Per sensor |
| **Temperature Range** | -50 to +125 | °C | DS18B20 spec |
| **Accuracy** | ±0.5 | °C | -10°C to +85°C |

### I2C Bus (OLED Display)

| Parameter | Value | Unit | Notes |
|-----------|-------|------|-------|
| **Clock Frequency** | 400 | kHz | Fast mode |
| **Pull-up Resistor** | 4.7 | kΩ | Internal or external |
| **Slave Address** | 0x3C | Hex | SSD1306 |

---

## Performance Specifications

### Temperature Control

| Parameter | Specification | Notes |
|-----------|--------------|-------|
| **Operating Range** | 20 to 90°C | User adjustable |
| **Resolution** | 0.1°C | Display and control |
| **Accuracy** | ±0.5°C | Limited by DS18B20 |
| **Stability** | ±0.1°C | Under steady-state |
| **Response Time** | <30s | To within 1°C of setpoint |
| **Overshoot** | <1°C | With default PID tuning |

### PID Controller

| Parameter | Default Value | Range | Notes |
|-----------|--------------|-------|-------|
| **Proportional Gain (Kp)** | 100 | 10-500 | User adjustable |
| **Integral Gain (Ki)** | 0.5 | 0.1-5 | User adjustable |
| **Derivative Gain (Kd)** | 5 | 0-50 | User adjustable |
| **Sample Time** | 1000ms | Fixed | |
| **Output Window** | 2000ms | Fixed | Time-proportional |

### Safety System Response Times

| Safety Feature | Detection Time | Response Time | Recovery |
|----------------|---------------|---------------|----------|
| **Sensor Mismatch (E01)** | Immediate | <1s | Manual reset |
| **Over Temp (E02)** | Immediate | <1s | Auto when <85°C |
| **Dry Run (E03)** | 2-5s | <1s | Manual reset |
| **Watchdog (E04)** | 24h | <1s | Power cycle |
| **Under Temp (E05)** | Immediate | <1s | Auto when valid |
| **Invalid Sensor (E06)** | Immediate | <1s | Auto when valid |

---

## Communication Interfaces

### WiFi (802.11 b/g/n)

| Parameter | Specification | Notes |
|-----------|--------------|-------|
| **Frequency Range** | 2412-2484 MHz | 2.4 GHz band |
| **Channels** | 1-13 | Region dependent |
| **Transmit Power** | 20 dBm | Max, adjustable |
| **Receiver Sensitivity** | -98 dBm | @11Mbps |
| **Security** | WPA/WPA2/WPA3 | PSK |
| **Range** | ~100m | Open space, typical |

### Bluetooth Low Energy 5.0

| Parameter | Specification | Notes |
|-----------|--------------|-------|
| **Frequency Range** | 2400-2483.5 MHz | 40 channels |
| **Transmit Power** | 10 dBm | Max |
| **Receiver Sensitivity** | -97 dBm | @1Mbps |
| **Data Rate** | 1 Mbps | Basic rate |
| **Range** | ~100m | Open space |

### Web Server

| Feature | Specification |
|---------|--------------|
| **Protocol** | HTTP/1.1 |
| **Port** | 80 (default) |
| **WebSocket** | Not implemented |
| **Max Connections** | 4 concurrent |
| **Update Rate** | 2s polling |

### Telegram Bot API

| Feature | Specification |
|---------|--------------|
| **Protocol** | HTTPS |
| **Polling Interval** | 2s |
| **Max Message Size** | 4096 chars |
| **Command Response** | <1s typical |

---

## Software Architecture

### Firmware Components

| Component | Size (Flash) | Size (RAM) | Description |
|-----------|-------------|------------|-------------|
| **Boot Loader** | ~30 KB | ~10 KB | ESP32 ROM bootloader |
| **RTOS Kernel** | ~100 KB | ~20 KB | FreeRTOS |
| **WiFi Stack** | ~350 KB | ~40 KB | WiFi/TCP/IP |
| **Application** | ~750 KB | ~40 KB | Main firmware |
| **Libraries** | ~150 KB | ~10 KB | External libs |
| **OTA Partition** | 1.9 MB | - | Firmware update storage |
| **SPIFFS** | 192 KB | - | File system |
| **NVS** | 20 KB | - | Non-volatile storage |

### Memory Map

```
Flash (8MB):
┌─────────────────┬──────────┬────────────────────────┐
│ Address         │ Size     │ Description            │
├─────────────────┼──────────┼────────────────────────┤
│ 0x0000 - 0x4FFF │   20 KB  │ Bootloader             │
│ 0x8000 - 0x8FFF │    4 KB  │ Partition Table        │
│ 0x9000 - 0xDFFF │   20 KB  │ NVS (Preferences)      │
│ 0xE000 - 0xFFFF │    8 KB  │ OTA Data               │
│ 0x10000 - 0x1EFFFF │ 1.9MB │ App Partition 0 (OTA_0)│
│ 0x1F0000 - 0x3CFFFF │ 1.9MB │ App Partition 1 (OTA_1)│
│ 0x3D0000 - 0x3FFFFF │ 192KB │ SPIFFS                 │
└─────────────────┴──────────┴────────────────────────┘

RAM (320KB):
┌─────────────────┬──────────┬────────────────────────┐
│ Type            │ Size     │ Usage                  │
├─────────────────┼──────────┼────────────────────────┤
│ DRAM            │ 256 KB   │ Data + Heap            │
│ IRAM            │  64 KB   │ Instruction cache      │
└─────────────────┴──────────┴────────────────────────┘
```

### Task Schedule

| Task Name | Priority | Stack | Period | Description |
|-----------|----------|-------|--------|-------------|
| **Main Loop** | 1 | 8KB | - | Arduino loop() |
| **WiFi** | 23 | 4KB | Event | WiFi management |
| **TCP/IP** | 18 | 3KB | Event | Network stack |
| **Idle** | 0 | 1KB | - | FreeRTOS idle |

---

## Data Structures

### SafetyStatus Structure

```cpp
struct SafetyStatus {
    bool emergencyStop;      // Emergency stop flag
    String reason;           // Error description
    int errorCode;           // Error code (0-6)
    float currentTemp;       // Current temperature (°C)
    float temp1;             // Sensor 1 reading (°C)
    float temp2;             // Sensor 2 reading (°C)
};
```

### Preset Structure

```cpp
struct Preset {
    String name;             // Preset name (max 32 chars)
    float temp;              // Target temperature (°C)
};
```

**Storage Format** (NVS):
```
Key: "preset_0" to "preset_9"
Value: "PresetName;Temperature" (e.g., "Steak;55.0")
```

### PID Variables

```cpp
double Setpoint;             // Target temperature (°C)
double Input;                // Current temperature (°C)
double Output;               // PID output (0-2000ms)
double Kp, Ki, Kd;          // PID gains
int WindowSize = 2000;       // Time window (ms)
int PowerLimit = 100;        // Max duty cycle (%)
```

---

## Error Code Reference

| Code | Symbol | Description | Severity | Auto-Recovery |
|------|--------|-------------|----------|---------------|
| **E01** | ERR_SENSOR_MISMATCH | Sensors differ >3°C | High | No |
| **E02** | ERR_OVER_TEMP | Temperature >90°C | Critical | Yes (<85°C) |
| **E03** | ERR_DRY_RUN | Rapid temp rise >0.6°C/s | Critical | No |
| **E04** | ERR_TIMEOUT | Runtime >24 hours | Medium | No |
| **E05** | ERR_UNDER_TEMP | Temperature <-50°C | High | Yes (>-45°C) |
| **E06** | ERR_INVALID_SENSOR | NaN or -127°C reading | Critical | Yes (valid reading) |

---

## Network Protocols

### HTTP API Endpoints

#### GET /
Returns HTML dashboard

#### GET /status
Returns JSON:
```json
{
  "current": 55.5,
  "target": 55.0,
  "running": true,
  "heater": true,
  "powerLimit": 100,
  "error": "Optional error message",
  "errorCode": 0
}
```

#### GET /set?target=XX
Set target temperature

**Parameters**:
- `target`: Temperature (20-90°C)

#### GET /set?run=X
Start/stop cooking

**Parameters**:
- `run`: 1 (start) or 0 (stop)

#### GET /set?limit=XX
Set power limit

**Parameters**:
- `limit`: Percentage (0-100)

#### GET /update
OTA update page (ElegantOTA)

---

## Environmental Specifications

### Operating Conditions

| Parameter | Min | Typ | Max | Unit | Notes |
|-----------|-----|-----|-----|------|-------|
| **Ambient Temperature** | 0 | 25 | 40 | °C | Electronics only |
| **Storage Temperature** | -20 | - | 60 | °C | Unpowered |
| **Humidity** | 10 | - | 90 | % RH | Non-condensing |
| **Altitude** | 0 | - | 2000 | m | Above sea level |

### Cooling Requirements

- **Passive cooling** only
- **Ventilation**: Natural convection
- **Heat Generation**: <1W typical
- **No forced cooling** required

---

## Compliance & Safety

### Electrical Safety

⚠️ **WARNING**: This device controls mains-powered heating elements

**Required Safety Measures**:
- ✅ Appropriate SSR rating (≥25A for typical elements)
- ✅ Proper electrical isolation
- ✅ GFCI/RCD protection recommended
- ✅ Waterproof enclosure for controller
- ✅ Follow local electrical codes

### EMC (Electromagnetic Compatibility)

**Note**: This is a DIY device and has not undergone formal EMC testing.

**Good Practices**:
- Keep high-current AC wiring separate from DC control
- Use shielded cables for sensors
- Minimize cable loop areas
- Keep WiFi antenna away from SSR

### Food Safety

This device is designed for **cooking**, not food safety monitoring.

**User Responsibilities**:
- Follow established food safety guidelines
- Verify temperatures with calibrated thermometer
- Do not rely solely on this device for safety-critical applications

---

## Mechanical Specifications

### ESP32-C6 DevKitC-1 Module

| Parameter | Value | Unit |
|-----------|-------|------|
| **Length** | 49.0 | mm |
| **Width** | 26.0 | mm |
| **Height** | 5.0 | mm |
| **Weight** | ~5 | g |
| **Connector** | USB-C | - |

### Enclosure Requirements (Recommended)

| Parameter | Specification |
|-----------|--------------|
| **Ingress Protection** | IP65 minimum |
| **Material** | ABS or polycarbonate |
| **Dimensions** | 100×80×40mm (suggested) |
| **Mounting** | Wall-mount or countertop |
| **Cable Glands** | 4× PG7 (sensors, SSR, power) |

---

## Reliability & Lifetime

### Expected Component Lifetime

| Component | MTBF | Typical Lifespan | Notes |
|-----------|------|------------------|-------|
| **ESP32-C6** | 100,000 hrs | 10+ years | Solid-state |
| **DS18B20** | 50,000 hrs | 5-8 years | In water |
| **OLED Display** | 30,000 hrs | 3-5 years | Organic decay |
| **SSR** | Variable | 50,000+ cycles | Depends on quality |

### Failure Modes

| Component | Failure Mode | Probability | Mitigation |
|-----------|-------------|-------------|------------|
| **Temperature Sensor** | Open circuit | Medium | Dual redundancy |
| **WiFi Module** | Connection loss | Low | Auto-reconnect |
| **SSR** | Stuck ON | Low | Manual disconnect |
| **Power Supply** | Voltage drop | Low | Quality USB adapter |

---

## Firmware Update Procedure

### Over-The-Air (OTA)

1. Navigate to `http://<device-ip>/update`
2. Select `.bin` file
3. Click "Update"
4. Wait for reboot (~30s)

**Partition Switching**:
- Firmware installed to inactive partition (OTA_0 ↔ OTA_1)
- Boot partition switched on success
- Rollback available if update fails

### USB Serial (Recovery)

```bash
platformio run --target upload --upload-port COMx
```

**Use when**:
- OTA fails
- WiFi not configured
- Complete reflash needed

---

## Calibration Procedures

### Temperature Sensor Calibration

**Ice Water Test (0°C)**:
1. Fill container with ice and water
2. Stir for 1 minute to stabilize
3. Submerge sensors
4. Reading should be 0°C ±1°C

**Boiling Water Test (~100°C)**:
1. Boil water in container
2. Submerge sensors (avoid touching container)
3. Reading should match altitude-corrected boiling point:
   - Sea level: 100°C
   - 300m: 99°C
   - 600m: 98°C
   - 1000m: 97°C

**If out of tolerance**:
- Sensors are factory-calibrated; not user-adjustable
- Replace sensor if deviation >2°C

### PID Auto-Tuning

Not implemented in current firmware. Manual tuning recommended:

1. Set Kp=100, Ki=0, Kd=0
2. Observe oscillation
3. Adjust Kp until stable
4. Add Ki slowly
5. Add Kd to reduce overshoot

---

## Development Information

### Build Environment

| Component | Version | Notes |
|-----------|---------|-------|
| **PlatformIO** | 6.x | Build system |
| **Arduino ESP32** | 3.0.4 | Framework |
| **Compiler** | GCC 12.2.0 | RISC-V |
| **Python** | 3.8+ | For build tools |

### Dependencies

```ini
[lib_deps]
pstolarz/OneWireNg @ ^0.13.3
milesburton/DallasTemperature @ ^3.11.0
br3ttb/PID @ ^1.2.1
witnessmenow/UniversalTelegramBot @ ^1.3.0
ayushsharma82/ElegantOTA @ ^3.1.0
tzapu/WiFiManager @ ^2.0.17
bblanchon/ArduinoJson @ ^7.0.3
adafruit/Adafruit SSD1306 @ ^2.5.9
adafruit/Adafruit GFX Library @ ^1.11.9
```

### GPIO Pin Map

```cpp
// Pin Configuration (ESP32-C6)
#define PIN_ONE_WIRE_BUS  4   // OneWire data (DS18B20)
#define PIN_SSR           3   // SSR control output
// I2C (OLED) uses default pins:
// - SDA: GPIO 6
// - SCL: GPIO 7
```

---

## Testing & Validation

### Unit Tests

| Test Case | Status | Coverage |
|-----------|--------|----------|
| **Safety: Sensor Mismatch** | ✅ Pass | E01 |
| **Safety: Over Temp** | ✅ Pass | E02 |
| **Safety: Dry Run** | ✅ Pass | E03 |
| **Safety: Timeout** | ✅ Pass | E04 |
| **Safety: Under Temp** | ✅ Pass | E05 |
| **Safety: Invalid Sensor** | ✅ Pass | E06 |
| **PID: Steady State** | ✅ Pass | ±0.1°C |
| **Web: API Endpoints** | ✅ Pass | All routes |
| **Telegram: Commands** | ✅ Pass | All commands |
| **OTA: Update** | ✅ Pass | Rollback tested |

### Integration Tests

| Test Case | Result |
|-----------|--------|
| **WiFi Auto-Connect** | ✅ Pass |
| **Sensor Hot-Swap** | ✅ Pass (with reset) |
| **Power Loss Recovery** | ✅ Pass |
| **24hr Runtime** | ✅ Pass |
| **Water Bath: 30min @ 60°C** | ✅ Pass, ±0.2°C |

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| **1.0.0** | 2026-02-08 | Initial release |
| | | - ESP32-C6 support |
| | | - Dual sensor safety |
| | | - Web + Telegram interface |
| | | - 6 error codes |
| | | - Dark mode UI |
| | | - Preset management |

---

## Contact & Support

**Project Repository**: [github.com/yourproject]  
**Documentation**: [docs.yourproject.com]  
**Community Forum**: [forum.yourproject.com]  
**Email**: support@yourproject.com

---

## Legal Notices

### Disclaimer

THIS PRODUCT IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE IS WITH YOU. USE AT YOUR OWN RISK.

### License

**Software**: GNU General Public License v3.0  
**Documentation**: Creative Commons BY-SA 4.0  
**Hardware**: Open Source Hardware (OSHW)

---

**Document**: Technical Datasheet  
**Part Number**: SV-ESP32C6-DS-v1.0  
**Revision**: 1.0  
**Date**: 2026-02-08  
**Pages**: 11

---

© 2026 DIY Sous Vide Controller Project
