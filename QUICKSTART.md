# ArduiBeeScale v2.0 - Quick Start Guide

**⚡ 5 Minutes to Get Running**

---

## Step 1: Setup Configuration (2 min)

```bash
cd arduino
cp config_template.h config.h
```

Edit `arduino/config.h` and set your credentials:
```cpp
const char API_KEY[] PROGMEM = "YOUR_API_KEY_HERE";
const char APN_CONFIG[] PROGMEM = "webaut";  // Your carrier APN
const char DEVICE_IDENT[] PROGMEM = "ScaleHouse";  // Your device name
const char TIMEZONE_CONFIG[] PROGMEM = "Europe/Vienna";
```

**Where to get API_KEY?** https://beta.btree.at/setting/profile/api

---

## Step 2: Configure Arduino IDE (1 min)

1. **Install Libraries** (Arduino → Sketch → Include Library → Manage Libraries)
   - HX711 (by Bogde)
   - DHT
   - Adafruit Unified Sensor

2. **Select Board**: Arduino → Tools → Board → Arduino UNO

3. **Select Port**: Arduino → Tools → Port → COM3 (or your port)

---

## Step 3: Upload Code (1 min)

1. Open `arduino/arduino.ino` in Arduino IDE
2. Click **Verify** ✓ (compile test)
3. Click **Upload** ➜ (flash to board)

---

## Step 4: Monitor Output (1 min)

Open **Serial Monitor** (Tools → Serial Monitor) at **9600 baud**

You should see:
```
[INF] Watchdog enabled (8-second timeout)
[INF] System initializing...
[INF] Checking if GSM is running...
[INF] GSM ready!
...
[INF] Reading sensors...
[INF] Weight (kg): 25.45
[INF] Temperature (C): 22.50
[INF] Humidity (%): 65.30
[INF] Request cycle complete!
```

---

## ✅ You're Done!

Data should appear in your bTree dashboard within 2 hours (or next wake cycle).

---

## 🐛 Troubleshooting

| Issue | Fix |
|-------|-----|
| **Compilation error: `fatal error: config.h: No such file`** | Run: `cp config_template.h config.h` |
| **"API returns 401 Unauthorized"** | Check API_KEY in config.h is correct |
| **"Could not get IP address"** | Check APN in config.h matches your carrier |
| **"Failed to read from DHT sensor"** | Check DHT22 wiring and power |
| **Arduino doesn't respond to AT commands** | Check SIM900 power pin (pin 9) connection |

---

## 📖 More Information

- **Setup Details**: See `SETUP_GUIDE.md`
- **Technical Info**: See `CRITICAL_FIXES_SUMMARY.md`
- **All Changes**: See `IMPROVEMENTS_SUMMARY.md`
- **Scale Calibration**: Upload `calibrate/calibrate.ino` first

---

## 🎯 Debug Modes

Change debug level in `arduino/arduino.ino` line 47:

```cpp
#define DEBUG_LEVEL  DEBUG_OFF       // No debug output
#define DEBUG_LEVEL  DEBUG_ERRORS    // Only errors (default)
#define DEBUG_LEVEL  DEBUG_INFO      // Errors + info
#define DEBUG_LEVEL  DEBUG_VERBOSE   // Everything
```

Output format:
- `[ERR]` - Errors
- `[INF]` - Information
- `[VRB]` - Verbose debug

---

## ⚙️ Hardware Pins

| Component | Pin | Arduino Pin |
|-----------|-----|-------------|
| HX711 Data | DT | 5 |
| HX711 Clock | CLK | 6 |
| DHT22 | Data | 10 |
| SIM900 | Power | 9 |
| ATtiny | Signal | 2 |

To change pins, edit lines 14-18 in `arduino.ino`

---

## 🔧 Common Customizations

### Change Update Frequency
Edit `attiny/attiny.ino` line 15:
```cpp
int const circles = 900;  // 900 * 8sec = 120 min
// Change to:
int const circles = 450;  // 450 * 8sec = 60 min (every hour)
```

### Change Sensor Ranges
Edit `arduino.ino` lines 35-40:
```cpp
#define MIN_WEIGHT_KG  0.0
#define MAX_WEIGHT_KG  500.0
// Adjust to your scale capacity
```

### Change Watchdog Timeout
Edit `arduino.ino` line 31:
```cpp
#define WATCHDOG_TIMEOUT  WDTO_8S  // 8 seconds
// Options: WDTO_1S, WDTO_2S, WDTO_4S, WDTO_8S (default)
```

---

## 📊 What's New in v2.0?

✅ **Fixed**:
- Stack overflow from unbounded recursion
- API key security vulnerability
- Buffer null-termination crashes

✨ **Added**:
- Watchdog timer (prevents hangs)
- Sensor data validation (range checking)
- Debug logging framework
- Configuration management

📖 **Documentation**:
- Setup guide with troubleshooting
- Technical deep-dive for developers
- Complete improvements summary

---

## 🚀 Next Steps

1. **Test it**: Monitor for 2-3 cycles
2. **Calibrate**: Run `calibrate/calibrate.ino` if needed
3. **Deploy**: Install in beehive
4. **Monitor**: Check bTree dashboard for data

---

## 💡 Pro Tips

- Use **lithium batteries** for better GSM performance
- Keep **antenna** clear of metal
- Mount in **weather-resistant enclosure**
- Check **signal strength** regularly
- Review **logs** when troubleshooting

---

## ❓ Questions?

- See `SETUP_GUIDE.md` troubleshooting section
- Review `CRITICAL_FIXES_SUMMARY.md` for technical details
- Check `README.md` for hardware recommendations

---

**Status**: ✅ Production Ready v2.0
**Last Updated**: 2025-11-15
**Maintained by**: Jeremy JEANNE
