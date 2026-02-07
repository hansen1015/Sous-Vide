# ESP32 Sous Vide Controller

DIY Internet-connected Sous Vide machine powered by ESP32.

## Features
- **Precise Temperature Control:** PID algorithm with SSR PWM support.
- **Dual Connectivity:** 
  - **WiFi Client:** Connects to home WiFi for remote monitoring via Telegram.
  - **Fallback AP:** Creates `SousVide_Offline` Access Point if WiFi fails, allowing local control.
- **Safety First:** 
  - Dual DS18B20 sensor redundancy (shuts down if mismatch > 3°C).
  - Dry Run Protection (detects rapid temp rise).
  - 24-Hour Watchdog.
- **Interfaces:**
  - **Web Dashboard:** Real-time Chart.js graph, presets, manual control.
  - **Telegram Bot:** Status updates and basic commands (`/status`, `/start`, `/stop`, `/set 55`).
- **OTA Updates:** Flash firmware wirelessly via `/update` endpoint.

## Getting Started

### 1. Hardware Setup
Refer to `docs/Project_Detailed_Plan.md` for the wiring guide and BOM.
**WARNING:** Mains voltage is dangerous. Double-check all wiring.

### 2. Configuration
1. Open `include/Secrets.h`.
2. Add your Telegram Bot Token and Chat ID.
3. (Optional) Adjust PID parameters in `src/main.cpp` if needed.

### 3. Flashing (PlatformIO)
1. Install VSCode and the **PlatformIO** extension.
2. Open this project folder.
3. Connect ESP32 via USB.
4. Click the **PlatformIO: Upload** button (Arrow icon) in the bottom toolbar.

### 4. First Run
1. Power on the device.
2. Connect to the WiFi network `SousVide_Config_AP` (password: none, or as configured).
3. A captive portal should open (or go to `192.168.4.1`).
4. Enter your home WiFi credentials.
5. The device will reboot and connect. 
   - Observe the Serial Monitor/Terminal for the assigned IP address.
   - Or allow it to fail to connect to enter `SousVide_Offline` mode.

### 5. Control
- **Web:** Navigate to `http://<ESP32_IP>/`
- **Telegram:** Send `/start` to the bot.
- **Update Firmware:** Go to `http://<ESP32_IP>/update`

## Safety Notes
- Always test with water (not dry) first to verify "Dry Run" protection doesn't trigger falsely.
- Ensure the thermal fuse is physically attached to the heater.
