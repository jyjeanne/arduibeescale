# HX711 Scale Calibration Guide

This guide explains how to calibrate your load cell and HX711 module for accurate weight measurements.

## Overview

The HX711 outputs raw ADC values that need to be converted to real weight. Two values control this:

- **SCALE_CALIBRATION**: Converts raw units to kilograms
- **SCALE_OFFSET**: The raw reading when the scale is empty (tare)

## Equipment Needed

- Known reference weight (e.g., 5kg dumbbells, 10kg bag of sugar)
- USB cable for ESP32
- Computer with Arduino IDE

## Step-by-Step Calibration

### Step 1: Prepare Calibration Firmware

Edit `config.h` temporarily:

```cpp
// Temporary calibration values
#define SCALE_CALIBRATION    1.0    // Raw output
#define SCALE_OFFSET         0L     // No offset (L suffix for long type)
#define DEBUG_ENABLED        1      // Enable serial output (1=on, 0=off)

// Optionally reduce sleep for faster testing
#define SLEEP_INTERVAL_HOURS 0.01   // ~36 seconds (for testing only!)
```

### Step 2: Get the Zero Offset

1. **Assemble the scale** with the load cell mounted but **nothing on it**

2. Upload the firmware and open Serial Monitor (115200 baud)

3. Watch the weight readings. With an empty scale, you'll see something like:
   ```
   [DEBUG] Raw weight: -145.67 kg
   ```

4. **Note this value**: This is your raw offset (e.g., `-145.67` → offset should be around `-145670` in raw units)

5. To get the exact raw offset, we need to read it directly. You can temporarily add this to setup():
   ```cpp
   // Add after scale.begin() for calibration
   Serial.printf("Raw offset (no weight): %ld\n", scale.read_average(20));
   ```

6. **Record the raw offset value** (e.g., `145670`)

### Step 3: Get the Calibration Factor

1. Place a **known weight** on the scale (the heavier the better for accuracy)
   - Use at least 5kg, preferably 10-20kg
   - Make sure it's stable and centered

2. Watch the Serial Monitor output:
   ```
   [DEBUG] Raw weight: 358.42 kg  (this is wrong because calibration = 1.0)
   ```

3. **Calculate the calibration factor**:
   ```
   calibration = (raw_reading - offset) / known_weight_kg

   Example with 10kg weight:
   - Raw reading: -214350
   - Raw offset: -145670
   - Delta: -214350 - (-145670) = -68680

   calibration = -68680 / 10 = -6868.0
   ```

   Or simpler, if using calibration=1.0 and offset=0:
   ```
   calibration = displayed_weight / known_weight

   Example:
   - Displayed: 358.42 kg
   - Known: 10 kg
   - Calibration = 358.42 / 10 = 35.842
   - But we want it to show 10, so: calibration = 35842 (raw units per kg)
   ```

### Step 4: Apply Calibration Values

Update `config.h` with your calculated values:

```cpp
#define SCALE_CALIBRATION    -21500.0    // Your calculated factor
#define SCALE_OFFSET         -145670     // Your recorded offset
```

### Step 5: Verify Calibration

1. Upload the updated firmware

2. Test with the known weight:
   ```
   [INFO] Sensors: Weight=10.02kg ...
   ```

3. Test with empty scale:
   ```
   [INFO] Sensors: Weight=0.01kg ...
   ```

4. If still off, fine-tune the values:
   - Reading too high → increase calibration factor (more negative if negative)
   - Reading too low → decrease calibration factor (less negative if negative)

### Step 6: Production Settings

Once calibrated, restore production settings:

```cpp
#define SLEEP_INTERVAL_HOURS 2          // Normal interval
#define DEBUG_ENABLED        0          // Disable for battery life (0=off)
```

## Calibration Tips

### Sign of Calibration Factor

- The sign (+ or -) depends on load cell wiring
- If weight shows negative, flip the sign of SCALE_CALIBRATION
- Or swap the A+ and A- wires on the HX711

### Temperature Effects

Load cells are temperature-sensitive. For best accuracy:
- Calibrate at the expected operating temperature
- Or implement temperature compensation in software

### Load Cell Quality

- Cheap load cells can have 1-3% non-linearity
- For better accuracy, use a calibrated/certified load cell
- Consider using a 4-load-cell platform for heavier loads

### Mounting

- Load cell must be mounted rigidly
- No flexing or vibration during measurement
- Protect from moisture (affects readings)

## Calibration Code Snippet

For manual calibration testing, you can use this standalone sketch:

```cpp
#include "HX711.h"

#define HX711_DOUT_PIN  16
#define HX711_SCK_PIN   17

HX711 scale;

void setup() {
    Serial.begin(115200);
    Serial.println("HX711 Calibration");

    scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);

    Serial.println("Remove all weight from scale");
    delay(5000);

    scale.tare();  // Reset to zero
    long offset = scale.get_offset();
    Serial.printf("Offset: %ld\n", offset);

    Serial.println("Place known weight on scale (enter weight in kg via Serial)");
}

void loop() {
    if (scale.is_ready()) {
        long reading = scale.get_value(10);
        Serial.printf("Raw: %ld\n", reading);

        if (Serial.available()) {
            float known = Serial.parseFloat();
            if (known > 0) {
                float cal = reading / known;
                Serial.printf("Known: %.2f kg\n", known);
                Serial.printf("Calibration factor: %.2f\n", cal);
                Serial.println("Use this value for SCALE_CALIBRATION");
            }
        }
    }
    delay(1000);
}
```

## Common Issues

### Readings Jump Around

- Add more samples: `scale.get_units(20)` instead of `10`
- Check for electrical interference
- Ensure stable power supply

### Readings Drift Over Time

- Temperature change affecting load cell
- Load cell creep (normal for cheap cells)
- Consider periodic tare (not practical for beehive)

### Weight Shows Zero

- Check wiring to HX711
- Verify HX711 is powered (3.3V or 5V)
- Try different HX711 gain: `scale.set_gain(64)` or `scale.set_gain(128)`

---

## Quick Reference

| Parameter | Description | How to Find |
|-----------|-------------|-------------|
| SCALE_OFFSET | Raw value with empty scale | `scale.read_average(20)` with nothing on scale |
| SCALE_CALIBRATION | Raw units per kg | `(raw_reading) / known_weight_kg` |
