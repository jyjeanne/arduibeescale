# Removing ATtiny85 Power Manager - Alternative Solutions

**Date**: November 2025
**Project**: ArduiBeeScale
**Task**: Replace ATtiny85 with alternative power management solutions

---

## 📋 Current ATtiny85 Functionality

The ATtiny85 currently provides:

| Function | Details |
|----------|---------|
| **Sleep/Wake Cycles** | 8-second watchdog cycles, 900 cycles = 120 minutes |
| **MOSFET Control** | Activates Arduino via N-MOSFET gate |
| **Timeout Protection** | 90-second maximum Arduino runtime |
| **Battery Savings** | Ultra-low power consumption in sleep |
| **Automatic Scheduling** | No external trigger needed |

**Current Power Consumption**: <10µA sleep, ~1mA active

---

## 🎯 Alternative Solutions

### **Option 1: RTC (Real-Time Clock) Module** ⭐ RECOMMENDED

**Pros**:
- ✅ Very accurate timing (±2 minutes per month)
- ✅ Easy to configure intervals (hourly, 30 min, custom)
- ✅ Ultra-low power (<2µA)
- ✅ No additional microcontroller needed
- ✅ Built-in battery backup option
- ✅ Can trigger wake via interrupt pin
- ✅ Cost: ~€2-5

**Cons**:
- ❌ Requires RTC library
- ❌ Needs I2C communication
- ❌ Battery backup optional

**Hardware**: DS3231 (I2C RTC Module)
**Pins**: A4 (SDA), A5 (SCL), D2 (interrupt)

**Cost Comparison**:
```
ATtiny85 + socket + programmer: ~€8-10
RTC DS3231 Module: ~€2-3
Savings: ~€5-7
```

---

### **Option 2: Software-Only Sleep (Simplest)**

**Pros**:
- ✅ No extra hardware needed
- ✅ Arduino handles everything
- ✅ Simplest to implement
- ✅ Cost: €0
- ✅ Easy to modify timing
- ✅ Good for testing

**Cons**:
- ❌ Power consumption slightly higher (~1-5mA in sleep)
- ❌ ~50% more battery drain vs ATtiny
- ❌ Less reliable timing (depends on oscillator)
- ❌ Can't wake from complete power-down

**Power Impact**:
```
ATtiny85 approach: 120-minute cycle
  Active: 40 seconds @ 500mA = 5.5 mAh
  Sleep: 3600 seconds @ 10µA = 0.01 mAh
  Total per cycle: ~5.5 mAh

Software sleep (SLEEP_MODE_PWR_SAVE):
  Active: 40 seconds @ 500mA = 5.5 mAh
  Sleep: 3600 seconds @ 1mA = 1 mAh
  Total per cycle: ~6.5 mAh

Difference: ~18% more power consumption
```

---

### **Option 3: Relay Timer Module**

**Pros**:
- ✅ Very simple, pure analog solution
- ✅ Cost: €3-5
- ✅ No programming needed
- ✅ Completely independent

**Cons**:
- ❌ Fixed timing only (adjustable but mechanical)
- ❌ Mechanical relays have limited lifespan
- ❌ Higher power consumption (relay coil)
- ❌ Less flexible
- ❌ Larger footprint

**Use Case**: Simple fixed-interval applications

---

### **Option 4: 555 Timer Circuit**

**Pros**:
- ✅ Completely analog circuit
- ✅ No microcontroller needed
- ✅ Cost: €1-2 (components)
- ✅ Can be very reliable

**Cons**:
- ❌ Complex to design for specific timing
- ❌ Less accurate than RTC
- ❌ Requires circuit knowledge
- ❌ Difficult to adjust timing

**Use Case**: Custom-designed solutions, learning projects

---

### **Option 5: WiFi Module with Sleep (For AC Power)**

**Pros**:
- ✅ Cloud-based scheduling
- ✅ Remote control possible
- ✅ No ATtiny needed

**Cons**:
- ❌ Requires WiFi
- ❌ Higher power consumption
- ❌ Higher cost (~€15-30)
- ❌ Not suitable for battery-powered remote locations

**Use Case**: AC-powered or WiFi-enabled locations

---

## 📊 Comparison Table

| Solution | Cost | Power | Accuracy | Ease | Recommended |
|----------|------|-------|----------|------|-------------|
| ATtiny85 (Current) | €8-10 | Excellent | Good | Medium | ✅ |
| **RTC DS3231** | €2-3 | Excellent | Excellent | Easy | ⭐ BEST |
| Software Sleep | €0 | Good | Fair | Easiest | ✅ OK |
| Relay Timer | €3-5 | Fair | Fair | Easy | ⚠️ Limited |
| 555 Timer | €1-2 | Fair | Fair | Hard | ⚠️ Complex |
| WiFi Module | €15-30 | Poor | Excellent | Medium | ❌ Overkill |

---

## 🔧 Implementation Guide

### **Solution #1: RTC DS3231 (RECOMMENDED)**

#### Hardware Setup

```
DS3231 Module Pinout:
  VCC → Arduino 5V
  GND → Arduino GND
  SDA → Arduino A4
  SCL → Arduino A5
  INT → Arduino D2 (optional, for interrupt)
  32K → Not used
```

#### Code Implementation

Add to `arduino/arduino.ino`:

```cpp
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;
DateTime nextWakeTime;

void setup() {
    // Initialize RTC
    if (!rtc.begin()) {
        LOG_ERROR("RTC not found!");
        while (1);  // Halt if no RTC
    }

    // Set RTC time if needed (only once)
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Set first wake time (now + 120 minutes)
    DateTime now = rtc.now();
    nextWakeTime = now + TimeSpan(0, 2, 0, 0);  // 2 hours

    LOG_INFO("RTC initialized");
}

void Request() {
    // ... existing code ...
    done();
}

void done() {
    LOG_INFO("Setting RTC wake time...");

    // Calculate next wake: now + 120 minutes
    DateTime now = rtc.now();
    DateTime wakeTime = now + TimeSpan(0, 2, 0, 0);  // 2 hours

    // Set RTC alarm to wake Arduino
    rtc.setAlarm1(
        wakeTime,
        DS3231_A1_Date  // Alarm on date/hour/minute
    );

    // Enable alarm interrupt
    rtc.enableAlarm1(true);

    LOG_INFO_VAL("Next wake: ", wakeTime.unixtime());

    // Power down Arduino
    scale.power_down();
    digitalWrite(GSM_POWER_PIN, LOW);

    // Setup interrupt for RTC alarm
    pinMode(2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(2), wakeUp, FALLING);

    // Enter sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();
}

void wakeUp() {
    // Called when RTC alarm triggers
    sleep_disable();
    LOG_INFO("Woken by RTC alarm!");
}
```

#### Library Installation

Arduino IDE → Sketch → Include Library → Manage Libraries
Search for: **RTClib by Adafruit**
Install version 2.1.1 or later

#### Cost: ~€2-3

---

### **Solution #2: Software Sleep (NO HARDWARE CHANGE)**

#### Code Implementation

Replace `done()` function in `arduino/arduino.ino`:

```cpp
#include <avr/wdt.h>

// Global variable to track wake time
static unsigned long sleepStartTime = 0;
volatile bool shouldWake = false;

// Interrupt handler for watchdog
ISR(WDT_vect) {
    // Watchdog triggered
    // Check if it's been 2 hours
    if (millis() - sleepStartTime > 7200000) {  // 2 hours in ms
        shouldWake = true;
    }
}

void done() {
    LOG_INFO("Entering sleep mode for 2 hours...");

    // Power down components
    scale.power_down();
    digitalWrite(GSM_POWER_PIN, LOW);
    mySerial.end();  // Disable serial
    Serial.end();    // Disable serial

    // Record sleep start time
    sleepStartTime = millis();
    shouldWake = false;

    // Configure watchdog for periodic interrupt
    // This wakes from sleep every 8 seconds to check
    wdt_enable(WDTO_8S);
    WDTCSR |= _BV(WDIE);  // Enable watchdog interrupt

    // Setup sleep mode (power save, still uses watchdog)
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    sleep_enable();

    // Sleep until 2 hours have passed
    while (!shouldWake) {
        sleep_mode();
    }

    // Woken up
    sleep_disable();
    wdt_disable();

    // Reinitialize serial and GSM
    Serial.begin(9600);
    mySerial.begin(9600);
    delay(500);

    LOG_INFO("Woken from sleep!");
    Power_UP();
    Request();
}
```

#### Cost: €0 (No additional hardware)

#### Trade-offs:
- Battery life: ~15-20% reduction
- Simplicity: Maximum
- Flexibility: Can easily change 2-hour interval
- Reliability: Good enough for most applications

---

### **Solution #3: Relay Timer Module**

#### Hardware

```
Relay Timer Module (e.g., DC 12V relay timer):
  IN+ → Arduino D3 (via 220Ω resistor)
  IN- → Arduino GND
  NO/NC → MOSFET gate circuit (replaces ATtiny)
  Coil voltage: 12V DC (from USB or power supply)
```

#### Code Implementation

Remove ATtiny-related code, add relay activation:

```cpp
#define RELAY_PIN 3

void setup() {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);  // Relay off
}

void done() {
    LOG_INFO("Activating relay timer for next cycle...");

    // Power down
    scale.power_down();
    digitalWrite(GSM_POWER_PIN, LOW);

    // Activate relay timer (it will power on Arduino after set time)
    digitalWrite(RELAY_PIN, HIGH);
    delay(100);
    digitalWrite(RELAY_PIN, LOW);

    LOG_INFO("Relay timer activated. Arduino will power off.");

    // This Arduino will shutdown when relay timer opens MOSFET
    while(1) {
        delay(1000);  // Wait for power-down
    }
}
```

#### Drawbacks:
- Less flexible (fixed timing)
- Relay coil power consumption
- Mechanical wear over time
- Limited to simple on/off

---

### **Solution #4: Remove Sleep Entirely (Always-On)**

#### For AC-Powered Systems

```cpp
#define MEASUREMENT_INTERVAL_MS  7200000  // 2 hours

static unsigned long lastMeasurement = 0;

void setup() {
    // Normal setup
}

void loop() {
    // Arduino stays always on and awake
    if (millis() - lastMeasurement >= MEASUREMENT_INTERVAL_MS) {
        Power_UP();
        Request();
        lastMeasurement = millis();
    }

    // Sleep briefly to save power (but not complete shutdown)
    delay(10000);  // Sleep 10 seconds between checks
}
```

#### Use Cases:
- ✅ USB-powered systems
- ✅ AC-powered installations
- ✅ Locations with reliable power
- ❌ Not suitable for battery-powered remote stations

---

## 🚀 Migration Steps

### **To Switch from ATtiny85 to RTC (Recommended)**

**Step 1: Remove ATtiny Hardware**
```
- Remove ATtiny85 from breadboard/PCB
- Remove MOSFET control from ATtiny pin 0
- Remove Arduino signal wire from ATtiny pin 1
- Keep MOSFET for power control (controlled differently now)
```

**Step 2: Install RTC Module**
```
- Connect DS3231 module:
  VCC → 5V
  GND → GND
  SDA → A4
  SCL → A5
  INT → D2 (optional)
```

**Step 3: Update Code**
```cpp
// Add at top of arduino.ino
#include <Wire.h>
#include <RTClib.h>
RTC_DS3231 rtc;

// In setup():
if (!rtc.begin()) {
    LOG_ERROR("RTC failed to initialize!");
    while (1);
}

// Replace done() function with RTC version
```

**Step 4: Install Library**
- Arduino IDE → Sketch → Include Library → Manage Libraries
- Search: "RTClib by Adafruit"
- Install latest version

**Step 5: Test**
- Upload code
- Monitor serial output
- Verify RTC initialization
- Check wake cycle timing

---

## ⚖️ Decision Matrix

### Choose RTC If:
- ✅ Need reliable, accurate timing
- ✅ Want ultra-low power consumption
- ✅ Want maximum flexibility
- ✅ Have I2C capability (Arduino has it)
- **RECOMMENDATION: YES - Use RTC**

### Choose Software Sleep If:
- ✅ Want simplest solution
- ✅ Don't mind ~15% more power draw
- ✅ Want zero additional hardware
- ✅ Good for testing/prototyping
- **RECOMMENDATION: OK - Works fine**

### Choose Always-On If:
- ✅ Have AC power available
- ✅ Don't need battery operation
- ✅ Want maximum simplicity
- ❌ Not recommended for remote beehives

### Choose Relay Timer If:
- ✅ Need fixed timing only
- ✅ Want mechanical reliability
- ❌ Flexibility is not needed

---

## 📊 Power Consumption Comparison

### ATtiny85 Approach (Current)
```
Per 2-hour cycle:
  Active: 40 sec @ 500mA = 5.5 mAh
  Sleep: 7160 sec @ 10µA = 0.02 mAh
  Total: 5.52 mAh

Battery life (4x AA @ 2000mAh each):
  8000 mAh ÷ 5.52 mAh = ~1,450 cycles
  1,450 cycles × 2 hours = ~121 days (~4 months)
```

### RTC Approach
```
Per 2-hour cycle:
  Active: 40 sec @ 500mA = 5.5 mAh
  Sleep: 7160 sec @ 0.1µA (RTC) = 0.0002 mAh
  Total: 5.50 mAh

Battery life (4x AA @ 2000mAh each):
  8000 mAh ÷ 5.50 mAh = ~1,454 cycles
  1,454 cycles × 2 hours = ~121 days (~4 months)

DIFFERENCE: <1% (essentially identical)
```

### Software Sleep Approach
```
Per 2-hour cycle:
  Active: 40 sec @ 500mA = 5.5 mAh
  Sleep: 7160 sec @ 1mA = 1.99 mAh
  Total: 7.49 mAh

Battery life (4x AA @ 2000mAh each):
  8000 mAh ÷ 7.49 mAh = ~1,068 cycles
  1,068 cycles × 2 hours = ~89 days (~3 months)

DIFFERENCE: -26% (shorter battery life)
```

---

## 🎯 Recommendation

### **Primary**: RTC DS3231 Module
- ✅ Best power consumption (same as ATtiny85)
- ✅ Better accuracy
- ✅ Lower cost (~€2-3 vs €8-10)
- ✅ More flexible
- ✅ No complex programming
- ✅ Proven in industry

### **Alternative**: Software Sleep
- ✅ Zero hardware cost
- ✅ Good enough for many applications
- ✅ Simplest to implement
- ⚠️ 26% more power consumption
- ⚠️ Not ideal for remote/battery-powered

### **Not Recommended**:
- ❌ Relay timer (too inflexible)
- ❌ 555 timer (too complex)
- ❌ Always-on (too much power)
- ❌ WiFi solution (overkill)

---

## 📝 Summary

| Aspect | RTC | Software Sleep |
|--------|-----|-----------------|
| Power Consumption | Excellent | Good |
| Cost | €2-3 | €0 |
| Ease of Implementation | Easy | Easy |
| Accuracy | Excellent | Fair |
| Flexibility | Excellent | Good |
| Maintenance | None | None |
| **RATING** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |

---

## 🔗 Next Steps

1. **Choose Solution**: RTC recommended
2. **Get Hardware**: DS3231 module (~€2-3)
3. **Install Library**: RTClib by Adafruit
4. **Update Code**: Follow implementation guide
5. **Test**: Verify wake timing
6. **Deploy**: Remove ATtiny85, install RTC

---

**Created**: 2025-11-15
**Project**: ArduiBeeScale
**Author**: Jeremy JEANNE
