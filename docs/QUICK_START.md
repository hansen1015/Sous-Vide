# 🚀 Quick Start Guide

## Get Cooking in 10 Minutes!

### What You Need
- ✅ ESP32-C6 DevKitC-1 (assembled and programmed)
- ✅ Water bath container
- ✅ WiFi network
- ✅ Smartphone or computer

---

## Step 1: Power On (30 seconds)

1. **Plug in** the USB-C cable
2. **OLED Display** will show:
   ```
   Sous Vide Booting...
   Connecting WiFi...
   ```

---

## Step 2: WiFi Setup (2 minutes)

### First Time Setup

1. **On your phone**, look for WiFi network: `SousVide_Config_AP`
2. **Connect** to it (no password)
3. **Browser opens automatically** (or go to `192.168.4.1`)
4. **Select** your home WiFi
5. **Enter password**
6. **Click "Save"**

✅ Device reboots and connects to your WiFi!

### Find Your Sous Vide

**Option A: OLED Display**
```
IP: 192.168.1.100
```

**Option B: Serial Monitor**
```bash
platformio device monitor
```

---

## Step 3: First Cook (5 minutes)

### Via Web Interface

1. **Open browser**: `http://192.168.1.100` (use your IP)
2. **Enter target temp**: e.g., `55°C`
3. **Click "Set Temp"**
4. **Click "START"**
5. Done! 🎉

### Via Telegram Bot

1. **Open Telegram**
2. **Search** for your bot (name from BotFather)
3. **Send** `/start`
4. **Tap** `🥩 Steak 55°C`
5. Done! 🎉

---

## Quick Cooking Examples

### Steak (Medium-Rare)
```
Target: 55°C
Time: 1-2 hours
Power: 100%
```

### Chicken Breast
```
Target: 63°C
Time: 1.5 hours
Power: 100%
```

### Soft-Cooked Eggs
```
Target: 64°C  
Time: 45 minutes
Power: 80%
```

---

## Troubleshooting

### ❌ Can't find `SousVide_Config_AP`
**Wait 180 seconds** after power-on. AP starts if WiFi fails.

### ❌ Web page won't load
1. Check IP address on OLED
2. Ping device: `ping 192.168.1.100`
3. Try reboot (press RESET button)

### ❌ Temperature not changing
1. Check sensors are **submerged**
2. Verify **water level** covers heating element
3. Check SSR LED is **blinking**

### ⚠️ ERROR message
Check error code:
- **E01**: Sensors disagree - stir water, wait 30s
- **E02**: Over 90°C - **TURN OFF POWER** immediately
- **E03**: Dry run - add water, ensure element submerged
- **E06**: Sensor fault - check connections

---

## Safety Checklist ✅

Before every cook:
- [ ] Water level covers **all sensors**
- [ ] Heating element **fully submerged**
- [ ] SSR and AC wiring **properly insulated**
- [ ] Container on **stable, heat-safe surface**
- [ ] Within reach to **monitor** during cook

---

## Web Interface Tour

### Status Dashboard
```
┌─────────────┬─────────────┬─────────────┬─────────────┐
│ Current Temp│ Target Temp │   Status    │ Power Limit │
│   55.2°C    │   55.0°C    │  ⚡HEATING   │    100%     │
└─────────────┴─────────────┴─────────────┴─────────────┘
```

### Quick Actions
- **Set Temp**: Type temperature, click "Set Temp"
- **START/STOP**: Big buttons, can't miss them
- **Presets**: One-click common temperatures
- **Power Limit**: Slider to reduce max power (0-100%)

### Live Graph
Real-time temperature plot updates every 2 seconds

---

## Telegram Bot Commands

### Most Used
```
/status        → Current temperature and state
/stop          → Emergency stop
🥩 Button      → Quick preset (55°C steak)
```

### Preset Management
```
/presets                → List all saved presets
/addpreset Salmon 50    → Create new preset
/cook Salmon            → Start cooking with preset
/delpreset Salmon       → Remove preset
```

### Advanced
```
/set 60                 → Set target to 60°C
/setpower 80            → Limit power to 80%
/settings               → View current settings
```

---

## Tips & Tricks

### 💡 Pro Tips

**Faster Heating**
- Use hot tap water to start
- Set power limit to 100%
- Smaller container = faster

**Better Accuracy**
- Let stabilize for 10 minutes before adding food
- Don't open lid during cook
- Stir occasionally for large batches

**Energy Saving**
- Reduce power to 60-80% once at temperature
- Use insulated container
- Cook overnight (it's safe!)

### 🔥 Common Mistakes

❌ **Too much water**
- Longer heat-up time
- Wasted energy

✅ **Just enough**
- Cover food + 2cm
- Faster and efficient

---

❌ **Opening lid**
- Temperature drops
- Longer cook time

✅ **Leave sealed**
- Set timer, walk away
- Perfect results

---

❌ **Rushing**
- Sous vide is low and slow
- Minimum times matter

✅ **Be patient**
- Use the time for prep
- Impossible to overcook (mostly)

---

## Common Cooking Times

| Food | Temp | Min Time | Max Time |
|------|------|----------|----------|
| **Steak (1")** | 54°C | 1 hr | 4 hrs |
| **Chicken Breast** | 63°C | 1.5 hrs | 3 hrs |
| **Pork Chops** | 60°C | 1.5 hrs | 4 hrs |
| **Salmon** | 50°C | 30 min | 1 hr |
| **Eggs (soft)** | 64°C | 45 min | 1 hr |
| **Carrots** | 84°C | 1 hr | 2 hrs |

**Note**: These are safe minimum times. Going longer (within range) is usually fine!

---

## Firmware Updates

### OTA (Over-The-Air)

1. **Download** latest `.bin` from GitHub
2. **Open** `http://192.168.1.100/update`
3. **Select** `.bin` file
4. **Click "Update"**
5. **Wait** ~30 seconds
6. Done!

### USB (If OTA fails)

```bash
platformio run --target upload
```

---

## Next Steps

### Learn More
- 📖 **User Manual**: Full documentation (`docs/USER_MANUAL.md`)
- 🔧 **Datasheet**: Technical specs (`docs/TECHNICAL_DATASHEET.md`)
- 🍳 **Recipes**: Check online sous vide recipe sites

### Customize
- **PID Tuning**: Adjust Kp, Ki, Kd in `main.cpp`
- **Telegram**: Add custom presets
- **Web UI**: Edit `WebPage.h` for your style

### Community
- 💬 **Forum**: Share your cooks
- 🐛 **Issues**: Report bugs on GitHub
- ⭐ **Star**: Support the project

---

## Emergency Procedures

### 🚨 If You See "ERROR"

1. **Press STOP** (web or Telegram)
2. **Read error code** (E01-E06)
3. **Check** error code guide in manual
4. **Fix issue** before restarting

### 🚨 If Element Won't Turn Off

1. **UNPLUG** power immediately
2. **DO NOT** try to restart
3. **Check** SSR is not failed
4. **Replace SSR** if stuck ON

### 🚨 If Sensors Disagree

1. **Stop** cooking
2. **Stir** water bath thoroughly
3. **Wait** 30 seconds
4. **Check** both sensors submerged
5. **Restart** if error clears

---

## FAQ

**Q: Can I leave it overnight?**
A: Yes! That's the beauty of sous vide. The 24-hour watchdog ensures safety.

**Q: Why is it taking so long to heat up?**
A: Large water volume = longer time. Use hot water to start, or smaller container.

**Q: Can I use Ziploc bags?**
A: Yes! Use the water displacement method. Remove air, seal most of the way, submerge slowly, seal completely.

**Q: What if WiFi disconnects?**
A: Cooking continues! Just reconnect to check status.

**Q: My egg exploded!**
A: Too hot or thermal shock. Use slow warm-up (50° then 64°). Pierce large end.

**Q: Can I cook from frozen?**
A: Yes, but add 50% more time. Thaw for best texture.

---

## Success Checklist

After your first cook:
- [ ] Temperature stayed within ±0.5°C
- [ ] Food cooked evenly
- [ ] No error messages
- [ ] Easy to control (web/Telegram)
- [ ] Ready to try more recipes!

---

🎉 **Congratulations! You're now a sous vide pro!**

Happy Cooking! 🥩🍗🥚🐟

---

**Version**: 1.0  
**Date**: 2026-02-08  
**Support**: Check USER_MANUAL.md for detailed help

© 2026 DIY Sous Vide Controller Project
