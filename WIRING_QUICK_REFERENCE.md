# ArduiBeeScale - Wiring Quick Reference (Printable)

**Print this page and keep it at your workbench!**

---

## ⚠️ CRITICAL RULE

```
╔═══════════════════════════════════════════════════╗
║ ESP-01 = 3.3V logic                               ║
║ Arduino = 5V logic                                ║
║ NEVER connect TX/RX directly!                     ║
║ ALWAYS use Level Shifter (TXB0104)                ║
║ Consequence: ESP-01 destroyed by 5V ❌           ║
╚═══════════════════════════════════════════════════╝
```

---

## 1. Power Distribution

```
Battery (4.5-6V)
    │
    ├─[5kΩ]─┬─ 3.3V Reg In
    │        │
    │     ┌──▼─────┐
    │     │ 3.3V   │
    │     │ Reg    │ Output ───→ 3.3V Rail (ESP-01 only)
    │     └────────┘
    │
    └───────────┬─────────── 5V Rail (Arduino & Modules)
            GND ────────────── GND Rail (Common)
```

**Voltage Check** (use multimeter):
- 5V Rail: 4.8-5.2V ✓
- 3.3V Rail: 3.2-3.4V ✓
- GND: 0V reference ✓

---

## 2. Serial Communication (CRITICAL!)

```
Arduino UNO          Level Shifter          ESP-01
Pin 1 (TX) ───────→ A2 ↔ B2 ───────────→ RX (Pin 3)
Pin 0 (RX) ←─────── A1 ↔ B1 ←───────────  TX (Pin 2)
5V ─────────────→ VCC_A
GND ────────────→ GND_A              GND ←─ GND_B
                VCC_B ←─ 3.3V
```

**DO NOT SKIP LEVEL SHIFTER!**

---

## 3. ESP-01 Setup (MUST DO)

```
GPIO0 ──────────→ GND        (Normal operation)
GPIO2 ──[10kΩ]→ 3.3V        (Pull-up resistor)
CH_PD ──[10kΩ]→ 3.3V        (Pull-up resistor)
3.3V ─────────→ From regulator (NOT Arduino 3.3V pin!)
GND ──────────→ Common with Arduino GND
Antenna ──────→ Attached
```

---

## 4. Arduino Pins Reference

| Pin | Purpose | Component |
|-----|---------|-----------|
| 0 | RX | ESP-01 via Level Shifter |
| 1 | TX | ESP-01 via Level Shifter |
| 2 | Digital In | Button (optional) |
| 5 | Digital In | HX711 DOUT |
| 6 | Digital Out | HX711 CLK |
| 10 | Digital In | DHT22 Data |
| A0 | Analog In | Battery Voltage |
| A4 | I2C SDA | LCD (optional) |
| A5 | I2C SCL | LCD (optional) |
| 5V | Power | Modules |
| GND | Ground | All |

---

## 5. HX711 Load Cell (Pin 5 & 6)

```
HX711
├─ VCC ─→ 5V
├─ GND ─→ GND
├─ DOUT ─→ Pin 5
├─ CLK ─→ Pin 6
│
└─ Load Cell Connections:
  ├─ Red (E+) ─→ HX711 E+
  ├─ Black (E-) ─→ HX711 E-
  ├─ White (A-) ─→ HX711 A-
  └─ Green (A+) ─→ HX711 A+
```

---

## 6. DHT22 Sensor (Pin 10)

```
DHT22
├─ VCC ─→ 5V
├─ GND ─→ GND
└─ Data ─→ Pin 10
         ├─ [10kΩ pull-up to 5V] (optional)
         └─ Arduino
```

---

## 7. Battery Voltage (A0)

```
Battery +
  │
  ├─[5kΩ]─┬─→ A0 (Arduino)
  │       │
  │     [10µF] (optional smoothing)
  │       │
  └─[1kΩ]─┼─→ GND
          │
         GND
```

---

## 8. LCD I2C Display (Optional - Pins A4, A5)

```
LCD1602 I2C
├─ VCC ─→ 5V
├─ GND ─→ GND
├─ SDA ─→ A4
└─ SCL ─→ A5
```

---

## 9. Push Button (Optional - Pin 2)

```
Button
├─ Pin 1 ─→ Arduino Pin 2
└─ Pin 2 ─→ GND
```

---

## Assembly Checklist

**Before Soldering**:
- [ ] All components identified and tested
- [ ] Breadboard layout planned
- [ ] Level shifter module available
- [ ] 3.3V regulator available
- [ ] Multimeter ready for testing

**Power Distribution** (First priority!):
- [ ] 5V rail created
- [ ] GND rail created
- [ ] 3.3V rail created
- [ ] Battery + to 5kΩ resistor
- [ ] 3.3V regulator connected
- [ ] Verify voltages: 5V, 3.3V, GND

**Serial Connection** (CRITICAL!):
- [ ] Level shifter installed
- [ ] Arduino TX → Level Shifter A2
- [ ] Arduino RX → Level Shifter A1
- [ ] Level Shifter B2 → ESP-01 TX
- [ ] Level Shifter B1 → ESP-01 RX

**ESP-01 Setup** (MUST DO!):
- [ ] GPIO0 → GND
- [ ] GPIO2 → 3.3V via 10kΩ
- [ ] CH_PD → 3.3V via 10kΩ
- [ ] 3.3V from regulator
- [ ] Antenna attached

**Sensors**:
- [ ] HX711 (Pins 5, 6)
- [ ] DHT22 (Pin 10)
- [ ] Battery divider (A0)
- [ ] LCD I2C (A4, A5) - optional
- [ ] Button (Pin 2) - optional

**Verification**:
- [ ] No loose wires
- [ ] No shorts
- [ ] Continuity tested
- [ ] All connections solid
- [ ] Ready to upload code

---

## Testing Steps (In Order)

1. **Power Test**
   - Supply 5V and 3.3V
   - Verify with multimeter
   - ESP-01 LED lights up

2. **Serial Test**
   - Upload serial monitor test
   - Check TX/RX communication
   - Look for messages at 9600 baud

3. **Sensor Test**
   - HX711: Verify data on Pin 5
   - DHT22: Verify data on Pin 10
   - Battery: Check A0 voltage reading

4. **WiFi Test**
   - Upload main code
   - Watch Serial Monitor
   - Should see "WiFi connecting..."
   - Should see "WiFi connected!"

5. **MQTT Test**
   - Check Raspberry Pi
   - Run: `mosquitto_sub -h localhost -t "beehive/#" -v`
   - Should see JSON data arriving

---

## Quick Troubleshooting

| Problem | Check This |
|---------|-----------|
| ESP-01 no response | Level Shifter connected? TX/RX swapped? |
| 5V/3.3V wrong | Regulator pins connected right? |
| HX711 no data | Pin 5/6 connected? VCC/GND OK? |
| DHT22 no data | Pin 10 connected? Sensor powered? |
| Battery voltage 0V | Voltage divider resistors right? A0 connected? |
| Code won't upload | Arduino board selected? COM port right? |
| Serial Monitor garbage | Check baud rate (9600)? |

---

## Solder Joint Quality

```
Good Joint              Bad Joint
    ╱╲                    │
   ╱  ╲                   │
  ╱    ╲                 ╭╮
 ╱______╲               ╭╮│
 Shiny    ✓             Dull ✗

- Use thin solder
- Heat component + wire
- Solder flows smoothly
- Cools to shiny finish
- Good electrical connection
```

---

## Continuity Testing (Multimeter)

```
Set multimeter to: Ω (Ohms) or resistance
Touch both probes to ends of wire/solder joint
Multimeter beeps = Good continuity ✓
No beep = Bad connection ✗
```

---

## Wire Colors (Standard)

| Color | Purpose |
|-------|---------|
| 🔴 Red | 5V Power |
| ⚫ Black | GND (Ground) |
| 🟠 Orange | 5V (alternate) |
| 🟡 Yellow | Data signals |
| 🟢 Green | Data signals / TX |
| 🔵 Blue | Data signals / RX |
| ⚪ White | Data signals |
| 🟣 Purple | Analog signals |

---

## Resistor Color Codes

**5kΩ Resistor**: Red-Black-Red-Gold
**1kΩ Resistor**: Brown-Black-Red-Gold
**10kΩ Resistor**: Brown-Black-Orange-Gold

Use multimeter to verify values!

---

## Critical Connection Points

❌ **NEVER do this**:
```
Arduino TX directly to ESP-01 RX
Arduino RX directly to ESP-01 TX
Arduino 5V to ESP-01 power
Arduino 3.3V to ESP-01 power
```

✅ **ALWAYS do this**:
```
Arduino TX → Level Shifter A2 → B2 → ESP-01 TX
Arduino RX → Level Shifter A1 → B1 → ESP-01 RX
ESP-01 3.3V ← 3.3V Regulator ← Battery
ESP-01 GND = Arduino GND = Battery GND (common)
```

---

## Component Orientation

**3.3V Regulator** (AMS1117):
```
  IN  GND  OUT
   ▼   ▼    ▼
┌──────────────┐
│ AMS1117-3.3  │
└──────────────┘
 Input: Battery+
 GND: to GND rail
 Output: 3.3V rail
```

**Level Shifter** (TXB0104):
```
A-side (5V)          B-side (3.3V)
├─ VCC_A ─ 5V        ├─ VCC_B ─ 3.3V
├─ GND_A ─ GND       ├─ GND_B ─ GND
├─ A1 ↔ B1           │
├─ A2 ↔ B2           │
└─ A3-A4 ↔ B3-B4     │
```

---

## Reference Voltages

| Point | Should Be | Range |
|-------|-----------|-------|
| 5V Rail | ~5V | 4.8-5.2V |
| 3.3V Rail | ~3.3V | 3.2-3.4V |
| Battery Input | 4.5-6.0V | Depends on batteries |
| A0 (Battery div) | 0-1V | Varies with battery |
| Level Shifter VCC_B | 3.3V | 3.2-3.4V |
| ESP-01 Supply | 3.3V | 3.2-3.4V |
| Arduino TX (pin 1) | 0 or 5V | Digital |
| Arduino RX (pin 0) | 0 or 5V | Digital |

---

## Pin Diagram (Top View)

```
Arduino UNO (Top View)
┌─────────────────────────────────┐
│ [USB]                      [PWR]│
│                                  │
│  RX ←1  0→ D0                    │
│  TX ←2  1→ D1              [RST] │
│  D2←3   2→ D2 (INT0)      [AREF]│
│  D3←4   3→ D3 (INT1)       [3.3V]
│ [GND]←5                   [5V]  │
│ [VIN]←6                   [GND] │
│  D5←7   5→ D5             [VIN] │
│  D6←8   6→ D6    [IOREF]        │
│  D7←9   7→ D7               A0  │
│  D8←10  8→ D8               A1  │
│  D9←11  9→ D9               A2  │
│ D10←12  10→ D10 (SS)        A3  │
│ D11←13  11→ D11 (MOSI)      A4  │
│ D12←14  12→ D12 (MISO)      A5  │
│ D13←15  13→ D13 (SCK)      [GND]│
│                                  │
└─────────────────────────────────┘

Power Pins:
├─ IOREF: 5V reference
├─ Reset: Reset button connection
├─ 3.3V: 3.3V output (limited current)
├─ 5V: 5V output from USB/Vin
├─ GND: Ground (multiple pins)
└─ Vin: External power input

Digital Pins:
├─ 0-1: Serial TX/RX
├─ 2-3: Interrupt capable
├─ 5-13: Digital I/O
└─ PWM: 3, 5, 6, 9, 10, 11

Analog Pins:
└─ A0-A5: Analog input
```

---

## Most Common Errors

| Error | Cause | Fix |
|-------|-------|-----|
| ESP-01 dies | 5V applied to ESP-01 | Use 3.3V regulator, not Arduino 3.3V |
| No WiFi | TX/RX reversed | Swap wires or check level shifter B1/B2 |
| Data corrupted | Loose ground connection | Solder GND wire solid |
| Wrong readings | Voltage divider off | Check 5kΩ and 1kΩ resistor values |
| Button not working | Pin 2 not connected | Verify breadboard connection |
| LCD blank | Wrong I2C address | Run I2C_SCANNER.ino |

---

## Safety Tips

✅ **DO**:
- Check voltage with multimeter before connecting
- Use proper gauge wires for power
- Keep ESP-01 protected from accidental contact
- Double-check level shifter connections
- Test each component separately first

❌ **DON'T**:
- Apply 5V directly to ESP-01
- Reverse polarity on power
- Leave loose wires near power rails
- Solder near IC chips without proper heat management
- Assume connections are correct without testing

---

## Final Pre-Assembly Questions

1. Do you have a Level Shifter (TXB0104)? ___
2. Do you have a 3.3V Regulator (AMS1117)? ___
3. Have you identified all resistor values? ___
4. Do you have a multimeter for testing? ___
5. Do you understand TX/RX must go through Level Shifter? ___

**If ANY answer is NO, STOP and get the missing item before proceeding!**

---

## Assembly Time Estimate

- Power distribution: 10 min
- Level shifter + serial: 10 min
- ESP-01 setup: 10 min
- HX711 connection: 10 min
- DHT22 connection: 5 min
- Battery divider: 5 min
- Optional (LCD + button): 10 min
- **Total: 50-60 min** (not including testing)

---

## After Assembly

1. Check all connections visually
2. Use multimeter to verify voltages
3. Test continuity on solder joints
4. Connect USB and upload test sketch
5. Monitor Serial output at 9600 baud
6. Verify WiFi connection
7. Check MQTT message arrival

---

**Keep this reference handy!**
**Laminate for water/dirt protection**
**Update date completed: _________**

