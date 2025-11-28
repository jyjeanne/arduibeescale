# ArduiBeeScale - LoRa32 Edition

**Beehive monitoring with DollaTek/Heltec WiFi LoRa 32 and built-in OLED display**

This variant uses the DollaTek WiFi LoRa 32 868MHz board (Heltec compatible) with its built-in 0.96" OLED screen.

---

## Board Features

| Feature | Description |
|---------|-------------|
| **MCU** | ESP32 with WiFi + Bluetooth |
| **LoRa** | SX1276 868MHz (not used - WiFi mode) |
| **Display** | Built-in SSD1306 OLED 0.96" (128x64) |
| **Battery** | Built-in charging circuit + ADC |
| **LED** | Built-in on GPIO25 |
| **Button** | PRG button on GPIO0 |

> **Note**: This version uses **WiFi** to connect to your Rock Pi/Raspberry Pi server. The LoRa radio is available for future long-range expansion but is not used in this firmware.

---

## Network Architecture

```
┌─────────────────────────┐         ┌─────────────────────────┐
│      BEEHIVE            │  WiFi   │     ROCK PI 6           │
│  ┌─────────────────┐    │ ~~~~~~> │  ┌─────────────────┐    │
│  │ LoRa32 + OLED   │    │         │  │ Home Assistant  │    │
│  │ + DHT22 + HX711 │    │         │  │ + Mosquitto     │    │
│  └─────────────────┘    │         │  └─────────────────┘    │
│  Battery: 6-12 months   │         │  WiFi AP: BeehiveNet    │
│  Press PRG for display  │         │  IP: 192.168.4.1        │
└─────────────────────────┘         └─────────────────────────┘
```

---

## Shopping List

### LoRa32 Beehive Unit (~€32)

| Qty | Component | Purpose | Price |
|-----|-----------|---------|-------|
| 1 | DollaTek WiFi LoRa 32 868MHz | Main board with OLED | €12.00 |
| 1 | HX711 Module | Weight sensor amplifier | €1.00 |
| 1 | 50kg Load Cell | Weight measurement | €8.00 |
| 1 | DHT22 (AM2302) | Temperature & humidity | €3.00 |
| 1 | 18650 Battery | Power source | €4.00 |
| 1 | Waterproof Enclosure | Outdoor protection | €4.00 |

**Total per hive: ~€32** (€4 less than ESP32-WROOM-32U version!)

### What You DON'T Need

- ~~External WiFi antenna~~ (built-in)
- ~~LCD display~~ (OLED built-in)
- ~~Push button~~ (PRG button built-in)
- ~~Battery voltage divider resistors~~ (built-in ADC)

---

## Wiring Diagram

### Pin Connections

```
         DollaTek/Heltec WiFi LoRa 32
        ┌─────────────────────────────┐
        │  [OLED 0.96"]    [Antenna]  │
        │  ┌──────────┐               │
        │  │          │               │
        │  └──────────┘               │
        │                             │
  DHT22 │ GPIO17 ◄──────── DATA      │
        │ 3V3    ◄──────── VCC       │
        │ GND    ◄──────── GND       │
        │                             │
  HX711 │ GPIO13 ◄──────── DT (Data) │
        │ GPIO12 ◄──────── SCK       │
        │ 3V3    ◄──────── VCC       │
        │ GND    ◄──────── GND       │
        │                             │
Battery │ BAT+   ◄──────── 18650 +   │
        │ BAT-   ◄──────── 18650 -   │
        │                             │
        │ [USB-C]  [PRG]  [RST]      │
        └─────────────────────────────┘
```

### Connection Table

| LoRa32 Pin | Connect To | Notes |
|------------|------------|-------|
| GPIO17 | DHT22 DATA | Temperature/humidity |
| GPIO13 | HX711 DT | Load cell data |
| GPIO12 | HX711 SCK | Load cell clock |
| 3V3 | DHT22 VCC, HX711 VCC | Sensor power |
| GND | DHT22 GND, HX711 GND | Common ground |
| BAT+ | 18650 positive | Battery input |
| BAT- | 18650 negative | Battery ground |

### Built-in Components (No wiring needed)

| Component | GPIO | Notes |
|-----------|------|-------|
| OLED SDA | GPIO4 | Fixed - I2C data |
| OLED SCL | GPIO15 | Fixed - I2C clock |
| OLED RST | GPIO16 | Fixed - Reset |
| Vext Power | GPIO21 | Controls OLED power |
| LED | GPIO25 | Status indicator |
| PRG Button | GPIO0 | Display trigger |
| Battery ADC | GPIO37 | Built-in divider |

---

## Setup Instructions

### Step 1: Install Arduino IDE Libraries

If using Arduino IDE, install these libraries:

1. **Sketch → Include Library → Manage Libraries**
2. Search and install:
   - `PubSubClient` by Nick O'Leary
   - `ArduinoJson` by Benoit Blanchon
   - `DHT sensor library` by Adafruit
   - `HX711 Arduino Library` by Bogdan Necula
   - `Adafruit SSD1306` by Adafruit
   - `Adafruit GFX Library` by Adafruit

### Step 2: Add ESP32 Board Support

1. **File → Preferences**
2. Add to "Additional Board URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. **Tools → Board → Boards Manager**
4. Search "esp32" and install **esp32 by Espressif Systems**

### Step 3: Select Board

**Tools → Board → ESP32 Arduino → Heltec WiFi LoRa 32(V2)**

If not available, use: **ESP32 Dev Module**

### Step 4: Configure Your Settings

1. Copy `config_template.h` to `config.h`
2. Edit `config.h`:

```cpp
// WiFi - Your Rock Pi beehive network
#define WIFI_SSID         "BeehiveNet"
#define WIFI_PASSWORD     "YourPassword"

// MQTT - Rock Pi server
#define MQTT_BROKER       "192.168.4.1"    // Rock Pi AP IP
#define MQTT_USER         "beehive"
#define MQTT_PASSWORD     "your_mqtt_password"

// Hive Identity
#define HIVE_ID           "hive01"
#define HIVE_NAME         "Garden Hive"
```

### Step 5: Upload Firmware

1. Connect LoRa32 via USB-C
2. Select correct COM port: **Tools → Port**
3. Click **Upload**
4. If upload fails, hold **PRG** button while clicking Upload

### Step 6: Verify Connection

1. Open **Serial Monitor** (115200 baud)
2. Press **RST** button on board
3. You should see:
   ```
   ================================================
      ArduiBeeScale LoRa32 Edition v4.1
      DollaTek/Heltec WiFi LoRa 32
   ================================================
   [INFO] Connecting to WiFi...
   [INFO] WiFi connected! IP: 192.168.4.11
   [INFO] Connecting to MQTT broker...
   [INFO] MQTT connected!
   [INFO] Data published successfully!
   ```

---

## Using the OLED Display

### Automatic Display (during transmission)

When the device wakes up to send data:
1. Shows "Connecting..." status
2. Shows "DATA SENT" on success
3. Shows error message on failure

### Manual Display (button press)

Press the **PRG button** anytime to view sensor values:

1. **Splash screen** - ArduiBeeScale + hive name
2. **Temperature** - Current temperature in °C
3. **Humidity** - Current humidity in %
4. **Weight** - Hive weight in kg
5. **Battery** - Battery percentage
6. **Battery voltage** - Actual voltage
7. **WiFi Signal** - RSSI in dBm

Each screen displays for 2.5 seconds, then the display turns off to save power.

### Wake from Sleep

If the device is sleeping, pressing PRG will:
1. Wake up the ESP32
2. Show sensor values on OLED
3. Return to deep sleep (no WiFi connection)

---

## Calibration

Same as ESP32 version. See [../esp32/CALIBRATION.md](../esp32/CALIBRATION.md)

Quick steps:
1. Set `DEBUG_ENABLED 1` in config.h
2. Upload and open Serial Monitor
3. Note weight with empty scale
4. Place known weight (e.g., 10 kg)
5. Calculate calibration factor
6. Update `SCALE_CALIBRATION` and `SCALE_OFFSET` in config.h

---

## Home Assistant Setup

This version works identically with Home Assistant. Follow the main guide:
- [../esp32/HOME_ASSISTANT_SETUP.md](../esp32/HOME_ASSISTANT_SETUP.md)
- [../esp32/home_assistant_examples.yaml](../esp32/home_assistant_examples.yaml)

The device appears in Home Assistant as "ArduiBeeScale LoRa32" with the same sensors:
- Weight (kg)
- Temperature (°C)
- Humidity (%)
- Battery (%)
- WiFi Signal (dBm)

---

## Comparison: LoRa32 vs ESP32-WROOM-32U

| Feature | ESP32-WROOM-32U | LoRa32 (This version) |
|---------|-----------------|----------------------|
| **Cost per hive** | ~€36 | ~€32 |
| **Display** | External LCD (optional) | Built-in OLED |
| **WiFi Antenna** | External (better range) | Internal |
| **WiFi Range** | ~100m | ~50m |
| **Battery Monitor** | DIY voltage divider | Built-in |
| **Size** | Larger | Compact |
| **Future LoRa** | No | Yes (2-10 km range) |

**Choose LoRa32 if:**
- You want built-in OLED display
- Your beehives are within 50m of WiFi AP
- You want a more compact solution
- You might want LoRa long-range in the future

**Choose ESP32-WROOM-32U if:**
- You need maximum WiFi range (100m+)
- You prefer larger LCD display
- You already have the components

---

## Troubleshooting

### OLED Not Working

- Check Vext power (GPIO21 must be LOW to power OLED)
- The firmware handles this automatically
- Try pressing RST button

### Upload Failed

- Hold PRG button while clicking Upload
- Release PRG after "Connecting..." appears
- Try different USB cable (data cable, not charge-only)

### WiFi Connection Issues

- LoRa32 internal antenna has ~50m range
- Move closer to Rock Pi for testing
- Check SSID and password in config.h
- Ensure Rock Pi WiFi AP is running

### Battery Reading Incorrect

- Adjust `BATTERY_DIVIDER_RATIO` in config.h
- Default is 3.2 for LoRa32 built-in divider
- Measure actual voltage and compare to reported value

### HX711 Not Responding

- Check wiring: GPIO13 → DT, GPIO12 → SCK
- Ensure 3V3 power to HX711
- Check load cell connections to HX711

---

## Files in This Folder

| File | Description |
|------|-------------|
| `esp32_lora32_beescale.ino` | Main firmware for LoRa32 |
| `config_template.h` | Configuration template |
| `platformio.ini` | PlatformIO project file |
| `README.md` | This documentation |

---

## License

GNU General Public License v3.0 - See [../LICENSE](../LICENSE)
