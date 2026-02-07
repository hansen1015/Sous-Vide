# Sous Vide Project - Detailed Implementation Plan

## 1. Bill of Materials (BOM) Checklist

### Core Electronics
- [ ] **MCU:** ESP32 WROOM-32 DevKit V1 (or NodeMCU-32S).
- [ ] **Temperature Sensors:** 2x DS18B20 (Waterproof probe type, 1m cable).
- [ ] **Resistor:** 1x 4.7kΩ (Pull-up for DS18B20).
- [ ] **Relay:** Solid State Relay (SSR-25DA or similar).
    - *Note:* While 60W is low current (~0.27A), an SSR is chosen for PID PWM capability. Ensure it accepts 3.3V control input. If not, a transistor driver (2N2222) is needed.
- [ ] **Heater:** 220V AC Heating Element (60W).
    - *Type:* Immersion cartridge or silicone pad, depending on vessel.
- [ ] **Power Supply:** 5V 1A Micro USB Wall Adapter (simplest) OR Hi-Link HLK-PM01 5V module for integrated power.

### Safety Components (CRITICAL)
- [ ] **Thermal Fuse (Cutoff):** Rated ~100°C - 115°C (depending on max cooking temp 85°C + buffer).
    - *Action:* Must be physically attached to the heating element or heat spreader. Failsafe if SSR shorts ON.
- [ ] **Fuse & Holder:** 2A Fast Blow Fuse (AC Side). Protective for internal shorts.
- [ ] **Grounding:** 3-Prong Power Cord (Live, Neutral, Earth) if using a metal chassis or immersion heater with metal sheath.
- [ ] **Case:** IP54 or higher rated plastic enclosure (protects electronics from steam/splashes). Waterproof Cable Glands for wire entry/exit.

### Wiring & Connectors
- [ ] **AC Wiring (High Voltage):** 0.75mm² (18 AWG) stranded wire with silicone insulation (heat resistant).
- [ ] **DC Wiring (Low Voltage):** 24 AWG or standard Dupont wires for sensors/logic.
- [ ] **Connectors:** Wago 221 lever connectors (safe for AC) or terminal blocks. Heat shrink tubing.

---

## 2. Wiring & Assembly Guide

> [!CAUTION]
> **DANGER: 220V AC Mains Voltage.**
> Never touch any component while plugged in.
> Isolate the Low Voltage (ESP32) side from the High Voltage (AC) side physically in the case.

### Phase 1: Low Voltage (Safe)
1.  **Sensors:**
    - Connect both DS18B20 `VCC` wires (Red) to ESP32 `3.3V`.
    - Connect both DS18B20 `GND` wires (Black) to ESP32 `GND`.
    - Connect both DS18B20 `DATA` wires (Yellow) to ESP32 `GPIO 4`.
    - Install 4.7kΩ resistor between `3.3V` and `GPIO 4` (only one resistor needed for the bus).
2.  **SSR Control:**
    - Connect SSR Control `+` (Input) to ESP32 `GPIO 15`.
    - Connect SSR Control `-` (Ground) to ESP32 `GND`.
3.  **Power:**
    - Connect USB cable to ESP32.

### Phase 2: High Voltage (Dangerous) - DO NOT PLUG IN YET
1.  **Power Entry:**
    - 3-Prong plug `Live` (Brown) -> Fuse Holder (2A).
    - 3-Prong plug `Neutral` (Blue) -> Common Neutral point.
    - 3-Prong plug `Earth` (Yellow/Green) -> Metal Chassis / Heater Body.
2.  **Safety Loop (Thermal Fuse):**
    - From Fuse output -> Thermal Fuse (attached to heater) -> SSR Load Terminal 1.
3.  **Heater Switching:**
    - SSR Load Terminal 2 -> Heater Connection 1.
    - Heater Connection 2 -> Common Neutral point.

### Diagram Logic
`AC LIVE` -> `FUSE` -> `THERMAL FUSE` -> `SSR` -> `HEATER` -> `AC NEUTRAL`

---

## 3. Testing Protocol

### Unit Tests (Hardware & Safety Logic)

| Test Case | Procedure | Expected Outcome |
| :--- | :--- | :--- |
| **1. Sensor Redundancy** | Connect both sensors. Place one in warm water, one in ambient. | System should read both. Control logic uses HIGHEST temp. Alarm if difference > 3°C (Simulated). |
| **2. Sensor Failure** | Disconnect one sensor (physically unplug data wire) while running. | System detects missing device count (on OneWire bus). Triggers **EMERGENCY STOP**. |
| **3. Dry Run (Slope)** | Heat the sensor rapidly (e.g., rub with fingers or hot air) without heater power. | Temp rise > 0.6°C/sec. System triggers **DRY RUN ERROR** and stops SSR. |
| **4. Watchdog Timer** | Firmware Simulation: Force state to "Heating" for > 24 hours (accelerated time). | System shuts down. |
| **5. SSR Failsafe** | (Danger) Simulate SSR stuck ON by bypassing it (short terminals). | Temp rises uncontrolled. **Electronic safety** (software) screams via Telegram. **Thermal Fuse** (hardware) blows at 115°C. |
| **6. WiFi Fallback** | Disconnect Router. Reboot ESP32. | ESP32 creates Access Point "SousVide_AP". |

---

## 4. Potential Pitfalls

### 1. Water Ingress & Humidity
*   **Risk:** Steam from the water bath enters the electronics case.
*   **Failure:** Corrosion of ESP32, shorts on AC side.
*   **Fix:** Use a sealed Tupperware/IP-rated box. Use "Conformal Coating" (nail varnish works for DIY) on the ESP32 PCB (avoiding USB/buttons). Silica gel packets inside.

### 2. Sensor Lag & Overshoot
*   **Risk:** DS18B20 has a slow response time (can be 750ms-1s conversion + thermal mass of stainless probe).
*   **Failure:** Heater stays on too long, water overshoots target by 2-5°C.
*   **Fix:** Ensure PID tuning is conservative (lower `Kp`, higher `Kd`). Circulate water well (aquarium pump or stirrer) - **Crucial for Sous Vide accuracy**.

### 3. SSR Overheating
*   **Risk:** Even at 60W, cheap SSRs can get warm if the switching frequency is too high.
*   **Failure:** SSR fails CLOSED (On).
*   **Fix:** Use "Slow PWM" (Time Proportioning) with a window matching mains frequency (e.g., 2000ms window). Do not PWM at 1000Hz.

### 4. ESP32 Brownout
*   **Risk:** When SSR switches or WiFi transmits, 5V rail dips.
*   **Failure:** ESP32 resets randomly.
*   **Fix:** Use a quality USB power supply (min 1A). Add a 470uF Capacitor across ESP32 5V/GND rails.

### 5. Telegram Rate Limits
*   **Risk:** Sending a message every second for temp updates.
*   **Failure:** Telegram API bans/blocks the bot for spamming.
*   **Fix:** Only send updates on status change (Idle->Heating), errors, or upon user request. Use the Chart.js web interface for "Live" monitoring, not Telegram.
