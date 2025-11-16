# LCD1602 I2C Display Hardware Setup Guide

**Project**: ArduiBeeScale with WiFi + MQTT + LCD1602
**Date**: 2025-11-16
**Status**: Complete Implementation Ready

---

## Quick Overview

This guide covers adding a **16x2 character LCD display** with I2C backpack and a **push button** to your ArduiBeeScale system. The display cycles through three screens when you press the button:

1. **Temperature & Humidity** (2 seconds)
2. **Weight in Kilograms** (2 seconds)
3. **Battery Percentage** (2 seconds)
4. **Display turns off** (waits for next button press)

---

## Parts Required

### LCD Display Module
| Part | Cost | Notes |
|------|------|-------|
| LCD1602 16x2 I2C Display | €3-5 | Blue or green backlight |
| I2C Backpack Module | Included | Converts 4-wire to I2C |
| **Total** | **€3-5** | Standard item |

### Button & Connections
| Part | Cost | Notes |
|------|------|-------|
| Push Button (momentary) | €0.50-1 | 6mm x 6mm tactile switch |
| 10kΩ resistor | €0.10 | Optional, for debounce |
| Connecting wires | €0.50 | 4-conductor bundle |
| **Total** | **€1-2** | Most Arduino kits include these |

### Complete LCD System Cost
```
LCD1602 I2C Module:        €3-5
Push Button + Wires:       €1-2
Level shifter (if needed): €1-3
─────────────────────────
TOTAL:                     €5-10 ✅
```

---

## Wiring Diagram

### Arduino to LCD1602 I2C Module

```
Arduino UNO          LCD1602 I2C Module
┌──────────────┐     ┌──────────────┐
│              │     │ (Back of LCD)│
│ 5V (pin +5V) ├─────┤ VCC (red)    │
│              │     │              │
│ GND (pin GND)├─────┤ GND (black)  │
│              │     │              │
│ A4 (SDA)     ├─────┤ SDA (green)  │
│              │     │              │
│ A5 (SCL)     ├─────┤ SCL (yellow) │
│              │     │              │
└──────────────┘     └──────────────┘
```

### Pin Connection Table

| Arduino Pin | I2C Module Pin | Wire Color | Purpose |
|-------------|---|---|---|
| 5V | VCC | Red | Power (5V) |
| GND | GND | Black | Ground |
| A4 | SDA | Green | I2C Data |
| A5 | SCL | Yellow | I2C Clock |

**IMPORTANT**: I2C communicates at 3.3V logic, but the backpack's VCC can accept 5V. Arduino's A4 and A5 have internal pull-up resistors, so this is safe.

---

### Push Button Wiring

```
Push Button         Arduino
┌─────────────┐     ┌─────────────┐
│    Pin 1    ├─────┤ Pin 2 (INT0)│
│             │     │             │
│    Pin 2    ├─────┤ GND         │
└─────────────┘     └─────────────┘
```

**Button Pin Connection**:

| Button Pin | Arduino Pin | Notes |
|-------------|---|---|
| Pin 1 (any) | Digital Pin 2 | Input pin (uses INT0 interrupt) |
| Pin 2 (other) | GND | Ground return |

**Debounce Resistor** (optional but recommended):
- Connect 10kΩ resistor from Pin 2 to GND (in parallel with button connection to GND)
- Helps eliminate electrical noise
- Software debounce (50ms) is also implemented in code

---

## Step-by-Step Wiring Instructions

### Phase 1: Identify Your LCD Module Type

Before wiring, determine your LCD's I2C address:

**Most Common Addresses**:
- **0x27** - Blue backlight modules (most common)
- **0x3F** - Green backlight or some variants

You'll need to find your address. We provide an **I2C Scanner sketch** for this.

### Phase 2: Connect I2C Wires

1. **Wire VCC** (Power):
   - Red wire from Arduino 5V → LCD VCC
   - Use a short wire (2-3 cm) to minimize voltage drop

2. **Wire GND** (Ground):
   - Black wire from Arduino GND → LCD GND
   - Make sure this is a solid connection

3. **Wire SDA** (I2C Data):
   - Green wire from Arduino A4 → LCD SDA
   - This is the data line for I2C communication

4. **Wire SCL** (I2C Clock):
   - Yellow wire from Arduino A5 → LCD SCL
   - This is the clock line for I2C communication

**Verification**: After connecting I2C wires, the LCD should light up with a white or blue backlight (with or without text visible).

### Phase 3: Connect Push Button

1. **Identify Button Pins**:
   - Push buttons have 4 pins (2x2 grid)
   - Diagonal pins are connected internally
   - You need any two pins on the same side OR opposite diagonal

2. **Connect Button**:
   - One button pin → Arduino Pin 2
   - Other button pin → Arduino GND
   - Use short wires (2-3 cm)

3. **Optional Debounce Resistor**:
   - Connect 10kΩ resistor from Pin 2 to GND (in parallel)
   - Solders to back of push button module if available
   - Not required (software debounce works fine)

### Phase 4: Verify Connections

Before uploading code:

```
Checklist:
□ LCD I2C VCC connected to Arduino 5V
□ LCD I2C GND connected to Arduino GND
□ LCD I2C SDA connected to Arduino A4
□ LCD I2C SCL connected to Arduino A5
□ LCD has blue/white backlight (power test)
□ Button pin 1 connected to Arduino Pin 2
□ Button pin 2 connected to Arduino GND
□ All solder joints are clean and shiny
□ No wires touching or crossed
```

---

## Finding Your LCD I2C Address

Most LCD1602 I2C modules use **0x27** or **0x3F**. The address is printed on the I2C backpack module (small chip with 8 legs).

### Method 1: I2C Scanner Sketch (Recommended)

Upload the provided **I2C_SCANNER.ino** sketch:

```bash
1. Open Arduino IDE
2. Open: File → New
3. Copy I2C_SCANNER.ino contents (provided separately)
4. Click Upload
5. Open Serial Monitor (9600 baud)
6. Look for: "I2C device found at address 0x27" or "0x3F"
7. Note the address
8. Update LCD_I2C_ADDRESS in config_wifi.h
```

Expected Serial Output:
```
Scanning I2C bus...
I2C device found at address 0x27 (0x27)
Scanning done.
```

If you see **0x27**, update config_wifi.h:
```cpp
#define LCD_I2C_ADDRESS    0x27
```

If you see **0x3F**, update config_wifi.h:
```cpp
#define LCD_I2C_ADDRESS    0x3F
```

### Method 2: Check PCB Markings

Look at the I2C backpack (small module attached to LCD):

- Look for a small chip labeled **PCF8574** or similar
- The address is often marked as **A0**, **A1**, **A2** jumpers
- Jumpers set to ground = 0x27
- Jumpers set to high = 0x3F

Take a photo and note the address before final assembly.

---

## Configuration Steps

### Step 1: Copy Template to Config File

```bash
# In Arduino IDE:
# Copy: config_wifi_template.h → config_wifi.h
# (Or use your file explorer)
```

### Step 2: Update LCD I2C Address

Edit `config_wifi.h` and find the LCD section:

```cpp
#define ENABLE_LCD              true        // Enable LCD
#define LCD_I2C_ADDRESS         0x27        // ← UPDATE THIS with your address
```

**Change 0x27 to your found address** (0x27 or 0x3F).

### Step 3: Configure Button Pin (Optional)

Default is Pin 2. If you used a different pin, update:

```cpp
#define BUTTON_PIN              2           // ← Change if using different pin
```

Available pins for interrupt:
- **Pin 2** (INT0) - Recommended
- **Pin 3** (INT1) - Alternative

### Step 4: Adjust Display Duration (Optional)

Default is 2 seconds per screen. If you want different timing:

```cpp
#define DISPLAY_DURATION_MS     2000        // 2000 = 2 seconds
                                             // 1000 = 1 second
                                             // 3000 = 3 seconds
```

---

## Installation & Testing

### Pre-Upload Verification

Before uploading `arduino_wifi_mqtt_lcd.ino`:

1. ✅ All I2C wires connected (VCC, GND, SDA, SCL)
2. ✅ Button wires connected (Pin 2, GND)
3. ✅ LCD I2C address found and updated in config_wifi.h
4. ✅ No loose wires or cold solder joints
5. ✅ Button pin selected (default Pin 2)

### Upload Arduino Code

```bash
1. Open Arduino IDE
2. Open: arduino_wifi_mqtt_lcd.ino
3. Verify → (should compile without errors)
4. Select Board: Arduino UNO
5. Select Port: COM3 (or your port)
6. Upload
7. Wait for "Done uploading" message
```

### Initial Testing

**Test 1: LCD Display Test**
```
Expected on startup:
- LCD backlight turns on
- First line: "BeezScale v3.1"
- Second line: "WiFi Connecting..."

After WiFi connects:
- First line: "WiFi: Connected"
- Second line: "MQTT: Connecting..."
```

**Test 2: Push Button Test**
```
1. Press button once
2. LCD should turn on (if it was off)
3. Should display: "Temp: XX.XXC Hum: XX%"
4. After 2 seconds: "Weight: X.XXkg"
5. After 2 seconds: "Battery: XX%"
6. After 2 seconds: LCD turns off
7. Press button again to repeat

If nothing happens:
- Check Pin 2 connection to button
- Check button GND connection
- Check LCD_I2C_ADDRESS in config_wifi.h
- Check Serial Monitor for errors
```

**Test 3: Data Display Verification**
```
When displaying temperature/humidity (2 sec):
□ Temperature value appears (XX.XX°C or XX.XXC)
□ Humidity value appears (XX%)
□ Both on same line (16 characters max)

When displaying weight (2 sec):
□ Weight value appears (X.XXkg or X.XXKg)
□ Properly formatted

When displaying battery (2 sec):
□ Battery percentage appears (XX%)
□ Battery voltage appears (X.XXV)
□ Both on same line
```

---

## Display Output Examples

### Screen 1: Temperature & Humidity
```
┌────────────────┐
│Temp: 22.5C 65%│  (Example)
│Hum:           │  (If second line needed)
└────────────────┘
```

### Screen 2: Weight
```
┌────────────────┐
│Weight: 32.45kg │  (Example)
│                │
└────────────────┘
```

### Screen 3: Battery Status
```
┌────────────────┐
│Battery: 75%    │  (Example)
│Voltage: 5.40V  │  (Example)
└────────────────┘
```

---

## Troubleshooting

### Problem: LCD Shows Nothing (No Backlight)

**Symptoms**: Blank LCD, no backlight

**Solutions**:
1. Check VCC wire (5V) - use multimeter to verify 5V at LCD VCC pin
2. Check GND wire - ensure solid ground connection
3. Try both I2C addresses: 0x27 and 0x3F
4. Upload I2C_SCANNER.ino to verify LCD is detected
5. Try: Restart Arduino, re-upload code

### Problem: LCD Shows Backlight but No Text

**Symptoms**: Blue/white backlight visible, but no characters

**Solutions**:
1. Wrong I2C address - run I2C_SCANNER.ino to find correct address
2. Wrong pins - verify A4 (SDA) and A5 (SCL) connections
3. Pull-up resistors - Arduino A4/A5 have internal pull-ups (usually fine)
4. Try: Unplug and replug USB, wait 5 seconds, re-upload

### Problem: Button Press Not Working

**Symptoms**: Press button, nothing happens

**Solutions**:
1. Check button wires - are they loose or cold-soldered?
2. Check button polarity - try swapping the two wires
3. Check Pin 2 - some Arduino clones use different pin for INT0
4. Check Serial Monitor for button press logging
5. Try: Press button firmly for 1-2 seconds
6. Upload I2C test sketch, ignore errors, still test button

### Problem: Display Shows "?????" Characters

**Symptoms**: LCD shows garbled characters instead of text

**Solutions**:
1. Wrong I2C address (0x27 vs 0x3F)
2. Loose I2C wires (SDA or SCL)
3. Power supply too weak - use direct USB power, not extension
4. Bad LCD module - try replacing LCD

### Problem: WiFi Connects but LCD Still Blank

**Symptoms**: Serial Monitor shows WiFi/MQTT OK, LCD inactive

**Solutions**:
1. Check ENABLE_LCD is true in config_wifi.h
2. Check LCD_I2C_ADDRESS matches your LCD
3. Press button - LCD might be in sleep state
4. Check button Pin 2 connection
5. Look for Serial Monitor errors

### Problem: Battery Percentage Shows 0% or 100%

**Symptoms**: Battery always shows same percentage, no change

**Solutions**:
1. Check battery voltage divider connections (pin A0)
2. Verify your battery voltage: measure with multimeter
3. Calibration values incorrect - battery should be 3.0V min, 6.0V max
4. Edit `calculateBatteryPercent()` function in code if needed

---

## Detailed Pin Reference

### Arduino Pins Used

| Pin | Type | Purpose | Conflict Risk |
|-----|------|---------|---|
| A4 | Analog/I2C SDA | LCD data | Battery reading uses A0 (no conflict) |
| A5 | Analog/I2C SCL | LCD clock | Same as above |
| Pin 2 | Digital/INT0 | Button input | Used for interrupt (no soft serial conflict) |
| 5V | Power | LCD VCC | Shared with other components (OK) |
| GND | Power | LCD GND | Shared with other components (OK) |

### Arduino Pins NOT Used (Available)
- D0, D1 (serial, don't touch)
- D3, D4, D7, D11, D12, D13 (available)
- A1, A2, A3 (available)

---

## Schematic Diagram

```
                 ┌─────────────────────────────────────┐
                 │      Arduino UNO Connections         │
                 └─────────────────────────────────────┘

┌─ 5V ─────────────────────────────────────────────────────┐
│                                                          │
│  ┌──────────────────┐              ┌──────────────────┐ │
│  │ LCD1602 I2C      │              │ Push Button      │ │
│  │ Module           │              │                  │ │
│  │                  │              │ Pin1 ────────┐   │ │
│  │ VCC (5V) ────────┤              │              │   │ │
│  │ GND ─────────┐   │              │ Pin2 ─────┬──┘   │ │
│  │ SDA (A4) ┐   │   │              │           │      │ │
│  │ SCL (A5) ┤   │   │              └───────────┼──────┘ │
│  └──────────┤   │   │                          │        │
│             │   │   │              ┌───────────┘        │
│             │   └───┴──────────┐    │                   │
│             │                  │    │                   │
│             │  Arduino A4(SDA) ├─┐  │              Digital Pin 2
│             │  Arduino A5(SCL) ├─┤  │              (INT0)
│             │  Arduino GND     ├─┴──┼──────────────────┤
│             │  Arduino 5V      ├────┘
│             │
└─────────────┴────────────────────────────────────────────┘

Legend:
═══ = Power (5V)
─── = Ground (GND)
─•─ = I2C Data (SDA)
─⊙─ = I2C Clock (SCL)
─○─ = Digital Signal
```

---

## Testing Procedures

### Pre-Deployment I2C Test

```bash
# Test 1: Verify I2C communication
1. Upload I2C_SCANNER.ino
2. Open Serial Monitor
3. Should show: "I2C device found at address 0xXX"
4. Note the address
5. Update config_wifi.h with this address
```

### LCD-Only Test

If you have issues, test LCD in isolation:

```bash
# Create simple test sketch:

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Try 0x27 or 0x3F

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.print("LCD Test OK!");
}

void loop() {
  delay(1000);
}
```

Upload and check if "LCD Test OK!" appears.

### Button-Only Test

```bash
# Simple button test:

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), handleButton, FALLING);
}

void handleButton() {
  Serial.println("Button pressed!");
}

void loop() {
  delay(100);
}
```

Upload and open Serial Monitor. Each button press should print "Button pressed!"

---

## Maintenance

### Weekly Checks
- LCD text displays correctly
- Button press triggers display
- Backlight turns on/off
- No loose wires around I2C connections

### Monthly Maintenance
- Wipe LCD with soft cloth (no liquid)
- Check solder joints on button connections
- Verify I2C address hasn't changed

### Long-term Storage
- Remove batteries before storing
- Keep LCD away from moisture
- Store wires coiled gently (don't crimp I2C wires)

---

## FAQ

**Q: Can I use different Arduino pins for I2C?**
A: No. A4 (SDA) and A5 (SCL) are hardware I2C pins on Arduino UNO. Other boards (Mega, Nano) use different pins.

**Q: Can I use a different pin for the button?**
A: Yes, Pin 3 (INT1) also works. Other pins don't have hardware interrupts. Update `BUTTON_PIN` and `digitalPinToInterrupt()` in code.

**Q: What if I have 0x3F address instead of 0x27?**
A: Just update `LCD_I2C_ADDRESS` in config_wifi.h to `0x3F`. Everything else stays the same.

**Q: Can I use 3.3V instead of 5V for LCD?**
A: No. The I2C backpack expects 5V. The logic levels are compatible with Arduino's 3.3V I2C input (with pull-ups).

**Q: How do I turn off the button feature?**
A: Set `ENABLE_LCD` to `false` in config_wifi.h. The button pins won't be initialized.

**Q: Can I add another button?**
A: Yes, use Pin 3 (INT1) with a separate `attachInterrupt()`. Define a different button function to handle it.

---

## Quick Reference

| Setting | File | Default | Options |
|---------|------|---------|---------|
| LCD I2C Address | config_wifi.h | 0x27 | 0x27 or 0x3F |
| Button Pin | config_wifi.h | 2 | 2 or 3 (INT0 or INT1) |
| Display Duration | config_wifi.h | 2000ms | 1000-5000ms |
| Enable LCD | config_wifi.h | true | true/false |

---

## Related Files

- **arduino_wifi_mqtt_lcd.ino** - Main Arduino sketch with LCD support
- **config_wifi_template.h** - Configuration template (includes LCD settings)
- **I2C_SCANNER.ino** - I2C address detection sketch (provided separately)
- **LCD_TESTING_GUIDE.md** - Detailed testing procedures (if needed)

---

**Last Updated**: 2025-11-16
**Version**: 1.0
**Status**: Complete ✅

