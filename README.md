# ESP32 Sous Vide Controller

**Professional IoT-enabled precision temperature controller for sous vide cooking**

![ESP32-C6](https://img.shields.io/badge/ESP32-C6-blue)
![Arduino](https://img.shields.io/badge/Framework-Arduino-00979D)
![PlatformIO](https://img.shields.io/badge/Build-PlatformIO-orange)
![License](https://img.shields.io/badge/License-GPL--3.0-green)

## ✨ Features

### 🌡️ Precision Control
- **PID Temperature Control** - Maintains ±0.1°C accuracy
- **Dual Sensor Redundancy** - DS18B20 sensors with safety checks
- **Real-time Monitoring** - Live temperature graphing
- **Adjustable Power Limit** - 0-100% SSR duty cycle control

### 🔐 Safety First
- **6-Layer Safety System** with error codes (E01-E06)
- **Sensor Mismatch Detection** - Alerts if sensors disagree >3°C
- **Over-Temperature Protection** - Emergency stop at 90°C
- **Dry Run Detection** - Prevents heating without water
- **24-Hour Watchdog** - Auto-shutdown after max runtime
- **Invalid Sensor Detection** - Catches NaN/-127°C readings

### 📱 Dual Interface
- **Web Dashboard** - VS Code-themed dark mode UI with Fira Code font
- **Telegram Bot** - Custom keyboard with preset management
- **OLED Display** - 128×32 local status (SSD1306)
- **OTA Updates** - Wireless firmware updates with authentication

### 💾 Customizable
- **Preset Manager** - Save/load/delete cooking profiles
- **Telegram Integration** - Remote control and monitoring
- **WiFi Manager** - Easy network configuration
- **Persistent Storage** - Settings saved to NVS

---

## 📚 Documentation

| Document | Description |
|----------|-------------|
| **[QUICK_START.md](docs/QUICK_START.md)** | Get cooking in 10 minutes! |
| **[USER_MANUAL.md](docs/USER_MANUAL.md)** | Complete user guide with recipes |
| **[TECHNICAL_DATASHEET.md](docs/TECHNICAL_DATASHEET.md)** | Full technical specifications |

---

## 🛠️ Hardware Requirements

### Electronics
- **ESP32-C6 DevKitC-1** (or compatible)
- **2× DS18B20** Temperature Sensors (waterproof)
- **4.7kΩ Resistor** (OneWire pull-up)
- **SSD1306 OLED** (128×64, I2C)
- **Solid State Relay** (25A+ rated)

### Cooking Equipment  
- Immersion heater (800-1500W)
- Water bath container (12L+)
- Vacuum sealer (optional but recommended)

**Budget**: ~$100-200 total

---

## ⚡ Quick Start

### 1. Flash Firmware

```bash
# Clone repository
git clone https://github.com/yourproject/sous-vide.git
cd sous-vide

# Copy secrets template and edit with your credentials
cp include/Secrets.h.example src/Secrets.h
nano src/Secrets.h

# Build and upload
platformio run --target upload
```

### 2. Configuration

Edit `src/Secrets.h` with your credentials:

```cpp
#define BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"    // From @BotFather
#define OTA_USERNAME "admin"                    // OTA update username
#define OTA_PASSWORD "sousvide2024"             // OTA update password (CHANGE THIS!)
#define WIFI_AP_PASSWORD "sousvide123"          // WiFi config AP password (min 8 chars)
```

⚠️ **Security**: Change default passwords before deployment!

### 3. WiFi Setup

1. Power on device
2. Connect to `SousVide_Config_AP` (password: `sousvide123`)
3. Enter your WiFi credentials
4. Find IP on OLED display

### 4. Start Cooking!

**Web**: `http://<device-ip>`
**OTA Updates**: `http://<device-ip>/update` (requires authentication)
**Telegram**: `/start` in your bot

---

## 🎨 Web Interface

**VS Code Dark Theme** with professional developer aesthetics:
- Fira Code monospace font
- Clean, minimal design
- Real-time temperature graphing
- Error code display system
- Mobile-responsive layout

![Web UI Preview](https://via.placeholder.com/800x400?text=VS+Code+Style+Dark+Mode)

---

## 🤖 Telegram Bot

### Quick Commands
```
/start          → Welcome message with custom keyboard
/status         → Current temperature and state
/stop           → Emergency stop
/set 60         → Set target to 60°C
```

### Preset Management
```
/presets                → List all saved presets
/addpreset Steak 55     → Create new preset
/cook Steak             → Start with preset
/delpreset Steak        → Remove preset
```

### Custom Keyboard
```
┌───────────────┬───────────────┐
│ 📊 Status     │ 🔴 Stop       │
├───────────────┼───────────────┤
│ 📋 Presets    │ ⚙️ Settings   │
├───────────────┼───────────────┤
│ 🥩 Steak 55°C │ 🐟 Fish 50°C  │
├───────────────┼───────────────┤
│ 🥚 Egg 64°C   │ 🍗 Chicken 65°│
└───────────────┴───────────────┘
```

---

## 🔧 Pin Configuration

| Function | GPIO | Notes |
|----------|------|-------|
| **OneWire Data** | GPIO 4 | DS18B20 sensors |
| **SSR Control** | GPIO 3 | Relay output |
| **I2C SDA** | GPIO 6 | OLED display |
| **I2C SCL** | GPIO 7 | OLED display |

---

## 🛡️ Safety Error Codes

| Code | Description | Action Required |
|------|-------------|-----------------|
| **E01** | Sensor Mismatch >3°C | Stir water, check sensors |
| **E02** | Over Temperature >90°C | **TURN OFF POWER** |
| **E03** | Dry Run Detected | Add water, submerge element |
| **E04** | 24H Timeout | Reset controller |
| **E05** | Under Temperature <-50°C | Check sensor connection |
| **E06** | Invalid Sensor (NaN/-127) | Replace faulty sensor |

---

## 📦 Dependencies

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

---

## 🍳 Cooking Guide

| Food | Temp | Time | Notes |
|------|------|------|-------|
| **Steak (Medium-Rare)** | 54-56°C | 1-4 hrs | Perfect pink center |
| **Chicken Breast** | 63-65°C | 1-2 hrs | Juicy and tender |
| **Salmon** | 48-50°C | 30-45 min | Buttery texture |
| **Soft Eggs** | 63-64°C | 45-60 min | Runny yolk |
| **Vegetables** | 84-85°C | 1-2 hrs | Crisp-tender |

See **[USER_MANUAL.md](docs/USER_MANUAL.md)** for complete cooking temperatures and times.

---

## 🏗️ Project Structure

```
Sous_Vide/
├── docs/
│   ├── QUICK_START.md           # 10-minute setup guide
│   ├── USER_MANUAL.md           # Complete user documentation
│   └── TECHNICAL_DATASHEET.md   # Technical specifications
├── src/
│   ├── main.cpp                 # Main application code
│   ├── Safety.cpp/h             # Safety system implementation
│   ├── Display.cpp/h            # OLED display driver
│   ├── WebPage.h                # Web interface HTML/CSS/JS
│   └── Secrets.h                # Telegram Bot Token (edit this!)
├── platformio.ini               # Build configuration
├── min_spiffs.csv               # Flash partition table
└── README.md                    # This file
```

---

## 🚀 Advanced Features

### PID Tuning
Default values in `main.cpp`:
```cpp
double Kp = 100;  // Proportional
double Ki = 0.5;  // Integral  
double Kd = 5;    // Derivative
```

Adjust these for your specific setup. See manual for tuning guide.

### OTA Updates
1. Navigate to `http://<device-ip>/update`
2. Enter username and password (default: `admin` / `sousvide2024`)
3. Upload `.bin` file
4. Automatic reboot with new firmware

⚠️ **Security**: Change the default OTA password in `src/Secrets.h`!

### Custom Presets
Via Telegram:
```
/addpreset MyRecipe 62.5
/cook MyRecipe
```

Up to 10 custom presets stored in NVS.

---

## 🐛 Troubleshooting

### Upload Fails
- **Check COM port**: Device Manager → Ports
- **Manual bootloader**: Hold BOOT, press RESET, release BOOT
- **Try different USB cable**: Ensure data transfer support

### WiFi Won't Connect
- Wait 180s for Config AP to appear
- Reset WiFi: Hold RESET for 10s
- Check 2.4GHz band is enabled on router

### Temperature Unstable
- Increase water volume
- Tune PID parameters
- Check sensor placement (both submerged)

See **[USER_MANUAL.md](docs/USER_MANUAL.md)** for complete troubleshooting guide.

---

## 🤝 Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create feature branch
3. Test thoroughly
4. Submit pull request

---

## 📄 License

**Software**: GNU General Public License v3.0  
**Documentation**: Creative Commons BY-SA 4.0  
**Hardware**: Open Source Hardware (OSHW)

---

## ⚠️ Disclaimer

This is a DIY project controlling mains-powered heating elements. **Use at your own risk.**

- Follow all electrical safety guidelines
- Ensure proper SSR ratings and isolation  
- Never leave unattended (24hr watchdog is backup only)
- Verify food safety temperatures independently

---

## 🌟 Acknowledgments

Built with:
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [PlatformIO](https://platformio.org/)
- [OneWireNg](https://github.com/pstolarz/OneWireNg)
- [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA)
- [UniversalTelegramBot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)

---

## 📞 Support

- **GitHub Issues**: [Report bugs](https://github.com/yourproject/issues)
- **Discussions**: [Community forum](https://github.com/yourproject/discussions)
- **Email**: support@yourproject.com

---

**Version**: 1.0.0  
**Last Updated**: 2026-02-08  
**Author**: DIY Sous Vide Controller Project

---

⭐ **If this helped you cook perfect steaks, give it a star!** ⭐

Happy Cooking! 🥩🍗🥚🐟
