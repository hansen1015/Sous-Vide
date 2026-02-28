# ESP32 Sous Vide Controller - User Manual

## Table of Contents
1. [Product Overview](#product-overview)
2. [Technical Specifications](#technical-specifications)
3. [Safety Information](#safety-information)
4. [Hardware Setup](#hardware-setup)
5. [Initial Configuration](#initial-configuration)
6. [Web Interface Guide](#web-interface-guide)
7. [Telegram Bot Guide](#telegram-bot-guide)
8. [Error Codes](#error-codes)
9. [Troubleshooting](#troubleshooting)
10. [Maintenance](#maintenance)

---

## Product Overview

The ESP32 Sous Vide Controller is an IoT-enabled precision temperature controller designed for sous vide cooking. It features dual-sensor redundancy, web and Telegram bot interfaces, and comprehensive safety systems.

### Key Features
- 🌡️ **Precision Control**: PID temperature control accurate to ±0.1°C
- 🔐 **Safety First**: 6-layer safety system with real-time monitoring
- 📱 **Dual Interface**: Web dashboard and Telegram bot control
- 📊 **Real-time Monitoring**: Live temperature graphing and status updates
- 🔄 **OTA Updates**: Wireless firmware updates
- 💾 **Custom Presets**: Save and manage cooking presets
- 📱 **OLED Display**: Local status display (128x64)

---

## Technical Specifications

### Hardware
| Component | Specification |
|-----------|--------------|
| **Microcontroller** | ESP32-C6 (RISC-V, 160MHz) |
| **RAM** | 320KB |
| **Flash** | 8MB |
| **WiFi** | 2.4GHz 802.11 b/g/n (WiFi 6 capable) |
| **Bluetooth** | BLE 5.0 |
| **Temperature Sensors** | 2× DS18B20 (Dual redundancy) |
| **Display** | SSD1306 OLED (128×64, I2C) |
| **Relay Output** | SSR-compatible GPIO (3.3V logic) |

### Pin Configuration
| Function | GPIO Pin |
|----------|----------|
| **OneWire Bus** | GPIO 4 |
| **SSR Control** | GPIO 3 |
| **I2C SDA** | GPIO 6 |
| **I2C SCL** | GPIO 7 |

### Temperature Range
- **Operating Range**: 20°C to 90°C
- **Sensor Range**: -50°C to +125°C
- **Absolute Max Safety Limit**: 90°C
- **Absolute Min Safety Limit**: -50°C

### Accuracy & Performance
- **Temperature Accuracy**: ±0.5°C (DS18B20 spec)
- **Control Resolution**: 0.1°C
- **Update Rate**: 1 second
- **PID Window**: 2000ms (2 seconds)
- **Max Continuous Runtime**: 24 hours (watchdog reset)

---

## Safety Information

### ⚠️ WARNINGS
- **Electrical Safety**: Use appropriately rated SSR (Solid State Relay) for your heating element
- **Water Safety**: Ensure all electrical components are isolated from water
- **Temperature Limits**: Controller will automatically shut down if temperature exceeds 90°C
- **Sensor Redundancy**: Both temperature sensors must agree within 3°C
- **Dry Run Protection**: Controller detects rapid temperature rise (>0.6°C/s above 40°C)

### Safety System Layers

#### 1. Sensor Redundancy Check (E01)
- Monitors difference between two temperature sensors
- Triggers emergency stop if difference exceeds 3°C
- Ensures accurate temperature readings

#### 2. Over-Temperature Protection (E02)
- Absolute maximum limit: 90°C
- Immediate shutdown if exceeded
- Prevents overheating hazards

#### 3. Dry Run Detection (E03)
- Monitors rate of temperature change
- Triggers if temperature rises >0.6°C per second above 40°C
- Prevents damage from heating without water

#### 4. Watchdog Timer (E04)
- Maximum continuous runtime: 24 hours
- Automatic shutdown after timeout
- Prevents unattended operation

#### 5. Under-Temperature Protection (E05)
- Minimum limit: -50°C
- Detects sensor malfunction
- Invalid readings trigger emergency stop

#### 6. Invalid Sensor Detection (E06)
- Detects NaN or -127°C readings (typical DS18B20 error values)
- Immediate shutdown on sensor failure
- Ensures reliable operation

---

## Hardware Setup

### Required Components
1. **ESP32-C6 DevKitC-1** (or compatible)
2. **2× DS18B20 Temperature Sensors** (waterproof recommended)
3. **4.7kΩ Resistor** (pull-up for OneWire bus)
4. **Solid State Relay (SSR)** - rated for your heating element
5. **128×64 I2C OLED Display** (SSD1306)
6. **Heating Element** - immersion heater or crock pot
7. **Container** - for water bath
8. **Power Supply** - 5V USB-C for ESP32

### Wiring Diagram

```
ESP32-C6 Connections:
┌─────────────────┐
│    ESP32-C6     │
│                 │
│ GPIO 4  ────────┼──── OneWire Data (Yellow) ──┬── Sensor 1
│                 │                              ├── Sensor 2
│ 3.3V    ────────┼──── OneWire VCC (Red)       │
│                 │          │                   │
│ GND     ────────┼──────────┼───────────────────┤
│                 │          │                   │
│            4.7kΩ│          │              GND (Black)
│         Resistor│──────────┘
│                 │
│ GPIO 3  ────────┼──── SSR Control (+)
│ GND     ────────┼──── SSR Control (-)
│                 │
│ GPIO 6  ────────┼──── OLED SDA
│ GPIO 7  ────────┼──── OLED SCL
│ 3.3V    ────────┼──── OLED VCC
│ GND     ────────┼──── OLED GND
└─────────────────┘

SSR Connections:
┌──────────┐
│   SSR    │
│          │
│ +  ──────┼──── ESP32 GPIO 3
│ -  ──────┼──── ESP32 GND
│          │
│ AC1 ─────┼──── Mains Hot (Live)
│ AC2 ─────┼──── Heater Element
│          │
└──────────┘
(Heater returns to Mains Neutral)
```

### Assembly Steps

1. **Prepare Sensors**
   - Verify DS18B20 sensors are waterproof
   - Test sensors with multimeter (should show ~1-2kΩ at room temp)

2. **OneWire Bus**
   - Connect both DS18B20 VCC (red) to ESP32 3.3V
   - Connect both DS18B20 GND (black) to ESP32 GND
   - Connect both DS18B20 Data (yellow) to GPIO 4
   - Connect 4.7kΩ pull-up resistor between Data line and 3.3V

3. **SSR Connection**
   - Connect SSR control (+) to GPIO 3
   - Connect SSR control (-) to ESP32 GND
   - **⚠️ DANGER**: SSR AC side carries mains voltage - ensure proper insulation

4. **OLED Display**
   - Connect SDA to GPIO 6
   - Connect SCL to GPIO 7
   - Connect VCC to 3.3V
   - Connect GND to GND

5. **Power**
   - Connect USB-C cable to ESP32
   - Use quality 5V 2A power supply

---

## Initial Configuration

### 1. Flash Firmware

**Requirements**:
- PlatformIO (VS Code extension or CLI)
- USB cable (data, not charge-only)

**Steps**:
```bash
# Open project in PlatformIO
cd /path/to/Sous_Vide

# Edit Secrets.h
# Set your Telegram Bot Token

# Build and upload
platformio run --target upload
```

### 2. WiFi Setup

1. **Power on the device**
2. **Look for WiFi AP**: `SousVide_Config_AP`
3. **Connect to it** (no password required)
4. **Browser will auto-open** config portal
5. **Select your WiFi network** and enter password
6. **Click Save**
7. Device will reboot and connect to your WiFi

### 3. Find Device IP Address

**Option 1: OLED Display**
- IP address is shown on the display after WiFi connection

**Option 2: Serial Monitor**
```bash
platformio device monitor
```
Look for: `WiFi Connected! IP: xxx.xxx.xxx.xxx`

**Option 3: Router Admin Panel**
- Look for device named "ESP32" or check DHCP leases

### 4. Telegram Bot Setup

1. **Get Bot Token**
   - Open Telegram, search for `@BotFather`
   - Send `/newbot`
   - Follow instructions, copy token
   - Edit `src/Secrets.h` and paste token

2. **Start Bot**
   - Search for your bot in Telegram
   - Send `/start`
   - Bot will save your Chat ID
   - You'll receive welcome message with custom keyboard

---

## Web Interface Guide

### Accessing the Interface
1. Open browser
2. Navigate to `http://<device-ip>`
3. Interface loads automatically

### Interface Overview

#### Status Dashboard
- **Current Temp**: Real-time temperature reading (°C)
- **Target Temp**: Your set temperature goal
- **Status**: Current state (HEATING/IDLE/ERROR)
- **Power Limit**: SSR duty cycle percentage

#### Temperature Graph
- Live plotting of temperature over time
- Last 50 data points shown
- Auto-scrolling X-axis
- Updates every 2 seconds

#### Controls

**Set Temperature**
1. Enter desired temperature (20-90°C)
2. Click "Set Temp"
3. Temperature target updated

**Start/Stop Cooking**
- **START**: Begins PID control loop
- **STOP**: Halts heating, SSR off

**Quick Presets**
- 🥩 **Steak**: 54°C (Medium-rare)
- 🥚 **Eggs**: 63°C (Soft-cooked)
- 🍗 **Chicken**: 60°C (Tender, juicy)
- 🥦 **Vegetables**: 85°C (Al dente)

**Power Limit Slider**
- Adjusts maximum SSR duty cycle (0-100%)
- Useful for:
  - Small water baths (reduce power)
  - Energy saving
  - Preventing overcurrent

#### Error Display
When safety system triggers:
- Red error banner appears
- Shows error description
- Displays error code (E01-E06)
- See [Error Codes](#error-codes) section

#### Firmware Update (OTA)
1. Click "🔧 Firmware Update"
2. Select `.bin` file
3. Click "Update"
4. Wait for reboot (approx. 30 seconds)

---

## Telegram Bot Guide

### Available Commands

#### Basic Control
| Command | Description | Example |
|---------|-------------|---------|
| `/start` | Show welcome message and keyboard | `/start` |
| `/status` | Get current temperature and status | `/status` |
| `/stop` | Stop cooking immediately | `/stop` |
| `/set <temp>` | Set target temperature | `/set 60` |

#### Preset Management
| Command | Description | Example |
|---------|-------------|---------|
| `/presets` | List all saved presets | `/presets` |
| `/addpreset <name> <temp>` | Add new preset | `/addpreset Steak 55` |
| `/delpreset <name>` | Delete preset | `/delpreset Steak` |
| `/cook <name>` | Start cooking with preset | `/cook Steak` |

#### Settings
| Command | Description | Example |
|---------|-------------|---------|
| `/settings` | Show current settings | `/settings` |
| `/setpower <0-100>` | Set power limit | `/setpower 80` |

### Custom Keyboard

The bot provides a custom keyboard for quick access:

**Row 1**: `📊 Status` | `🔴 Stop`
**Row 2**: `📋 Presets` | `⚙️ Settings`
**Row 3**: `🥩 Steak 55°C` | `🐟 Fish 50°C`
**Row 4**: `🥚 Egg 64°C` | `🍗 Chicken 65°C`

Simply tap a button to execute the action!

### Preset Usage Examples

**Create a custom preset**:
```
/addpreset Salmon 50
```
Returns: `✅ Preset Salmon saved at 50.0°C`

**List presets**:
```
/presets
```
Returns:
```
📋 Saved Presets:

🔹 `Salmon` → 50.0°C
🔹 `Steak` → 55.0°C
🔹 `Chicken` → 65.0°C
```

**Cook with preset**:
```
/cook Salmon
```
Returns: `🔥 Cooking Salmon at 50.0°C`

**Delete preset**:
```
/delpreset Salmon
```
Returns: `🗑️ Preset Salmon deleted.`

---

## Error Codes

### E01: Sensor Mismatch
**Description**: Temperature sensors disagree by more than 3°C

**Possible Causes**:
- One sensor is failing
- Sensors in different locations (poor water circulation)
- Electrical interference

**Solution**:
1. Stop cooking immediately
2. Check sensor wiring
3. Verify both sensors are submerged
4. Stir water bath, wait 30 seconds
5. If persists, replace suspect sensor

### E02: Over Temperature
**Description**: Temperature exceeded 90°C

**Possible Causes**:
- Runaway heating
- SSR stuck ON
- PID tuning issue
- Sensor failure reading low

**Solution**:
1. **IMMEDIATELY TURN OFF MAINS POWER**
2. Let water cool
3. Check SSR is functioning correctly
4. Verify sensors are accurate (test in ice water: 0°C)
5. Re-calibrate PID if necessary

### E03: Dry Run Detected
**Description**: Temperature rising >0.6°C/second above 40°C

**Possible Causes**:
- Heating element not submerged in water
- Insufficient water in container
- Element touching container bottom

**Solution**:
1. Stop immediately
2. Turn off power
3. Check water level
4. Ensure element is fully submerged
5. Add water if necessary
6. Restart after verification

### E04: 24H Timeout
**Description**: Continuous runtime exceeded 24 hours

**Possible Causes**:
- Forgot to turn off
- Very long cook time

**Solution**:
1. This is a safety timeout
2. If cooking is complete, stop and remove food
3. If cooking needs to continue:
   - Stop cooking
   - Reset by pressing ESP32 reset button
   - Restart cooking

### E05: Under Temperature
**Description**: Temperature reading below -50°C

**Possible Causes**:
- Sensor disconnected
- Sensor short circuit
- Wiring fault

**Solution**:
1. Check sensor connections
2. Verify pull-up resistor (4.7kΩ)
3. Test sensor resistance
4. Replace faulty sensor

### E06: Invalid Sensor
**Description**: Sensor returning NaN or -127°C (DS18B20 error value)

**Possible Causes**:
- Sensor power failure
- OneWire bus fault
- Sensor internally damaged

**Solution**:
1. Check 3.3V power supply
2. Verify OneWire wiring
3. Test with single sensor (disconnect one)
4. Replace faulty sensor

---

## Troubleshooting

### WiFi Issues

**Problem**: Can't find `SousVide_Config_AP`
- Solution: Wait 180 seconds after power-on, AP starts if WiFi fails

**Problem**: Can't connect to saved WiFi
- Solution: Hold reset button for 10 seconds to clear WiFi credentials

**Problem**: WiFi keeps disconnecting
- Solution:
  - Check signal strength (move closer to router)
  - Verify 2.4GHz band is enabled
  - Check router DHCP pool isn't full

### Telegram Bot Issues

**Problem**: Bot not responding
- Solution:
  - Verify internet connection (WiFi must be connected)
  - Check Bot Token in `Secrets.h`
  - Send `/start` to register your Chat ID

**Problem**: "Unknown command" message
- Solution: Use `/start` to see correct command syntax

### Temperature Control Issues

**Problem**: Temperature oscillates ±2°C
- Solution: Increase PID `Kd` (damping) value in code

**Problem**: Takes too long to reach temperature
- Solution: Increase `Kp` (proportional gain) value

**Problem**: Overshoots target temperature
- Solution: Decrease `Kp` and increase `Kd`

**Current PID Values** (in `main.cpp`):
```cpp
double Kp = 100, Ki = 0.5, Kd = 5;
```

### Display Issues

**Problem**: OLED shows garbage or blank
- Solution:
  - Check I2C address (should be 0x3C)
  - Verify SDA/SCL not swapped
  - Check power (some OLEDs need 5V)

**Problem**: Display freezes
- Solution: I2C bus stalled, reset ESP32

### Heating Issues

**Problem**: SSR not switching
- Solution:
  - Verify SSR polarity (+ to GPIO3, - to GND)
  - Check SSR LED (should blink when heating)
  - Measure GPIO3 voltage (should toggle 0V/3.3V)

**Problem**: Always heating, can't stop
- Solution:
  - **IMMEDIATELY DISCONNECT POWER**
  - SSR may be failed (stuck ON)
  - Replace SSR before reconnecting

---

## Maintenance

### Regular Checks (Before Each Use)

1. **Visual Inspection**
   - Check sensor cables for damage
   - Verify waterproof seals intact
   - Inspect SSR for discoloration/burning

2. **Sensor Calibration Test**
   - Ice water bath: should read 0°C ±1°C
   - Boiling water: should read ~100°C (altitude dependent)

3. **Wiring Check**
   - Ensure connections are tight
   - Look for corrosion on terminals

### Periodic Maintenance (Monthly)

1. **Clean Sensors**
   - Remove mineral deposits with vinegar
   - Rinse thoroughly
   - Dry completely before use

2. **Update Firmware**
   - Check for updates on GitHub
   - Use OTA update feature

3. **PID Tuning**
   - If performance degrades, re-tune PID
   - Use auto-tuning libraries if needed

### Long-Term Storage

1. **Disconnect Power**
2. **Remove Sensors** from water
3. **Dry All Components** thoroughly
4. **Store in Dry Location**
5. **Coil Cables Loosely** (avoid sharp bends)

---

## Appendix A: PID Tuning Guide

### Understanding PID

- **P (Proportional)**: Immediate response to error
  - Too high: Oscillation
  - Too low: Slow response

- **I (Integral)**: Eliminates steady-state error
  - Too high: Overshoot
  - Too low: Never quite reaches target

- **D (Derivative)**: Dampens oscillations
  - Too high: Noise sensitivity
  - Too low: Overshoot

### Tuning Process

1. **Set I and D to 0**
2. **Increase P** until oscillation starts
3. **Reduce P** by 50%
4. **Add I** slowly until steady-state error eliminated
5. **Add D** to reduce overshoot

### Recommended Starting Values

| Water Bath Size | Kp | Ki | Kd |
|-----------------|----|----|-----|
| Small (2L) | 50 | 0.3 | 3 |
| Medium (6L) | 100 | 0.5 | 5 |
| Large (12L) | 150 | 0.7 | 8 |

---

## Appendix B: Cooking Temperature Guide

| Food | Temperature | Time | Notes |
|------|-------------|------|-------|
| **Beef** |
| Rare | 50-52°C | 1-4 hrs | Very tender, pink |
| Medium-Rare | 54-56°C | 1-4 hrs | Ideal for most cuts |
| Medium | 57-60°C | 1-4 hrs | Pink center |
| Well-Done | 62-65°C | 1-4 hrs | Fully cooked |
| **Chicken** |
| Breast | 60-65°C | 1-2 hrs | Juicy and tender |
| Thigh | 65-70°C | 2-4 hrs | Rich flavor |
| **Pork** |
| Chops | 57-60°C | 1-4 hrs | Slightly pink |
| Tenderloin | 60-63°C | 1-2 hrs | Fully cooked |
| **Fish** |
| Salmon | 48-50°C | 30-45 min | Buttery texture |
| Tuna | 45-48°C | 30-40 min | Sashimi-grade |
| White Fish | 50-55°C | 30-60 min | Flaky, moist |
| **Eggs** |
| Soft | 63-64°C | 45-60 min | Runny yolk |
| Medium | 65-66°C | 45-60 min | Jammy yolk |
| Hard | 67-68°C | 45-60 min | Firm throughout |
| **Vegetables** |
| Root Veg | 84-85°C | 1-2 hrs | Tender, al dente |
| Asparagus | 85°C | 15-30 min | Crisp-tender |

**Safety Note**: Times are for pasteurization. Refer to food safety guidelines for minimum safe temperatures.

---

## Appendix C: Bill of Materials

### Electronics Components

| Item | Quantity | Approx. Cost | Notes |
|------|----------|--------------|-------|
| ESP32-C6 DevKitC-1 | 1 | $8-12 | Espressif official |
| DS18B20 (Waterproof) | 2 | $3-5 each | 1m cable min. |
| 4.7kΩ Resistor | 1 | $0.10 | 1/4W |
| SSD1306 OLED | 1 | $5-8 | 128×64, I2C |
| Solid State Relay | 1 | $8-15 | 25A+ rated |
| Jumper Wires | Set | $5 | Male-Female |
| Breadboard (optional) | 1 | $3 | For prototyping |

### Cooking Equipment

| Item | Quantity | Approx. Cost | Notes |
|------|----------|--------------|-------|
| Immersion Heater | 1 | $15-30 | 800-1500W |
| Container | 1 | $10-20 | 12L+ polycarbonate |
| Vacuum Sealer | 1 | $30-100 | Optional but recommended |
| Vacuum Bags | Pack | $10-20 | Or Ziploc bags |
| Clip/Weight | 1 | $5 | To secure bags |

**Total Budget**: $100-200 (depending on quality choices)

---

## Technical Support

### Community Resources
- **GitHub Issues**: [github.com/yourproject/issues]
- **User Forum**: [forum.yourproject.com]
- **Email Support**: support@yourproject.com

### Warranty
This is a DIY project. **No warranty** is provided. Use at your own risk. Always follow electrical safety guidelines and local regulations.

### License
GNU General Public License v3.0

---

**Document Version**: 1.0  
**Last Updated**: 2026-02-08  
**Firmware Version**: 1.0.0

---

© 2026 DIY Sous Vide Controller Project. All rights reserved.
