# LCD1602 I2C Display - Quick Reference Card

**Print this page and keep it near your workbench!**

---

## Wiring Overview (30 seconds)

```
Arduino → LCD1602 I2C Module
├─ 5V       → VCC (red)
├─ GND      → GND (black)
├─ A4 (SDA) → SDA (green)
├─ A5 (SCL) → SCL (yellow)

Arduino → Push Button
├─ Pin 2 → Button Pin 1
└─ GND   → Button Pin 2
```

---

## 5-Minute Setup Checklist

```
Before Wiring:
□ Identify LCD I2C address (0x27 or 0x3F)
□ Get I2C_SCANNER.ino from arduino/ folder
□ Upload I2C_SCANNER.ino, find address in Serial Monitor
□ Note address: ________

Configuration:
□ Copy config_wifi_template.h → config_wifi.h
□ Edit config_wifi.h, update LCD_I2C_ADDRESS with your address
□ Update WIFI_SSID and WIFI_PASSWORD
□ Update MQTT_BROKER with Raspberry Pi IP

Hardware Assembly:
□ Connect 4 I2C wires (VCC, GND, SDA, SCL)
□ Connect button pins (Pin 2, GND)
□ Verify no loose connections
□ Check all solder joints are shiny

Upload Code:
□ Open arduino_wifi_mqtt_lcd.ino
□ Verify compiles: Sketch → Verify ✅
□ Upload: Sketch → Upload
□ Wait for "Done uploading"

Test:
□ Open Serial Monitor (9600 baud)
□ Should see WiFi connecting messages
□ Press button: LCD should turn on
□ Cycle through 3 screens (2 sec each)
```

---

## I2C Address Lookup Table

| LCD Type | Most Likely Address | How to Find |
|---|---|---|
| Blue backlight | **0x27** | Run I2C_SCANNER.ino |
| Green backlight | **0x3F** | Run I2C_SCANNER.ino |
| Unknown | ? | Upload I2C_SCANNER.ino |

**Run I2C Scanner**: `arduino/I2C_SCANNER.ino`

---

## Troubleshooting Matrix

### Symptom: LCD Shows Nothing
```
→ Check 5V power (use multimeter)
→ Check GND connection (continuity)
→ Try address 0x3F instead of 0x27
→ Resolder I2C backpack connections
```

### Symptom: LCD Has Backlight But No Text
```
→ Wrong I2C address (change 0x27 ↔ 0x3F)
→ Check SDA (A4) wire connection
→ Check SCL (A5) wire connection
→ Resolder all I2C connections
```

### Symptom: Button Doesn't Work
```
→ Check Pin 2 connection to button
→ Check button GND connection
→ Try button on different pins (3, 4, etc.)
→ Check Serial Monitor for errors
```

### Symptom: Wrong Battery Percentage
```
→ Calibrate voltage divider (5kΩ + 1kΩ)
→ Check battery voltage with multimeter
→ Verify A0 connections solid
```

---

## File Locations

| File | Purpose | Location |
|---|---|---|
| **arduino_wifi_mqtt_lcd.ino** | Main code | `arduino/` |
| **config_wifi_template.h** | Config template | `arduino/` |
| **I2C_SCANNER.ino** | Find I2C address | `arduino/` |
| **LCD_HARDWARE_SETUP.md** | Detailed wiring | root folder |
| **LCD_TESTING_GUIDE.md** | Testing procedures | root folder |

---

## Essential Commands (Copy-Paste)

### Find Your Raspberry Pi IP
```bash
ssh pi@raspberrypi.local
hostname -I
# Example output: 192.168.1.100
```

### Test MQTT on Pi
```bash
mosquitto_sub -h localhost -t "beehive/#" -v
```

### View Arduino Serial Output
```
Tools → Serial Monitor
Set baud rate: 9600
```

---

## Configuration Values at a Glance

```cpp
// config_wifi.h - Key Settings

// CRITICAL: Your values
#define WIFI_SSID            "your_ssid"       // ← Change
#define WIFI_PASSWORD        "your_password"   // ← Change
#define MQTT_BROKER          "192.168.1.100"   // ← Your Pi IP

// LCD Settings
#define LCD_I2C_ADDRESS      0x27              // ← Your address
#define BUTTON_PIN           2                 // Default OK
#define DISPLAY_DURATION_MS  2000              // 2 seconds default

// Calibration (scale)
#define SCALE_CALIBRATION    -19689.35         // Your scale value
#define SCALE_OFFSET         -145680           // Your scale value
```

---

## Display Output Format

### Screen 1: Temperature & Humidity (2 sec)
```
┌────────────────┐
│Temp: 23.5C 65%│
└────────────────┘
```

### Screen 2: Weight (2 sec)
```
┌────────────────┐
│Weight: 32.45kg│
└────────────────┘
```

### Screen 3: Battery (2 sec)
```
┌────────────────┐
│Battery: 75%    │
│Voltage: 5.40V  │
└────────────────┘
```

---

## Pin Assignments

| Arduino Pin | Purpose | Type | Notes |
|---|---|---|---|
| **2** | Button Input | Digital/INT0 | Interrupt enabled |
| **5** | HX711 DT | Digital | Load cell data |
| **6** | HX711 CLK | Digital | Load cell clock |
| **8** | ESP RX | Digital | From WiFi module |
| **9** | ESP TX | Digital | To WiFi module |
| **10** | DHT22 Data | Digital | Temperature sensor |
| **A0** | Battery V | Analog | Voltage reading |
| **A4** | I2C SDA | Analog/I2C | LCD data (pull-up) |
| **A5** | I2C SCL | Analog/I2C | LCD clock (pull-up) |

---

## Battery Life Estimate

| Conditions | Battery Life | Note |
|---|---|---|
| Good WiFi (-65 dBm) | 3.5 months | ✅ Recommended |
| Weak WiFi (-80 dBm) | 1.5 months | ⚠️ Check signal |
| Very weak WiFi (-90 dBm) | 10-30 days | ❌ Move system |
| Fresh AA batteries (4x) | Baseline | ~8000 mAh total |

---

## Common Errors & Fixes

| Error | Cause | Fix |
|---|---|---|
| "No I2C devices found" | Missing LCD connection | Check VCC, GND, SDA, SCL |
| "Button press #0" | Button not working | Check Pin 2 → button connection |
| "WiFi connecting..." (stuck) | Wrong SSID/password | Check config_wifi.h values |
| "MQTT not responding" | Wrong broker IP | Verify Raspberry Pi IP |
| Garbled LCD text | Wrong I2C address | Change 0x27 ↔ 0x3F |

---

## Testing Sequence (Start Here)

```
1. Upload I2C_SCANNER.ino → Find LCD address
2. Update config_wifi.h with LCD address
3. Upload arduino_wifi_mqtt_lcd.ino
4. Open Serial Monitor
5. Look for: "WiFi connected"
6. Press button: Should display temperature
7. Check dashboard: http://raspberrypi.local:5000
```

---

## Hardware Cost Breakdown

```
LCD1602 I2C Module     €3-5
Push Button           €0.50-1
Wires & Connectors    €0.50
Debounce Resistor     €0.10

Total:                €4-7  ✅
```

---

## I2C Address Jumper Settings

| Jumper Config | PCF8574 | PCF8574A |
|---|---|---|
| None set | 0x20 | 0x38 |
| A0 set | 0x21 | 0x39 |
| A1 set | 0x22 | 0x3A |
| A2 set | 0x24 | 0x3C |
| **All set** | **0x27** | **0x3F** |

Most modules: A0+A1+A2 all grounded = 0x27

---

## Button Pin Options

| Pin | Interrupt | Type | Usage |
|---|---|---|---|
| **2** | INT0 | Hardware | ✅ Recommended |
| **3** | INT1 | Hardware | ✅ Alternative |
| 4-13 | PCINT | Software | ⚠️ Not recommended |

---

## Quick Diagnostic

```
Copy-paste this into Serial Monitor input:

Test 1 - I2C present?
→ Upload I2C_SCANNER.ino
→ Look for: "I2C device found at address"

Test 2 - LCD working?
→ Upload LCD test sketch (in LCD_TESTING_GUIDE.md)
→ Look for: Text on LCD screen

Test 3 - Button working?
→ Upload button test sketch
→ Press button
→ Look for: "Button press #1" in Serial Monitor

Test 4 - WiFi working?
→ Upload arduino_wifi_mqtt_lcd.ino
→ Look for: "WiFi connected!"

Test 5 - MQTT working?
→ On Raspberry Pi, run:
→ mosquitto_sub -h localhost -t "beehive/#" -v
→ Should see data arrive every 2 hours
```

---

## Serial Monitor Checklist

When debugging, check Serial Monitor for these messages:

```
✅ Success Path:
[INF] ArduiBeeScale - WiFi + MQTT Edition
[INF] System initializing...
[INF] WiFi SSID: your_ssid
[INF] WiFi connected!
[INF] MQTT connected!
[INF] Data published successfully!

❌ Problem Path:
[ERR] WiFi failed to connect
→ Check SSID/password in config_wifi.h

[ERR] MQTT connection failed
→ Check MQTT_BROKER IP in config_wifi.h
→ Verify Mosquitto running on Raspberry Pi

[ERR] No sensor data
→ Check HX711, DHT22, battery connections

[INF] Button pressed!
→ If not showing: Button wiring issue
```

---

## What To Do Next

1. **Find I2C Address** (5 min)
   - Upload `I2C_SCANNER.ino`
   - Note the address (0x27 or 0x3F)

2. **Configure System** (5 min)
   - Copy config_wifi_template.h → config_wifi.h
   - Update SSID, password, Pi IP, LCD address

3. **Wire Hardware** (15 min)
   - Connect 4 I2C wires
   - Connect button (Pin 2 + GND)

4. **Test Components** (30 min)
   - Follow LCD_TESTING_GUIDE.md phases 1-3
   - Verify LCD displays correctly
   - Verify button works

5. **Deploy System** (1-2 hours)
   - Full hardware assembly
   - Upload final code
   - Test WiFi + MQTT + LCD integration

---

## Emergency Troubleshooting

**"System won't compile"**
→ Check for typos in config_wifi.h
→ Ensure all libraries installed

**"Upload fails"**
→ Try different USB cable
→ Try different USB port
→ Select correct Arduino board: Tools → Board → Arduino UNO

**"Serial Monitor shows garbage"**
→ Wrong baud rate (select 9600)
→ Try different USB cable

**"Nothing works!"**
→ Start with I2C_SCANNER.ino (Phase 2)
→ Verify each component independently
→ Then integrate step by step

---

## Support Resources

| Resource | Contents |
|---|---|
| **LCD_HARDWARE_SETUP.md** | Detailed wiring, configuration, troubleshooting |
| **LCD_TESTING_GUIDE.md** | 7-phase testing procedures, diagnostics |
| **config_wifi_template.h** | All configuration options with comments |
| **arduino_wifi_mqtt_lcd.ino** | Main code with inline documentation |
| **README.md** | Full system architecture overview |

---

## Last-Minute Checklist Before Deployment

```
Before putting system in field:

Software:
□ Compiled without errors
□ config_wifi.h has correct SSID, password, Pi IP
□ LCD_I2C_ADDRESS matches your module
□ SCALE calibration values entered

Hardware:
□ All I2C wires soldered (VCC, GND, SDA, SCL)
□ Button connected (Pin 2, GND)
□ All sensors connected (HX711, DHT22, battery)
□ WiFi antenna on ESP-01
□ Fresh batteries installed (4x AA)
□ Enclosure waterproofed (if outdoor)

Testing:
□ Serial Monitor shows WiFi connected
□ Serial Monitor shows MQTT published
□ Button turns on LCD display
□ Dashboard shows data arriving
□ Battery voltage reading reasonable

Deployment:
□ WiFi signal strong at location (-65 dBm or better)
□ Raspberry Pi powered and on WiFi at home
□ System in weatherproof enclosure (if outdoor)
□ Power supply secure (batteries or USB cable)
□ All connections double-checked
```

---

**Version**: 1.0 | **Date**: 2025-11-16 | **Status**: Ready ✅

