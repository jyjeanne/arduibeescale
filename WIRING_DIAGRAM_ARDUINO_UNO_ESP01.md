# Complete Wiring Diagram - Arduino UNO + ESP-01 WiFi Module

**Project**: ArduiBeeScale
**Hardware**: Arduino UNO + ESP-01 (ESP8266)
**Date**: 2025-11-16
**Version**: 1.0

---

## 🎯 Overview

This document provides **complete, detailed wiring diagrams** for integrating an ESP-01 WiFi module with Arduino UNO, including all sensors and components for the ArduiBeeScale beehive monitoring system.

---

## ⚠️ CRITICAL: Level Shifter Required!

**ESP-01 operates at 3.3V logic. Arduino operates at 5V logic.**

❌ **NEVER connect Arduino TX/RX directly to ESP-01**
✅ **ALWAYS use a Level Shifter (TXB0104) for TX/RX lines**

**Consequence of ignoring this**: ESP-01 will be permanently damaged by 5V signals.

---

## 📋 Complete Parts List

### Essential Components
| Component | Quantity | Purpose | Cost |
|---|---|---|---|
| Arduino UNO | 1 | Main microcontroller | €8-12 |
| ESP-01 WiFi Module | 1 | WiFi connectivity | €3-5 |
| Level Shifter (TXB0104) | 1 | 5V ↔ 3.3V conversion | €1-3 |
| 3.3V Voltage Regulator | 1 | ESP-01 power supply | €1-2 |
| HX711 Load Cell Amplifier | 1 | Weight measurement | €2-3 |
| Load Cell (50kg) | 1 | Weight sensor | €8-12 |
| DHT22 Sensor | 1 | Temp/Humidity | €4-6 |
| 4x AA Battery Holder | 1 | Power supply | €1-2 |
| Breadboard (400+ holes) | 1 | Prototyping | €2-3 |
| Jumper Wires | 50+ | Connections | €2-3 |

### Optional Components
| Component | Purpose | Cost |
|---|---|---|
| LCD1602 I2C Module | Display data locally | €3-5 |
| Push Button | LCD control | €0.50 |
| 10kΩ Resistors | Pull-up/debounce | €0.50 |
| Capacitors (10µF, 0.1µF) | Power smoothing | €0.50 |
| Micro USB Cable | Arduino power | €1-2 |

---

## 🔌 Master Wiring Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                          SYSTEM OVERVIEW                         │
└─────────────────────────────────────────────────────────────────┘

                          ┌──────────────┐
                          │  Raspberry Pi│
                          │  Mosquitto   │
                          └──────┬───────┘
                                 │ WiFi
                     ┌───────────┘
                     │
          ┌──────────▼──────────┐
          │   ESP-01 Module     │
          │   (3.3V logic)      │
          └──────────┬──────────┘
                     │ Level Shifter
          ┌──────────▼──────────┐
          │    Arduino UNO      │
          │    (5V logic)       │
          └──────────┬──────────┘
                     │
        ┌────┬──────┬┴┬──────┬────┐
        │    │      │ │      │    │
    ┌───▼──┐│    ┌──▼──┐┌──▼───┐│
    │Battery││    │HX711││DHT22 ││
    │4x AA ││    │Load ││Temp/ ││
    │      ││    │Cell ││Humid ││
    └──────┘│    └─────┘└──────┘│
            │                   │
         Optional:        Optional:
         LCD1602          Button
         I2C Module       Pin 2
```

---

## 1️⃣ Power Distribution Diagram

### Battery Power Path (Primary)

```
┌──────────────────────────────────────────┐
│        4x AA Battery Holder              │
│        (4.5-6.0V nominal)                │
│                                          │
│   +  ─────┬─────────────────┬────────┐  │
│           │                 │        │  │
│           │                 │        │  │
└───────────┼─────────────────┼────────┘  │
            │                 │        │
        ┌───▼───┐         ┌───▼────┐  │
        │ 3.3V  │         │ Arduino│  │
        │ Reg   │         │  5V    │  │
        └───┬───┘         │  Power │  │
            │             └────────┘  │
            │                         │
         ┌──▼───┐             ┌───────▼──┐
         │ESP-01│             │All Other │
         │3.3V  │             │Modules   │
         │Power │             │5V Power  │
         └──────┘             └──────────┘

Voltage Levels:
Battery:  4.5-6.0V (depends on load)
3.3V Reg: 3.2-3.4V (for ESP-01 only!)
Arduino:  5V (from battery or USB)
```

**Wiring Details**:
```
Battery + ─────┬──────────────────────────────┐
               │                              │
         ┌─────▼─────┐  3.3V Regulator  ┌────▼────┐
         │ ESP-01    │<─────from───────>│Arduino  │
         │ 3.3V      │   battery +      │Vin      │
         │           │                  │         │
         └─────┬─────┘                  └────┬────┘
               │                             │
         ┌─────▼─────┐                 ┌────▼────┐
         │   GND     │                 │Arduino  │
         │           │                 │GND      │
         └───────────┘                 └─────────┘
```

---

## 2️⃣ ESP-01 WiFi Module Wiring (CRITICAL!)

### ESP-01 Pinout

```
┌─────────────────────┐
│   ESP-01 Front View │
├─────────────────────┤
│ GND  TX  RX  3.3V   │ ← Top edge (antenna side)
│ GPIO0 GPIO2 CH_PD   │ ← Bottom edge
└─────────────────────┘

Pin Names:
GND    = Ground (pin 1)
TX     = Transmit (pin 2) - connect to Level Shifter B2
RX     = Receive (pin 3) - connect to Level Shifter B1
3.3V   = Power (pin 4) - from 3.3V regulator
GPIO0  = GPIO0 (pin 5) - must be grounded for normal operation
GPIO2  = GPIO2 (pin 6) - pull-up to 3.3V
CH_PD  = Chip Enable (pin 7) - pull-up to 3.3V
GND    = Ground (pin 8)
```

### Complete ESP-01 Connection Diagram

```
                    3.3V Regulator (from battery)
                           │
                           │
         ┌─────────────────┼─────────────────┐
         │ 10kΩ     10kΩ   │   10kΩ          │
         │  ↓        ↓     │    ↓            │
        GND       GPIO2    │  CH_PD      3.3V
         │          │      │    │            │
    ┌────┴──────┬───┴──┬───┴┬───┴────┬──────┘
    │ 1         │      │    │        │ 4
    │ GND ──────┘      │    │        └──── 3.3V
    │                  │    │
    │ 8         6      │    │  7
    │ GND ─── GPIO2    │    └──── CH_PD
    │                  │              (pull-up 10kΩ
    │ 5                │               to 3.3V)
    │ GPIO0 ───┬───────┘
    │          │
    │         GND (tie to ground during normal operation)
    │
    │ 3         2
    │ RX ─────────────┬─ To Level Shifter B1 (from Arduino RX)
    │                 │
    │ 2         3
    │ TX ─────────────┬─ To Level Shifter B2 (from Arduino TX)
    │
    └── Antenna connection (standard antenna)
```

### ESP-01 Configuration Details

| Pin | Connection | Purpose | Notes |
|---|---|---|---|
| **1** | GND | Ground | Common ground with Arduino |
| **2** | TX (GPIO1) | Serial TX | To Level Shifter B2 |
| **3** | RX (GPIO3) | Serial RX | From Level Shifter B1 |
| **4** | 3.3V | Power | From 3.3V regulator (max 300mA) |
| **5** | GPIO0 | Boot Mode | Ground during normal operation |
| **6** | GPIO2 | GPIO | Pull-up 10kΩ to 3.3V |
| **7** | CH_PD | Chip Enable | Pull-up 10kΩ to 3.3V (MUST be high!) |
| **8** | GND | Ground | Common ground with Arduino |

**CRITICAL ESP-01 Setup**:
```
For NORMAL OPERATION (not flashing firmware):
├─ GPIO0 must be tied to GND
├─ CH_PD must be pulled HIGH (10kΩ to 3.3V)
├─ GPIO2 must be pulled HIGH (10kΩ to 3.3V)
├─ Both GND pins (1, 8) connected to Arduino GND
├─ 3.3V from proper regulator (NOT Arduino 3.3V pin!)
└─ TX/RX through Level Shifter (NEVER direct 5V!)
```

---

## 3️⃣ Level Shifter (TXB0104) Wiring

### Why Level Shifter is Essential

```
Arduino (5V)    ≠    ESP-01 (3.3V)
├─ Logic HIGH = 5V      ├─ Logic HIGH = 3.3V
├─ Logic LOW = 0V       ├─ Logic LOW = 0V
└─ Max input = 5.5V ✓   └─ Max input = 3.6V ✗

Direct connection: 5V signal → Destroys ESP-01 ❌
Through level shifter: 5V → 3.3V ✓ (safe) ✅
```

### Level Shifter Pinout (TXB0104)

```
         TXB0104 Module
    ┌─────────────────────┐
    │ Side A (5V side)    │ Side B (3.3V side)
    │                     │
    │ VCC_A   VCC_B       │
    │ GND_A   GND_B       │
    │ A1  ←→  B1          │
    │ A2  ←→  B2          │
    │ A3  ←→  B3          │
    │ A4  ←→  B4          │
    │ (extra pins)        │
    └─────────────────────┘

Standard I/O Mapping:
A1 ←→ B1 = RX (from Arduino RX → to ESP-01 RX)
A2 ←→ B2 = TX (from Arduino TX → to ESP-01 TX)
A3 ←→ B3 = (unused or other I/O)
A4 ←→ B4 = (unused or other I/O)
```

### Complete Level Shifter Connection

```
Level Shifter (TXB0104) Connections:

                    ┌──────────────────┐
                    │   Arduino UNO    │
                    │                  │
                    │ TX (Pin 1)   ────┼─┐
                    │ RX (Pin 0)   ────┼─┼─┐
                    │ 5V           ────┼─┼─┼─┐
                    │ GND          ────┼─┼─┼─┼─┐
                    └──────────────────┘ │ │ │ │
                                        │ │ │ │
                      ┌─────────────────┘ │ │ │
                      │                   │ │ │
        ┌─────────────┴────────────────┬──┘ │ │
        │                              │    │ │
        │   Level Shifter (TXB0104)    │    │ │
        │                              │    │ │
        │  VCC_A (5V) ◄───────────────┼────┼─┤
        │  GND_A      ◄───────────────┼────┼─┤
        │                              │    │ │
        │  A1 (RX from Arduino) ◄─────┤    │ │
        │  B1 (to ESP-01 RX)   ───────┤─┬──┼─┤
        │                              │ │  │ │
        │  A2 (TX from Arduino) ◄─────┤─┼──┤ │
        │  B2 (to ESP-01 TX)   ───────┤─┼──┼─┤
        │                              │ │  │ │
        │  VCC_B (3.3V) ◄─────────────┼─┼──┤ │
        │  GND_B        ◄─────────────┼─┼──┼─┤
        └──────────────────────────────┘ │ │ │
                                        │ │ │
                      ┌─────────────────┘ │ │
                      │                   │ │
                      │  ┌────────────────┘ │
                      │  │                  │
                      │  │  ┌───────────────┘
                      │  │  │
                    ┌─▼──▼──▼───┐
                    │  ESP-01    │
                    │            │
                    │ RX ◄────┐  │
                    │ TX ◄────┤  │
                    │ 3.3V◄───┤  │
                    │ GND ◄───┤  │
                    └────────┬┘
                             │
                          (antenna)
```

### Level Shifter Wiring Table

| Level Shifter Pin | Arduino Pin | ESP-01 Pin | Wire Color | Purpose |
|---|---|---|---|---|
| **VCC_A** | Arduino 5V | - | Red | 5V power for level shifter |
| **GND_A** | Arduino GND | - | Black | Ground (5V side) |
| **A1** | Arduino RX (Pin 0) | - | Blue | RX data (5V → 3.3V) |
| **B1** | - | ESP-01 RX | Blue | RX data (3.3V) |
| **A2** | Arduino TX (Pin 1) | - | Green | TX data (5V → 3.3V) |
| **B2** | - | ESP-01 TX | Green | TX data (3.3V) |
| **VCC_B** | - | 3.3V Regulator | Red | 3.3V power for level shifter |
| **GND_B** | Arduino GND | - | Black | Ground (3.3V side) |

---

## 4️⃣ Serial Communication Connections

```
Arduino UNO ←→ Level Shifter ←→ ESP-01

┌─────────────────────┐
│   Arduino UNO       │
│                     │
│   Pin 0 (RX) ───┐  │
│                 │  │
│   Pin 1 (TX) ───┼──┤   ┌──────────────────┐
│                 └──┼──┤ Level Shifter    │
│   5V  ──────────┐  │   │ (TXB0104)        │
│   GND ──────────┼──┼──┤                  │
└─────────────────┘  │   │ A1 ←→ B1 (RX)  │
                     └──┤ A2 ←→ B2 (TX)   │
                        │ VCC_A (5V)     │
                        │ VCC_B (3.3V)   │
                        │ GND_A & GND_B  │
                        └────────┬────────┘
                                 │
                    ┌────────────┼────────────┐
                    │            │            │
                    │   ┌────────▼──┐    ┌───▼─────┐
                    │   │  RX (3)   │    │  TX (2) │
                    │   │           │    │         │
                    │   └────────────┘    └────┬────┘
                    │                          │
                    │    ESP-01 Module         │
                    │                          │
                    └──────────┬───────────────┘
                               │
                            Antenna
```

---

## 5️⃣ HX711 Load Cell Amplifier Wiring

### HX711 Module Pinout

```
        HX711 Module (Top View)
    ┌──────────────────────┐
    │ GND  DOUT DT   VCC   │
    │ CLK  E+ A- B+  GND   │
    └──────────────────────┘

Pin Names:
GND   = Ground (both pins)
DOUT  = Data Output (to Arduino)
DT    = Data (same as DOUT)
CLK   = Clock Signal (from Arduino)
VCC   = Power 5V
E+    = Load Cell E+ (red wire)
E-    = Load Cell E- (black wire)
A-    = Load Cell A- (white wire)
A+    = Load Cell A+ (green wire)
B+    = Load Cell B+ (green wire)
B-    = Load Cell B- (white wire)
```

### HX711 to Arduino Connection

```
┌─────────────────────┐
│     HX711 Module    │
│                     │
│ VCC  ───────────┬── 5V
│ GND  ───────┬───┼── GND
│             │   │
│ DOUT/DT ────┼───┼── Pin 5 (Data)
│ CLK    ────┬┼───┼── Pin 6 (Clock)
│            ││   │
└────────────┼┼───┘
             ││
             └┴─────── Arduino UNO
                       Power

Arduino Pins for HX711:
├─ Pin 5 (DOUT) = Data output from HX711
├─ Pin 6 (CLK)  = Clock signal to HX711
├─ 5V (VCC)     = Power supply
└─ GND          = Ground
```

### Load Cell to HX711 Connection

```
Load Cell                HX711 Connector
(Typically 4-wire)
                 ┌──────────────┐
Red Wire ────────┤ E+ (or +Excitation)
                 │              │
Black Wire ───────┤ E- (or -Excitation)
                 │              │
White Wire ───────┤ A- (or Signal-)
                 │              │
Green Wire ───────┤ A+ (or Signal+)
                 │              │
                 └──────────────┘
                 (HX711 Bottom)

Note: Exact pin names vary by HX711 module
Common variants:
├─ E+/E- (Excitation voltage)
├─ A+/A- (Load cell A side)
└─ B+/B- (Load cell B side - usually unused)
```

---

## 6️⃣ DHT22 Temperature/Humidity Sensor Wiring

### DHT22 Module Pinout

```
DHT22 Sensor (Front View)
┌──────────┐
│ 1 2 3 4  │
└──────────┘

Pin 1: VCC (Power 5V)
Pin 2: SDA (Data signal)
Pin 3: NC (Not Connected)
Pin 4: GND (Ground)

Common module has:
+ (VCC), - (GND), S (Signal)
```

### DHT22 to Arduino Connection

```
┌──────────────────────┐
│   DHT22 Sensor       │
│                      │
│ Pin 1 (VCC) ───────┬─ 5V
│ Pin 2 (SDA) ───────┼─ Pin 10 (Data)
│ Pin 3 (NC) ────────┼─ (not connected)
│ Pin 4 (GND) ───────┼─ GND
│                    │
│ Optional:          │
│ 10kΩ Pull-up (Pin 2 to VCC)
│                      │
└──────────────────────┘
        │
        └─── Arduino UNO Pin 10

Arduino Pins for DHT22:
├─ Pin 10 (Data) = Signal line
├─ 5V (VCC)      = Power supply
└─ GND           = Ground
```

---

## 7️⃣ Battery Voltage Monitoring Wiring

### Voltage Divider Calculation

```
For 4x AA batteries (4.5-6.0V):
Measure voltage at analog input A0

Formula: V_A0 = V_battery × (R2 / (R1 + R2))

Using 5kΩ (R1) and 1kΩ (R2):
V_A0 = V_battery × (1 / 6) = V_battery × 0.167

Example:
├─ Battery: 6.0V → A0 sees: 1.0V
├─ Battery: 4.5V → A0 sees: 0.75V
└─ Battery: 3.0V → A0 sees: 0.5V

Arduino ADC Range: 0-5V = 0-1023 counts
Conversion: V_measured = (ADC_count / 1023) × 5
```

### Complete Voltage Divider Wiring

```
Battery +
  │
  ├─ 5kΩ Resistor (R1)
  │
  ├───┬─── A0 (Arduino Analog Input)
  │   │
  │ 10µF Capacitor (smoothing)
  │   │
  └─ 1kΩ Resistor (R2)
  │
Battery - / GND

Visual:
Battery+
   │
   ├──[5kΩ]──┬─────── A0 (Pin A0)
   │         │
   │         ├─[C]─── GND
   │         │ 10µF
   │         │
   │       ──┴──
   │      [1kΩ]
   │         │
   └─────────┴─────── GND (Battery-)

Component Values:
├─ R1: 5kΩ (tolerance ±5% or better)
├─ R2: 1kΩ (tolerance ±5% or better)
└─ C:  10µF (to smooth voltage spikes)
```

### Arduino Analog Input Configuration

```cpp
const int BATTERY_PIN = A0;

void setup() {
    pinMode(BATTERY_PIN, INPUT);
}

void loop() {
    int rawValue = analogRead(BATTERY_PIN);

    // Convert to voltage
    float v_a0 = (rawValue / 1023.0) * 5.0;

    // Get battery voltage (divider ratio = 6)
    float batteryVoltage = v_a0 * 6.0;
}
```

---

## 8️⃣ LCD1602 I2C Display Wiring (Optional)

### LCD1602 I2C Module Pinout

```
LCD1602 I2C Backpack
┌──────────────┐
│ GND SDA SCL VCC│
└──────────────┘

Pins:
GND = Ground
SDA = I2C Data (Serial Data)
SCL = I2C Clock
VCC = Power 5V
```

### LCD1602 I2C to Arduino Connection

```
┌─────────────────────┐
│  LCD1602 I2C        │
│                     │
│ VCC ───────────────┬─ 5V (Arduino)
│ GND ───────────┬───┼─ GND
│ SDA ───────────┼───┼─ A4 (Arduino)
│ SCL ───────────┼───┤ A5 (Arduino)
│                │   │
└────────────────┼───┘
                 │
        Arduino UNO

Arduino Pins for LCD:
├─ A4 (SDA) = I2C Data
├─ A5 (SCL) = I2C Clock
├─ 5V (VCC) = Power
└─ GND      = Ground
```

---

## 9️⃣ Push Button Wiring (Optional)

### Push Button to Arduino Connection

```
Push Button Module or Tactile Switch

    ┌─────┐
    │ GND │ (one pin)
    │     │
    │ +5V │ (optional, if using pull-down)
    └─────┘

Simple Connection:

Pin 2 (Arduino) ───┬─ Button Pin 1
                  │
                  │  [Button]
                  │
            Arduino GND ─── Button Pin 2

Arduino Internal Pull-up:
pinMode(2, INPUT_PULLUP);  // No external resistor needed
```

---

## 🔟 Complete System Wiring Summary Table

| Component | Arduino Pin | Connection Type | Wire Color |
|---|---|---|---|
| **ESP-01 RX** | Via Level Shifter B1 | Serial RX | Blue |
| **ESP-01 TX** | Via Level Shifter B2 | Serial TX | Green |
| **HX711 DOUT** | Pin 5 | Digital Input | Yellow |
| **HX711 CLK** | Pin 6 | Digital Output | Orange |
| **DHT22 Data** | Pin 10 | Digital Input | White |
| **Battery Voltage** | A0 | Analog Input | Purple |
| **Button** | Pin 2 | Digital Input (INT0) | Red |
| **LCD SDA** | A4 | I2C Data | Green |
| **LCD SCL** | A5 | I2C Clock | Blue |
| **5V Power** | 5V | Power Distribution | Red |
| **Ground** | GND | Ground Distribution | Black |

---

## 📐 Full System Schematic (ASCII Art)

```
┌──────────────────────────────────────────────────────────────────┐
│                    ArduiBeeScale System                           │
│                    Complete Wiring Diagram                        │
└──────────────────────────────────────────────────────────────────┘

                    ┌─────────────────────────┐
                    │   4x AA Battery         │
                    │   (4.5-6.0V)            │
                    └────┬───────────┬────────┘
                         │           │
                    ┌────▼───┐  ┌───▼────┐
                    │3.3V    │  │Arduino │
                    │Regulator  │5V Power│
                    └────┬───┘  └───┬────┘
                         │          │
                    ┌────▼──────────▼──────┐
                    │  Level Shifter       │
                    │  TXB0104             │
                    │ (5V ←→ 3.3V)         │
                    └────┬──────────┬──────┘
                         │          │
                    ┌────▼──────────▼──────┐
                    │   ESP-01 WiFi        │
                    │   Module             │
                    └────────┬─────────────┘
                             │
                         ┌───┴────────────────┐
                         │   Raspberry Pi     │
                         │   Mosquitto MQTT   │
                         └────────────────────┘

┌───────────────────────────────────────────────────────────────┐
│                    Arduino UNO Connections                     │
├───────────────────────────────────────────────────────────────┤
│                                                               │
│  Pin 0  (RX) ──→ Level Shifter A1 ──→ ESP-01 RX             │
│  Pin 1  (TX) ──→ Level Shifter A2 ──→ ESP-01 TX             │
│  Pin 2  ──→ Button (INT0)                                    │
│  Pin 5  ──→ HX711 DOUT (Load Cell Data)                      │
│  Pin 6  ──→ HX711 CLK  (Load Cell Clock)                     │
│  Pin 10 ──→ DHT22 SDA  (Temp/Humidity Data)                  │
│                                                               │
│  A0 (Analog) ──→ Voltage Divider ──→ Battery Monitoring      │
│  A4 (SDA) ──→ LCD1602 I2C Data                               │
│  A5 (SCL) ──→ LCD1602 I2C Clock                              │
│                                                               │
│  5V ──→ Power distribution for all modules                   │
│  GND ──→ Ground distribution (common ground)                 │
│                                                               │
└───────────────────────────────────────────────────────────────┘

                    ┌──────────────────┐
                    │   Sensors        │
                    │                  │
                    │ ├─ HX711 + Load  │
                    │ │   Cell (Pin 5) │
                    │ ├─ DHT22         │
                    │ │   (Pin 10)     │
                    │ ├─ Battery Volt  │
                    │ │   (A0)         │
                    │ └─ LCD1602 I2C   │
                    │     (A4, A5)     │
                    │                  │
                    └──────────────────┘
```

---

## ✅ Pre-Assembly Checklist

Before starting assembly:

```
Components Verification:
□ Arduino UNO (genuine or clone)
□ ESP-01 WiFi Module with antenna
□ Level Shifter TXB0104 module
□ 3.3V Voltage Regulator (AMS1117-3.3)
□ HX711 Load Cell Amplifier
□ Load Cell (50kg, 4-wire)
□ DHT22 Temperature/Humidity Sensor
□ 4x AA Battery Holder with leads
□ 400+ hole breadboard
□ Jumper wires (50+ pieces)

Resistor/Capacitor Values:
□ 5kΩ resistor (battery divider R1)
□ 1kΩ resistor (battery divider R2)
□ 10kΩ resistor × 3 (ESP-01 pull-ups)
□ 10µF capacitor (battery smoothing)
□ 0.1µF capacitor (noise filtering - optional)

Optional Components:
□ LCD1602 I2C module
□ Push button (tactile switch)
□ 10kΩ resistor (debounce)

Tools Needed:
□ Soldering iron (25-30W)
□ Solder (lead-free)
□ Multimeter
□ Wire strippers
□ USB cable (for Arduino)
□ Computer with Arduino IDE
```

---

## 🔧 Assembly Steps

### Step 1: Prepare Breadboard Layout (5 min)

1. Place Arduino UNO on breadboard (or keep off for cleaner layout)
2. Place Level Shifter on breadboard
3. Place HX711 module on breadboard
4. Leave space for power distribution rows

### Step 2: Build Power Distribution (10 min)

1. Create 5V power rail
2. Create GND (ground) rail
3. Create 3.3V rail (from regulator output)
4. Connect battery + through 5kΩ resistor to input of 3.3V regulator
5. Connect battery - to regulator GND
6. Verify voltages with multimeter:
   - 5V rail: 4.8-5.2V
   - 3.3V rail: 3.2-3.4V
   - GND: 0V reference

### Step 3: Connect ESP-01 (10 min)

1. Install level shifter
2. Connect Arduino TX (Pin 1) → Level Shifter A2
3. Connect Arduino RX (Pin 0) → Level Shifter A1
4. Connect Level Shifter B2 → ESP-01 TX
5. Connect Level Shifter B1 → ESP-01 RX
6. Connect ESP-01 GND → Arduino GND
7. Connect ESP-01 3.3V → 3.3V rail (NOT Arduino 3.3V!)
8. Connect ESP-01 GPIO0 → GND (normal operation)
9. Connect ESP-01 GPIO2 → 3.3V via 10kΩ pull-up
10. Connect ESP-01 CH_PD → 3.3V via 10kΩ pull-up
11. Add ESP-01 antenna

### Step 4: Connect HX711 Load Cell (10 min)

1. Place HX711 on breadboard
2. Connect HX711 VCC → 5V rail
3. Connect HX711 GND → GND rail
4. Connect HX711 DOUT → Arduino Pin 5
5. Connect HX711 CLK → Arduino Pin 6
6. Connect load cell wires to HX711:
   - Red (E+) → HX711 E+
   - Black (E-) → HX711 E-
   - White (A-) → HX711 A-
   - Green (A+) → HX711 A+

### Step 5: Connect DHT22 Sensor (5 min)

1. Connect DHT22 VCC → 5V rail
2. Connect DHT22 GND → GND rail
3. Connect DHT22 SDA → Arduino Pin 10
4. (Optional) Connect 10kΩ pull-up from Pin 10 to 5V

### Step 6: Connect Battery Voltage Divider (5 min)

1. Connect 5kΩ resistor from Battery+ to node
2. Connect 1kΩ resistor from node to GND
3. Connect node (middle point) → Arduino A0
4. (Optional) Connect 10µF capacitor from A0 to GND (for smoothing)

### Step 7: Optional - Connect LCD1602 (5 min)

1. Connect LCD VCC → 5V rail
2. Connect LCD GND → GND rail
3. Connect LCD SDA → Arduino A4
4. Connect LCD SCL → Arduino A5

### Step 8: Optional - Connect Push Button (3 min)

1. Connect button pin 1 → Arduino Pin 2
2. Connect button pin 2 → GND rail

### Step 9: Verify All Connections (10 min)

```
Verification Checklist:
□ No loose jumper wires
□ All solder joints shiny (if soldered)
□ 5V rail has ~5V (multimeter check)
□ 3.3V rail has ~3.3V (multimeter check)
□ GND rail has 0V reference
□ ESP-01 powered (LED should light)
□ HX711 powered (LED should light)
□ DHT22 powered (no LED, but powered)
□ All data lines connected
□ No shorts between power rails
```

### Step 10: Connect Arduino to Computer (5 min)

1. Connect USB cable to Arduino
2. Select correct board: Tools → Board → Arduino UNO
3. Select correct port: Tools → Port → COM3 (or your port)
4. Upload test sketch to verify all connections

---

## 🧪 Testing After Assembly

### Test 1: Power Supply Test

```cpp
void setup() {
    Serial.begin(9600);
    Serial.println("Testing power supplies...");

    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
}

void loop() {
    int raw5v = analogRead(A0);   // Should be ~1023 if measuring Arduino 5V
    float voltage = (raw5v / 1023.0) * 5.0;

    Serial.print("5V Rail: ");
    Serial.print(voltage);
    Serial.println("V");

    delay(1000);
}
```

Expected output: ~4.9-5.1V

### Test 2: Serial Communication Test

Upload example sketch to verify ESP-01 communication is working.

### Test 3: Sensor Test

Test each sensor independently before full integration.

---

## 🚨 Troubleshooting Common Wiring Issues

| Problem | Cause | Solution |
|---|---|---|
| **ESP-01 not responding** | TX/RX reversed or missing level shifter | Check level shifter B1/B2 connections, verify TX/RX not swapped |
| **HX711 no data** | Pin 5 or 6 not connected | Verify breadboard connections, use multimeter to check continuity |
| **DHT22 no reading** | Pin 10 not connected or sensor damaged | Check Pin 10 connection, try different DHT22 if available |
| **Battery voltage wrong** | Voltage divider resistor values wrong | Verify 5kΩ and 1kΩ resistor values with multimeter |
| **ESP-01 gets hot** | 5V applied to ESP-01 (should be 3.3V only) | Check 3.3V regulator output, remove ESP-01 to be safe |
| **Arduino not responding** | USB cable loose or wrong port selected | Try different USB cable, check Tools → Port selection |
| **Level Shifter not working** | Pins A/B reversed or VCC not applied | Verify VCC_A and VCC_B connected to correct voltages |

---

## 📊 Final Wiring Verification Sheet

Print this and check off each item:

```
POWER DISTRIBUTION:
□ Battery + to 5kΩ resistor
□ 5kΩ to 3.3V regulator input
□ 3.3V regulator output to 3.3V rail
□ Battery - to GND rail
□ Arduino 5V connected to 5V rail
□ Arduino GND connected to GND rail
□ Multiple GND connections (all common)

SERIAL COMMUNICATION:
□ Arduino TX (Pin 1) → Level Shifter A2
□ Arduino RX (Pin 0) → Level Shifter A1
□ Level Shifter VCC_A to Arduino 5V
□ Level Shifter VCC_B to 3.3V rail
□ Level Shifter B2 → ESP-01 TX (pin 2)
□ Level Shifter B1 → ESP-01 RX (pin 3)
□ Level Shifter GND to Arduino GND

ESP-01 CONFIGURATION:
□ ESP-01 3.3V from 3.3V regulator (NOT Arduino 3.3V!)
□ ESP-01 GND common with Arduino GND
□ ESP-01 GPIO0 to GND (normal mode)
□ ESP-01 GPIO2 to 3.3V via 10kΩ resistor
□ ESP-01 CH_PD to 3.3V via 10kΩ resistor
□ Antenna connected to ESP-01

SENSORS:
□ HX711 Pin 5 (DOUT) to Arduino Pin 5
□ HX711 Pin 6 (CLK) to Arduino Pin 6
□ HX711 VCC to 5V rail
□ HX711 GND to GND rail
□ Load cell red → HX711 E+
□ Load cell black → HX711 E-
□ Load cell white → HX711 A-
□ Load cell green → HX711 A+
□ DHT22 VCC to 5V rail
□ DHT22 GND to GND rail
□ DHT22 data to Arduino Pin 10

OPTIONAL:
□ Battery voltage divider: 5kΩ-[node]-1kΩ-GND
□ Node connected to Arduino A0
□ 10µF capacitor from A0 to GND
□ LCD SDA to Arduino A4
□ LCD SCL to Arduino A5
□ Button Pin 1 to Arduino Pin 2
□ Button Pin 2 to GND

FINAL CHECK:
□ No loose jumper wires
□ No wires crossing exposed connections
□ All solder joints solid (if applicable)
□ 5V rail measures 4.8-5.2V
□ 3.3V rail measures 3.2-3.4V
□ ESP-01 LED lights up
□ Arduino can be programmed via USB
□ Serial monitor shows output (9600 baud)
```

---

## 📝 Notes for Future Reference

- **ESP-01 Replacement**: If damaged, only the ESP-01 needs replacement ($3-5)
- **Level Shifter Critical**: This is the most important protection component
- **Breadboard vs. Soldering**: Breadboard allows testing; solder for permanent installation
- **Multiple Grounds**: Ensure all GND connections are common (same node)
- **Power Supply**: Can be battery or USB during development; battery required for field deployment

---

**Version**: 1.0
**Date**: 2025-11-16
**Status**: Complete ✅

This comprehensive wiring diagram is your reference for assembling the ArduiBeeScale system. Keep it handy during assembly!

