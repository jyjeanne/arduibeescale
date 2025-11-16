# LCD1602 I2C Display Testing & Verification Guide

**Project**: ArduiBeeScale with WiFi + MQTT + LCD1602
**Date**: 2025-11-16
**Status**: Complete Testing Procedures

---

## Overview

This guide provides step-by-step testing procedures to verify your LCD1602 display, push button, and integrated system before full deployment.

---

## Phase 1: Pre-Wiring Verification (15 minutes)

Before connecting anything, verify your components:

### LCD Module Inspection

```
Visual Checklist:
□ LCD1602 module intact (no cracks, no burnt components)
□ I2C backpack soldered to LCD (check all 4 solder joints)
□ Address jumpers visible on I2C backpack
□ No loose solder bridges or components
□ LCD contrast potentiometer visible (small blue box)
```

### Push Button Inspection

```
Visual Checklist:
□ Push button tactile switch intact
□ 4 pins visible (2x2 arrangement)
□ No bent or broken pins
□ Button cap/cover intact
□ Electrical continuity confirmed (will test later)
```

### Wire Quality Check

```
Visual Checklist:
□ 4 I2C wires separate and not tangled
□ Color-coded: Red (5V), Black (GND), Green (SDA), Yellow (SCL)
□ Wires not damaged or frayed
□ Appropriate gauge (22-24 AWG for I2C)
□ 2-3 conductor button wires available
```

---

## Phase 2: I2C Address Detection (20 minutes)

### Step 1: Upload I2C Scanner Sketch

```bash
1. Open Arduino IDE
2. Open File → New
3. Copy contents of I2C_SCANNER.ino (provided)
4. Click Sketch → Verify (should compile without errors)
5. Click Sketch → Upload
6. Wait for "Done uploading" message
```

### Step 2: Connect LCD Module ONLY

At this stage, only connect the I2C wires:

```
Arduino ↔ LCD1602 I2C Module
├─ 5V (Arduino pin +5V) → VCC (red wire)
├─ GND (Arduino pin GND) → GND (black wire)
├─ A4 (Arduino pin A4) → SDA (green wire)
└─ A5 (Arduino pin A5) → SCL (yellow wire)

DO NOT connect button yet
```

### Step 3: Run I2C Scanner

```bash
1. Open Serial Monitor (Tools → Serial Monitor)
2. Set baud rate: 9600
3. You should see:

   ┌────────────────────────────────┐
   │ I2C Scanner                    │
   │ ====================           │
   │ Scanning I2C bus...            │
   │                                │
   │ I2C device found at address 0x27
   │ ^ This looks like your LCD1602 │
   │   address!                     │
   │                                │
   │ Found 1 device(s)              │
   │ ====================           │
   │ Scan complete!                 │
   └────────────────────────────────┘
```

### Step 4: Record Your Address

```
Note your I2C address: __________ (write here)

Common values:
□ 0x27 (most common - blue backlight)
□ 0x3F (green backlight or some variants)
□ Other: ________________
```

### Troubleshooting: No Device Found

**If you see "No I2C devices found":**

1. **Check VCC (5V) connection**:
   - Use multimeter: probe Arduino 5V and LCD VCC
   - Should read 4.8-5.2V
   - If <4.5V: Check USB power, try different cable

2. **Check GND connection**:
   - Use multimeter: probe Arduino GND and LCD GND
   - Continuity should exist (beep)
   - If no beep: Wire connection is broken, resolder

3. **Check I2C wires (SDA and SCL)**:
   - Wiggle wires while watching Serial Monitor
   - If address appears and disappears: Loose connection
   - Resolder or use crimp connectors for secure contact

4. **Try different I2C addresses**:
   - Edit I2C_SCANNER.ino, change address range
   - Some modules use non-standard addresses
   - Add debugging: higher voltage, different power source

5. **Power cycle LCD**:
   - Unplug USB completely (wait 10 seconds)
   - Plug back in
   - Re-run I2C scanner

6. **Test with simple code**:
   ```cpp
   // Add this to I2C_SCANNER.ino loop()
   Serial.print(".");
   delay(500);
   // If you see dots, code is running, I2C issue
   ```

---

## Phase 3: LCD Display Test (30 minutes)

### Step 1: Update Configuration

Edit `config_wifi.h`:

```cpp
#define LCD_I2C_ADDRESS    0x27  // ← Use YOUR address from Phase 2
#define ENABLE_LCD         true
#define BUTTON_PIN         2     // We'll connect button next
```

### Step 2: Upload LCD Test Sketch

Create a minimal test sketch to verify LCD display:

```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // ← Update to YOUR address

void setup() {
  Serial.begin(9600);

  Serial.println("Initializing LCD...");

  Wire.begin();
  lcd.init();
  lcd.backlight();

  Serial.println("LCD initialized!");

  // Test sequence
  lcd.clear();
  lcd.print("BeezScale v3.1");
  delay(2000);

  lcd.clear();
  lcd.print("Temperature:");
  lcd.setCursor(0, 1);
  lcd.print("23.50C");
  delay(2000);

  lcd.clear();
  lcd.print("Humidity:");
  lcd.setCursor(0, 1);
  lcd.print("65%");
  delay(2000);

  lcd.clear();
  lcd.print("Weight: 32.45kg");
  delay(2000);

  lcd.clear();
  lcd.print("Battery: 75%");
  lcd.setCursor(0, 1);
  lcd.print("5.40V");
  delay(2000);

  lcd.clear();
  lcd.noBacklight();
  Serial.println("Test complete!");
}

void loop() {
  delay(1000);
}
```

### Step 3: Expected Output

**Serial Monitor** (9600 baud):
```
Initializing LCD...
LCD initialized!
Test complete!
```

**LCD Display** (cycling through):
```
Screen 1 (2 sec):  | BeezScale v3.1 |
                   |                |

Screen 2 (2 sec):  | Temperature:   |
                   | 23.50C         |

Screen 3 (2 sec):  | Humidity:      |
                   | 65%            |

Screen 4 (2 sec):  | Weight: 32.45kg|
                   |                |

Screen 5 (2 sec):  | Battery: 75%   |
                   | 5.40V          |

Screen 6:          | (backlight off)|
```

### Troubleshooting: LCD Display Issues

**Problem: LCD shows nothing (no backlight)**

- [ ] Verify VCC is 5V (use multimeter)
- [ ] Try different LCD address (0x3F instead of 0x27)
- [ ] Resolder I2C backpack connections
- [ ] Try different USB power (AC adapter instead of computer)

**Problem: LCD has backlight but no text**

- [ ] Wrong I2C address - change 0x27 to 0x3F
- [ ] Loose SDA/SCL wires - resolder both
- [ ] Corrupt LCD module - replace module
- [ ] Try contrast potentiometer (blue box on back): use small screwdriver to adjust (only if needed)

**Problem: Garbled text or "?????" characters**

- [ ] Address is wrong - run I2C_SCANNER again
- [ ] Power supply too weak - check USB power level
- [ ] Power noise - ensure good GND connections
- [ ] Replace LCD module

**Problem: Only partial text visible**

- [ ] Too many characters for one line (16 max)
- [ ] Check `lcd.print()` length in code
- [ ] Use `lcd.setCursor()` to position text properly

---

## Phase 4: Push Button Testing (20 minutes)

### Step 1: Connect Button

Wire the push button:

```
Arduino Pin 2 ←→ Button Pin 1
Arduino GND   ←→ Button Pin 2

Visual diagram:
        [Button Top View]
           A      B
          [1]    [2]
           │      │
           └──┬───┘
           C  D  E  (pins)

Connect A to Arduino Pin 2
Connect C (or D, same row) to Arduino GND
```

### Step 2: Upload Button Test Sketch

```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // ← Update to YOUR address

volatile int buttonPressCount = 0;
unsigned long lastButtonPress = 0;

void setup() {
  Serial.begin(9600);

  Wire.begin();
  lcd.init();
  lcd.backlight();

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), handleButton, FALLING);

  Serial.println("Button test started. Press button to increment counter.");

  lcd.clear();
  lcd.print("Button Test");
  lcd.setCursor(0, 1);
  lcd.print("Presses: 0");
}

void handleButton() {
  unsigned long now = millis();
  if (now - lastButtonPress < 50) {  // Debounce 50ms
    return;
  }
  lastButtonPress = now;
  buttonPressCount++;

  Serial.print("Button press #");
  Serial.println(buttonPressCount);
}

void loop() {
  // Update LCD with press count
  lcd.setCursor(9, 1);

  if (buttonPressCount < 10) {
    lcd.print(" ");
  }

  lcd.print(buttonPressCount);

  delay(100);
}
```

### Step 3: Test Button Presses

```bash
Action:              Expected Result:
─────────────────────────────────────────────────────
1. Press button once │ Serial Monitor: "Button press #1"
                     │ LCD: "Presses: 1"
                     │
2. Press again       │ Serial Monitor: "Button press #2"
                     │ LCD: "Presses: 2"
                     │
3. Hold button down  │ Should count once, not multiple times
                     │ (debounce working)
                     │
4. Rapid presses (5) │ Should count 5, not 6-10
                     │ (no extra counts from bouncing)
```

### Expected Serial Output

```
Button test started. Press button to increment counter.
Button press #1
Button press #2
Button press #3
Button press #4
Button press #5
```

### Troubleshooting: Button Not Working

**Problem: Button press not registered**

- [ ] Check wire from Arduino Pin 2 to button - resolder if needed
- [ ] Check wire from button to GND - resolder if needed
- [ ] Try Pin 3 (INT1) instead of Pin 2:
  ```cpp
  attachInterrupt(digitalPinToInterrupt(3), handleButton, FALLING);
  pinMode(3, INPUT_PULLUP);
  ```
- [ ] Verify button is NOT pull-down type (should be pull-up for this circuit)

**Problem: Button counts multiple presses for single press**

- [ ] Debounce delay too short (change 50 to 75 or 100)
- [ ] Add 10kΩ debounce resistor (optional, parallel with button)
- [ ] Check Serial Monitor - sometimes display catches previous state

**Problem: Button counts even when not pressed**

- [ ] Floating input (pin not properly grounded)
- [ ] Add capacitor 0.1µF between pin and GND (optional)
- [ ] Check for nearby noise source (WiFi, power supply)

---

## Phase 5: Integrated Display State Machine Testing (30 minutes)

### Step 1: Update config_wifi.h

```cpp
#define LCD_I2C_ADDRESS         0x27   // Your address from Phase 2
#define ENABLE_LCD              true
#define BUTTON_PIN              2
#define DISPLAY_DURATION_MS     2000   // 2 seconds per screen
```

### Step 2: Upload Full WiFi+MQTT+LCD Code

```bash
1. Open arduino_wifi_mqtt_lcd.ino
2. Verify it compiles: Sketch → Verify ✅
3. Upload: Sketch → Upload
4. Wait for "Done uploading"
```

### Step 3: Test Display State Machine

**Action**: Press button once

**Expected Sequence**:
```
Time    LCD Display              Serial Monitor Output
────────────────────────────────────────────────────────
0s      (off, waiting)           [INF] System initializing...
                                 [INF] WiFi Connecting...

Press Button →

2s      Temp: 23.5C Hum: 65%    [INF] Button pressed!
        (or similar values)      [INF] Reading sensors...
                                 [INF] Backlight ON
                                 [INF] Display: TEMP_HUMID

4s      Weight: 32.45kg          [INF] Display: WEIGHT
        (or similar values)

6s      Battery: 75%             [INF] Display: BATTERY
        5.40V

8s      (backlight off)          [INF] Display: OFF
        (LCD dark)               [INF] Backlight OFF
```

### Expected LCD Output Details

**Screen 1 (Temperature & Humidity - Line 1):**
```
Expected format: "Temp: XX.XC HX%"
Example: "Temp: 23.5C 65%"

Actual output depends on sensor readings
```

**Screen 2 (Weight - Full Display):**
```
Expected format: "Weight: XX.XXkg"
Example: "Weight: 32.45kg"

Format: "Weight: " + value with 2 decimals + "kg"
```

**Screen 3 (Battery - Line 1):**
```
Expected format: "Battery: XX%"
Example: "Battery: 75%"

Second line may show voltage: "Voltage: 5.40V"
```

### Troubleshooting: Display State Issues

**Problem: Button press doesn't turn on display**

- [ ] Check button wiring (Phase 4 testing)
- [ ] Check ENABLE_LCD is true in config_wifi.h
- [ ] Check Serial Monitor for "Button pressed!" message
- [ ] Verify interrupt handler is attached in setup()

**Problem: Display shows only partial information**

- [ ] Screen too crowded for 16 characters
- [ ] Check formatting in displayTempHumidity(), displayWeight(), displayBattery()
- [ ] May need to split across two lines

**Problem: Display times don't match DISPLAY_DURATION_MS**

- [ ] Check millis() counter (may be off)
- [ ] Verify DISPLAY_DURATION_MS value in config_wifi.h
- [ ] System delays during WiFi/MQTT may affect timing

**Problem: Backlight doesn't turn off after sequence**

- [ ] Check lcdBacklightOff() function is called
- [ ] Verify last state transition sets currentDisplayState = STATE_OFF
- [ ] May need manual backlight disable in code

---

## Phase 6: Battery Percentage Calculation Testing (20 minutes)

### Step 1: Verify Voltage Divider

The battery voltage is measured on pin A0 via a voltage divider.

```
Expected setup:
Battery + (5-6V) → 5kΩ resistor → A0 → 1kΩ resistor → GND

Voltage at A0 = Battery_Voltage × (1kΩ / (5kΩ + 1kΩ))
              = Battery_Voltage × (1/6)
              = Battery_Voltage × 0.167
```

### Step 2: Measure Battery Voltage

```bash
1. Unplug USB from Arduino (use battery power only)
2. Use multimeter: Set to voltage (V) mode
3. Measure between Battery+ and GND: ________V
4. Calculate A0 voltage: ______V × 0.167 = ______V

Expected ranges (4x AA batteries):
├─ Fresh batteries: 6.0-6.1V total → ~1.0V at A0
├─ New batteries: 5.5-5.8V total → ~0.92V at A0
├─ Used batteries: 4.5-5.0V total → ~0.75-0.83V at A0
├─ Depleted: 3.0-3.5V total → ~0.5-0.58V at A0
└─ Dead: <3.0V total → <0.5V at A0
```

### Step 3: Upload Battery Test Sketch

```cpp
void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read raw analog value
  int rawValue = analogRead(A0);

  // Convert to voltage (5V reference, 10-bit ADC)
  // ADC: 0-1023 represents 0-5V
  float voltage = (rawValue / 1023.0) * 5.0;

  // This is voltage at A0 (divided voltage)
  // To get battery voltage, multiply by 6 (because 1/(1+5) divider)
  float batteryVoltage = voltage * 6.0;

  // Calculate percentage
  float batteryPercent = ((batteryVoltage - 3.0) / (6.0 - 3.0)) * 100.0;
  if (batteryPercent < 0) batteryPercent = 0;
  if (batteryPercent > 100) batteryPercent = 100;

  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print(" | A0V: ");
  Serial.print(voltage, 2);
  Serial.print("V | Battery: ");
  Serial.print(batteryVoltage, 2);
  Serial.print("V | Percent: ");
  Serial.print(batteryPercent, 0);
  Serial.println("%");

  delay(1000);
}
```

### Step 4: Verify Measurements

```
Expected Serial Output (with fresh batteries):

Raw: 820 | A0V: 4.01V | Battery: 6.01V | Percent: 100%
Raw: 820 | A0V: 4.01V | Battery: 6.01V | Percent: 100%
...
```

### Step 5: Create Battery Calibration Table

Measure and record different battery states:

| Battery Condition | Measured V | A0 Voltage | Percent | Status |
|---|---|---|---|---|
| Fresh AA (x4) | ______V | ______V | ___% | ✅ |
| One week use | ______V | ______V | ___% | ✅ |
| One month use | ______V | ______V | ___% | ✅ |
| Two month use | ______V | ______V | ___% | ⚠️ |
| Three+ months | ______V | ______V | ___% | ❌ |

Use this data to validate battery percentage accuracy.

### Troubleshooting: Battery Percentage Issues

**Problem: Battery always shows 100%**

- [ ] Battery voltage is actually >6.0V (replace batteries)
- [ ] Voltage divider resistors are wrong value
- [ ] Check 5kΩ and 1kΩ resistor values with multimeter

**Problem: Battery always shows 0%**

- [ ] Battery voltage is actually <3.0V (dead batteries)
- [ ] Voltage divider disconnected
- [ ] Check A0 wire connection to middle of divider

**Problem: Battery percentage jumps erratically**

- [ ] Noisy power supply (use USB hub with better power)
- [ ] A0 wire near noise source (WiFi antenna, power lines)
- [ ] Add 10µF capacitor to A0 for stability:
  ```
  A0 ─┬── 10kΩ ── GND
      │
    10µF (to GND)
  ```

---

## Phase 7: Full System Integration Test (1-2 hours)

### Pre-Test Checklist

```
All Previous Phases Passed:
□ Phase 2: I2C address found and working
□ Phase 3: LCD displays test sequence correctly
□ Phase 4: Button presses registered
□ Phase 5: Display state machine cycles properly
□ Phase 6: Battery voltage reading accurate

Hardware Fully Connected:
□ LCD I2C wires connected (VCC, GND, SDA, SCL)
□ Push button connected (Pin 2, GND)
□ All sensors connected (HX711, DHT22, Battery A0)
□ ESP-01 WiFi module connected via level shifter
□ 4x AA battery holder connected

Configuration Updated:
□ WIFI_SSID updated in config_wifi.h
□ WIFI_PASSWORD updated in config_wifi.h
□ MQTT_BROKER IP updated (your Raspberry Pi)
□ LCD_I2C_ADDRESS correct (0x27 or 0x3F)
□ BUTTON_PIN correct (2 or 3)
□ SCALE_CALIBRATION calibrated
□ SCALE_OFFSET calibrated
```

### Full System Test Procedure

**Step 1: Upload Final Arduino Code**

```bash
1. Verify config_wifi.h has all correct values
2. Upload arduino_wifi_mqtt_lcd.ino
3. Open Serial Monitor (9600 baud)
4. Check startup messages
```

**Step 2: Verify WiFi Connection**

```
Expected Serial Output:
[INF] ArduiBeeScale - WiFi + MQTT Edition
[INF] System initializing...
[INF] WiFi SSID: your_ssid
[INF] MQTT Broker: 192.168.1.100
[INF] Starting first measurement...
[INF] ========== Starting Measurement Cycle ==========
[INF] Reading sensors...
[INF] Weight (kg): XX.XX
[INF] Temperature (C): XX.XX
[INF] Humidity (%): XX.XX
[INF] Battery (V): X.XX
[INF] Connecting to WiFi...
[INF] WiFi connected!
```

**Step 3: Verify MQTT Connection**

```
Expected Serial Output (after WiFi):
[INF] Connecting to MQTT broker...
[INF] MQTT connected!
[INF] Publishing data to MQTT...
[INF] Data published successfully!
```

**Step 4: Test LCD Display via Button Press**

```bash
Action: Press button
Expected:
- Serial Monitor: "[INF] Button pressed!"
- LCD: Backlight turns on
- LCD Line 1: "Temp: XX.XC YY%" (values from sensors)
- LCD Line 2: (varies by implementation)
- After 2 sec: "Weight: XX.XXkg"
- After 2 sec: "Battery: XX% X.XXV"
- After 2 sec: Backlight off
```

**Step 5: Verify MQTT Message Arrival**

On Raspberry Pi:
```bash
mosquitto_sub -h localhost -t "beehive/#" -v

Expected output:
beehive/hive-001 {"temperature":23.50,"humidity":65.00,"weight":32.45,"battery_voltage":5.40,"battery_percent":75}
```

**Step 6: Check Dashboard Data**

On your computer:
```
1. Open: http://raspberrypi.local:5000
2. Select hive from dropdown (if multiple)
3. Verify stat cards show data:
   ✅ Temperature: XX.XX°C
   ✅ Humidity: XX.XX%
   ✅ Weight: XX.XX kg
   ✅ Battery: XX% (XX.XXV)
```

### Success Criteria

✅ **System is Production Ready if:**
- [ ] WiFi connects within 30 seconds
- [ ] MQTT publishes successfully
- [ ] Serial Monitor shows no errors
- [ ] LCD displays all three screens correctly
- [ ] Button press cycles through display
- [ ] Dashboard shows real-time data
- [ ] Battery percentage accurate (within ±5%)
- [ ] No console error messages

---

## Post-Deployment Monitoring

### Daily Checks (First Week)

```
□ Dashboard shows new data every 2 hours
□ No WiFi disconnection messages in logs
□ Battery percentage decreases slowly
□ LCD display works when button pressed
□ No sensor reading anomalies
```

### Weekly Checks

```
□ Battery voltage trending downward (expected)
□ Dashboard historical data visible
□ No repeated error messages
□ MQTT connection stable
□ WiFi signal strength consistent
```

### Monthly Checks

```
□ Backup database created
□ Battery still >50% capacity
□ All sensors reading valid data
□ No LCD display anomalies
□ Overall system stable
```

---

## Common Issues Summary

| Phase | Issue | Quick Fix |
|---|---|---|
| 2 | No I2C device found | Check VCC, GND, and SDA/SCL wires |
| 3 | LCD blank | Try different I2C address (0x3F) |
| 3 | Garbled text | Check address is correct, resolder wires |
| 4 | Button not responding | Check Pin 2 connection, resolder |
| 4 | Multiple counts per press | Increase debounce delay to 100ms |
| 5 | Display won't turn on | Verify button works (Phase 4) |
| 6 | Wrong battery % | Recalibrate voltage divider |
| 7 | MQTT not publishing | Check Mosquitto running, verify IP |
| 7 | Dashboard no data | Check MQTT topic matches Arduino config |

---

## Next Steps After Testing

Once all tests pass:

1. ✅ **Deploy to Field**: Place system at hive location with fresh batteries
2. ✅ **Monitor First 24 Hours**: Check dashboard updates every 2 hours
3. ✅ **Verify Battery Stability**: Watch for unexpected voltage drops
4. ✅ **Document Performance**: Note actual battery life vs. estimated
5. ✅ **Schedule Maintenance**: Plan monthly backups and battery replacement

---

**Last Updated**: 2025-11-16
**Version**: 1.0
**Status**: Complete ✅

