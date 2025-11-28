# ArduiBeeScale ESP32 Edition

**Complete beehive monitoring system** with ESP32, Home Assistant, and email alerts.

Monitor your beehives remotely: weight, temperature, humidity, battery level, and weather alerts.

---

## Table of Contents

1. [Quick Start Checklist](#quick-start-checklist)
2. [What You Need](#what-you-need-prerequisites)
3. [System Overview](#system-overview)
4. [Shopping List](#shopping-list)
5. [Step-by-Step Setup Guide](#step-by-step-setup-guide)
6. [Available Alerts](#available-alerts-summary)
7. [Adding Multiple Beehives](#adding-multiple-beehives) ← **For 2+ hives**
8. [Project Files](#project-files)
9. [Troubleshooting](#troubleshooting)
10. [Solution Comparison](#solution-comparison)

---

## Quick Start Checklist

Follow these steps in order:

- [ ] **Step 1**: Buy components (~€120 total)
- [ ] **Step 2**: Install Home Assistant on Raspberry Pi
- [ ] **Step 3**: Install Mosquitto MQTT broker add-on
- [ ] **Step 4**: Setup free weather API (OpenWeatherMap)
- [ ] **Step 5**: Wire the ESP32 and sensors
- [ ] **Step 6**: Upload firmware to ESP32
- [ ] **Step 7**: Calibrate the scale
- [ ] **Step 8**: Configure Home Assistant dashboard & alerts

**Estimated setup time**: 2-4 hours (first time)

---

## What You Need (Prerequisites)

### Skills Required
- Basic soldering (or use Dupont connectors)
- Installing software on Raspberry Pi
- Copying/pasting YAML configuration

### Tools Required
- Computer with USB port
- Soldering iron (optional)
- Multimeter (helpful for debugging)
- Known weight for calibration (5-10 kg)

---

## System Overview

```
┌─────────────────────────┐         ┌─────────────────────────┐
│      BEEHIVE            │  WiFi   │     RASPBERRY PI        │
│  ┌─────────────────┐    │ ~~~~~~> │  ┌─────────────────┐    │
│  │ ESP32 + Sensors │    │         │  │ Home Assistant  │    │
│  │ - Weight        │    │         │  │ - Dashboard     │    │
│  │ - Temperature   │    │         │  │ - Charts        │    │
│  │ - Humidity      │    │         │  │ - Email Alerts  │    │
│  │ - Battery       │    │         │  │ - Weather API   │    │
│  └─────────────────┘    │         │  └─────────────────┘    │
│  Battery: 6-12 months   │         │  Mosquitto MQTT Broker  │
└─────────────────────────┘         └─────────────────────────┘
```

### Key Features
- **Long battery life**: 6-12 months on single 18650 cell
- **Extended WiFi range**: External 3dBi antenna on both ESP32 and Pi
- **Auto-discovery**: Sensors appear automatically in Home Assistant
- **Multi-hive support**: Monitor 2-10+ hives with comparison charts
- **Email alerts**: Temperature, weight, battery, swarm detection, storms
- **Weather alerts**: Free API monitors storms and sends warnings
- **LCD display** (optional): Press button to see values on screen
- **100% local**: No cloud, no subscription, your data stays home

---

## Shopping List

### Beehive Unit Option A: ESP32-WROOM-32U (~€36)

Best WiFi range (100m+) with external antenna.

| Qty | Component | Purpose | Price |
|-----|-----------|---------|-------|
| 1 | ESP32-WROOM-32U | Main controller (has U.FL antenna connector) | €4.50 |
| 1 | 3dBi WiFi Antenna + U.FL pigtail | Extended WiFi range | €2.00 |
| 1 | HX711 Module | Weight sensor amplifier | €1.00 |
| 1 | 50kg Load Cell | Weight measurement | €8.00 |
| 1 | DHT22 (AM2302) | Temperature & humidity sensor | €3.00 |
| 1 | 18650 Battery + Holder | Power source | €5.50 |
| 1 | TP4056 Module (USB-C) | Battery charger with protection | €1.00 |
| 2 | 100KΩ Resistors | Battery voltage divider | €0.20 |
| 1 | Waterproof Enclosure | Outdoor protection | €6.00 |
| - | Wires, connectors | Assembly | €5.00 |

### Beehive Unit Option B: DollaTek/Heltec LoRa32 (~€32)

Built-in OLED display, more compact, €4 cheaper. See [../esp32-lora32/](../esp32-lora32/)

| Qty | Component | Purpose | Price |
|-----|-----------|---------|-------|
| 1 | DollaTek WiFi LoRa 32 868MHz | Board with built-in OLED 0.96" | €12.00 |
| 1 | HX711 Module | Weight sensor amplifier | €1.00 |
| 1 | 50kg Load Cell | Weight measurement | €8.00 |
| 1 | DHT22 (AM2302) | Temperature & humidity sensor | €3.00 |
| 1 | 18650 Battery | Power source (built-in charger) | €4.00 |
| 1 | Waterproof Enclosure | Outdoor protection | €4.00 |

> **LoRa32 Advantages**: Built-in OLED display, built-in battery charger, built-in battery ADC, more compact. No external components needed for display or battery monitoring.
>
> **LoRa32 Limitations**: Internal WiFi antenna (~50m range vs 100m). Use for hives closer to the AP.

### Optional Add-ons

| Component | Purpose | Price |
|-----------|---------|-------|
| LCD 1602 I2C + Button | On-device display (ESP32-WROOM-32U only) | €2.70 |
| Solar Panel 6V 1W | Infinite battery life | €5.00 |

### Server Unit (~€84-97)

**Option A: Raspberry Pi 4** (Simpler setup)
| Component | Purpose | Price |
|-----------|---------|-------|
| Raspberry Pi 4 (2GB+) | Home Assistant server | €45.00 |
| MicroSD Card 32GB+ | Operating system | €8.00 |
| Power Supply 5V 3A | Pi power | €10.00 |
| Case with cooling | Protection | €10.00 |
| Ethernet cable | Network connection | €3.00 |
| USB WiFi Dongle + Antenna | Extended range to beehives | €8.00 |
| **Subtotal** | | **~€84** |

**Option B: Rock Pi 4/6** (More powerful, dedicated beehive WiFi)
| Component | Purpose | Price |
|-----------|---------|-------|
| Rock Pi 4/6 (4GB+) | Home Assistant server | €60.00 |
| MicroSD Card 32GB+ | Operating system | €8.00 |
| Power Supply 5V 3A | Power | €10.00 |
| Case | Protection | €8.00 |
| Ethernet cable | Network connection | €3.00 |
| USB WiFi Dongle + Antenna | **Dedicated beehive WiFi AP** | €8.00 |
| **Subtotal** | | **~€97** |

**Option B+ (Recommended): Rock Pi 4/6 with SSD** (Maximum reliability)
| Component | Purpose | Price |
|-----------|---------|-------|
| Rock Pi 4/6 (4GB+) | Home Assistant server | €60.00 |
| MicroSD Card 32GB+ | Operating system boot | €8.00 |
| **NVMe/SATA SSD 128GB+** | **Data storage (prevents SD wear)** | €25.00 |
| **SSD Extension Card** | **Connects SSD to Rock Pi** | €15.00 |
| Power Supply 5V 3A | Power | €10.00 |
| Case | Protection | €8.00 |
| Ethernet cable | Network connection | €3.00 |
| USB WiFi Dongle + Antenna | **Dedicated beehive WiFi AP** | €8.00 |
| **Subtotal** | | **~€137** |

> **Rock Pi Advantage**: Create a **separate WiFi network** just for beehives (e.g., "BeehiveNet"), isolated from your home network. Better security and reliability.
>
> **SSD Advantage**: Home Assistant writes frequently to disk (database, logs). MicroSD cards fail after 1-2 years of heavy use. SSD provides 10+ years reliability. See [ROCKPI_SETUP.md](ROCKPI_SETUP.md)

### Total Cost

| Setup | Cost |
|-------|------|
| 1 hive (ESP32-WROOM) + Raspberry Pi | ~€120 |
| 1 hive (LoRa32) + Raspberry Pi | ~€116 |
| 1 hive (ESP32-WROOM) + Rock Pi + SSD | ~€173 |
| 1 hive (LoRa32) + Rock Pi + SSD | ~€169 |
| 1 hive + existing server | ~€36 (ESP32) / ~€32 (LoRa32) |
| Each additional hive | ~€36 (ESP32) / ~€32 (LoRa32) |

---

## Step-by-Step Setup Guide

### Step 1: Prepare Raspberry Pi with Home Assistant

**Time: 30-60 minutes**

1. **Download Home Assistant OS**
   - Go to: https://www.home-assistant.io/installation/raspberrypi
   - Download image for your Pi model

2. **Flash SD card**
   - Download Balena Etcher: https://www.balena.io/etcher/
   - Flash the downloaded image to SD card

3. **First boot**
   - Insert SD card into Raspberry Pi
   - Connect Ethernet cable to router
   - Connect power supply
   - Wait 10-20 minutes for initial setup

4. **Access Home Assistant**
   - Open browser: `http://homeassistant.local:8123`
   - Create your account
   - Complete the setup wizard

5. **Note your Raspberry Pi IP address**
   - Go to: Settings → System → Network
   - Write down the IP (e.g., `192.168.1.100`)
   - **Important**: Set a static IP in your router settings

---

### Step 2: Install Mosquitto MQTT Broker

**Time: 10 minutes**

1. In Home Assistant: **Settings → Add-ons → Add-on Store**

2. Search for **"Mosquitto broker"** → Click **Install**

3. Go to **Configuration** tab and set:
   ```yaml
   logins:
     - username: beehive
       password: your_secure_password
   ```
   (Replace `your_secure_password` with your own password)

4. Click **Save**

5. Go to **Info** tab → Click **Start**

6. Enable **Start on boot** and **Watchdog**

7. Setup MQTT integration:
   - Go to: **Settings → Devices & Services → + Add Integration**
   - Search for **"MQTT"** → Select it
   - It should auto-detect Mosquitto
   - Enter username/password from step 3
   - Click **Submit**

---

### Step 3: Setup Free Weather API

**Time: 10 minutes**

For storm alerts, you need a free weather API. **OpenWeatherMap** is recommended.

1. **Create free account**
   - Go to: https://openweathermap.org/api
   - Click **Sign Up** (top right)
   - Verify your email
   - Go to: https://home.openweathermap.org/api_keys
   - Copy your API key (takes 10-30 min to activate)

2. **Add to Home Assistant**
   - Go to: **Settings → Devices & Services → + Add Integration**
   - Search for **"OpenWeatherMap"**
   - Enter your API key
   - Enter your city name (e.g., "Paris", "Lyon, FR")
   - Select **"Free"** mode
   - Click **Submit**

3. **Verify it works**
   - Go to: **Developer Tools → States**
   - Search for `weather.openweathermap`
   - You should see current conditions

> **Alternative (no registration)**: Use **Open-Meteo** via HACS - completely free with no API key.

---

### Step 4: Wire the Hardware

**Time: 30-60 minutes**

> **Detailed diagrams**: See [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md)

#### Quick Connection Reference

| ESP32 Pin | Connect To | Wire Color |
|-----------|------------|------------|
| GPIO4 | DHT22 DATA pin | Yellow |
| GPIO16 | HX711 DT (data) pin | Green |
| GPIO17 | HX711 SCK (clock) pin | Blue |
| GPIO34 | Voltage divider middle | Orange |
| GPIO21 | LCD SDA (optional) | White |
| GPIO22 | LCD SCL (optional) | Purple |
| GPIO13 | Button to GND (optional) | Gray |
| 3V3 | DHT22 VCC, HX711 VCC, LCD VCC | Red |
| GND | All ground connections | Black |
| VIN | TP4056 OUT+ | Red |

#### Simplified Wiring

```
                    ESP32-WROOM-32U
                   ┌───────────────┐
                   │   [Antenna]   │
    DHT22 DATA ───►│ GPIO4    3V3 │◄─── DHT22/HX711/LCD power
   HX711 DOUT ────►│ GPIO16   GND │◄─── All grounds
    HX711 SCK ────►│ GPIO17       │
     LCD SDA ─────►│ GPIO21       │ (optional)
     LCD SCL ─────►│ GPIO22       │ (optional)
      Button ─────►│ GPIO13       │ (optional, to GND)
  Battery ADC ────►│ GPIO34  VIN  │◄─── TP4056 OUT+
                   └───────────────┘
```

#### Battery Voltage Divider

```
TP4056 OUT+ ────┬─────► ESP32 VIN
                │
             [100KΩ]
                │
                ├─────► ESP32 GPIO34
                │
             [100KΩ]
                │
GND ────────────┴─────► ESP32 GND
```

---

### Step 5: Upload Firmware to ESP32

**Time: 20-30 minutes**

#### 5.1 Install Arduino IDE

1. Download from: https://www.arduino.cc/en/software
2. Install and open Arduino IDE

#### 5.2 Add ESP32 Board Support

1. Go to: **File → Preferences**
2. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Click **OK**
4. Go to: **Tools → Board → Boards Manager**
5. Search for **"esp32"**
6. Install **"esp32 by Espressif Systems"**

#### 5.3 Install Required Libraries

Go to: **Sketch → Include Library → Manage Libraries**

Install these (search by name):
- **PubSubClient** by Nick O'Leary
- **ArduinoJson** by Benoit Blanchon
- **DHT sensor library** by Adafruit
- **HX711 Arduino Library** by Bogdan Necula
- **LiquidCrystal I2C** by Frank de Brabander (if using LCD)

#### 5.4 Configure Your Settings

1. Copy `config_template.h` to `config.h`:
   ```
   Copy config_template.h → Rename to config.h
   ```

2. Edit `config.h` with your settings:
   ```cpp
   // WiFi - Your home network
   #define WIFI_SSID         "YourWiFiName"
   #define WIFI_PASSWORD     "YourWiFiPassword"

   // MQTT - Your Raspberry Pi
   #define MQTT_BROKER       "192.168.1.100"    // Pi's IP address
   #define MQTT_USER         "beehive"           // From Mosquitto setup
   #define MQTT_PASSWORD     "your_secure_password"

   // Hive Identity
   #define HIVE_ID           "hive01"            // Unique ID
   #define HIVE_NAME         "Garden Beehive"    // Display name
   ```

#### 5.5 Upload to ESP32

1. Connect ESP32 to computer via USB
2. Select board: **Tools → Board → ESP32 Dev Module**
3. Select port: **Tools → Port → (your COM port)**
4. Click **Upload** (arrow button)
5. If upload fails, hold **BOOT** button while uploading

#### 5.6 Verify Connection

1. Open **Serial Monitor** (Tools → Serial Monitor)
2. Set baud rate to **115200**
3. Press **RESET** button on ESP32
4. You should see:
   ```
   [INFO] Connecting to WiFi...
   [INFO] WiFi connected!
   [INFO] Connecting to MQTT...
   [INFO] MQTT connected!
   [INFO] Sensors appear in Home Assistant!
   ```

---

### Step 6: Calibrate the Scale

**Time: 15-30 minutes**

> **Detailed guide**: See [CALIBRATION.md](CALIBRATION.md)

#### Quick Calibration

1. **Get empty scale reading**
   - With nothing on scale, note the displayed weight
   - This is your "offset" value

2. **Get calibration factor**
   - Place known weight (e.g., 10 kg) on scale
   - Note the displayed weight
   - Calculate: `calibration = displayed_weight / actual_weight`

3. **Update config.h**
   ```cpp
   #define SCALE_CALIBRATION    -21500.0    // Your calculated value
   #define SCALE_OFFSET         0           // Adjust if needed
   ```

4. **Upload and verify**
   - Upload firmware again
   - Check weight matches known weight

---

### Step 7: Setup Home Assistant Dashboard & Alerts

**Time: 30-60 minutes**

> **Complete guide**: See [HOME_ASSISTANT_SETUP.md](HOME_ASSISTANT_SETUP.md)
> **Ready-to-use YAML**: See [home_assistant_examples.yaml](home_assistant_examples.yaml)

#### 7.1 Verify Sensors Appear

After ESP32 connects, check:
- Go to: **Settings → Devices & Services → MQTT**
- You should see your beehive device with sensors:
  - `sensor.beehive_1_weight`
  - `sensor.beehive_1_temperature`
  - `sensor.beehive_1_humidity`
  - `sensor.beehive_1_battery`
  - `sensor.beehive_1_wifi_signal`

#### 7.2 Setup Email Notifications

1. Edit `configuration.yaml` (via File Editor add-on):
   ```yaml
   notify:
     - name: email_beehive_alert
       platform: smtp
       server: smtp.gmail.com
       port: 587
       sender: your_email@gmail.com
       username: your_email@gmail.com
       password: !secret gmail_app_password
       recipient:
         - your_email@gmail.com
       sender_name: "Beehive Monitor"
   ```

2. For Gmail, create App Password:
   - Go to: https://myaccount.google.com/apppasswords
   - Generate password for "Home Assistant"

3. Add to `secrets.yaml`:
   ```yaml
   gmail_app_password: "xxxx xxxx xxxx xxxx"
   ```

#### 7.3 Create Alert Automations

Import automations from [home_assistant_examples.yaml](home_assistant_examples.yaml):

| Alert | Trigger |
|-------|---------|
| Temperature Alert | Below 27°C (configurable) |
| Weight Alert | Below your threshold |
| Swarm Detection | Sudden weight drop >1.5 kg |
| Low Battery | Below 20% |
| Hive Offline | No data for 6 hours |
| Storm Alert | Lightning, hail, high wind |
| Daily Summary | 8 AM daily report |

---

## Available Alerts Summary

| Alert Type | Trigger | Email Notification |
|------------|---------|-------------------|
| **Temperature** | Below threshold (default 27°C) | Yes |
| **Weight** | Below user-defined limit | Yes |
| **Swarm Detection** | Weight drops >1.5 kg suddenly | Yes |
| **Low Battery** | Below 20% | Yes |
| **Offline** | No data for 6+ hours | Yes |
| **Storm Warning** | Lightning, hail, high wind | Yes |
| **Storm Forecast** | Severe weather predicted | Yes |
| **Daily Summary** | Every day at 8 AM | Yes |

All thresholds are configurable via Home Assistant UI sliders - no YAML editing needed!

---

## Adding Multiple Beehives

Each additional hive needs its own ESP32 with a unique `HIVE_ID`:

| Hive | config.h Setting | Entity Example |
|------|------------------|----------------|
| Hive 1 | `#define HIVE_ID "hive01"` | `sensor.beehive_1_weight` |
| Hive 2 | `#define HIVE_ID "hive02"` | `sensor.beehive_2_weight` |
| Hive 3 | `#define HIVE_ID "hive03"` | `sensor.beehive_3_weight` |

**Cost per additional hive**: ~€36 (ESP32-WROOM) or ~€32 (LoRa32)

> **Tip**: You can mix ESP32-WROOM-32U and LoRa32 boards. Use LoRa32 for hives close to the AP (built-in display), ESP32-WROOM-32U for distant hives (better WiFi range).

See [HOME_ASSISTANT_SETUP.md](HOME_ASSISTANT_SETUP.md#7-multiple-beehives-setup) for:
- Multi-hive dashboard cards
- Comparison charts (weight, temperature)
- Apiary-wide alerts
- Template sensors (total weight, average temp, lowest battery)

---

## Project Files

| File | Description |
|------|-------------|
| `esp32_beescale.ino` | Main ESP32-WROOM-32U firmware |
| `config_template.h` | Configuration template (copy to `config.h`) |
| `WIRING_DIAGRAM.md` | Detailed wiring diagrams |
| `CALIBRATION.md` | Scale calibration guide |
| `HOME_ASSISTANT_SETUP.md` | Complete HA setup with email alerts + multi-hive |
| `home_assistant_examples.yaml` | Ready-to-use YAML configurations |
| `platformio.ini` | PlatformIO configuration (alternative to Arduino IDE) |
| `ROCKPI_SETUP.md` | Rock Pi setup with dedicated beehive WiFi network |
| **[../esp32-lora32/](../esp32-lora32/)** | **DollaTek/Heltec LoRa32 variant with OLED** |

---

## Troubleshooting

### ESP32 Won't Connect to WiFi

- [ ] Check WiFi credentials in `config.h`
- [ ] Ensure 2.4 GHz network (ESP32 doesn't support 5 GHz)
- [ ] Check antenna is connected properly
- [ ] Move closer to router for testing

### MQTT Connection Failed

- [ ] Verify Mosquitto add-on is running
- [ ] Check MQTT credentials match Mosquitto config
- [ ] Verify Raspberry Pi IP address is correct
- [ ] Test with: Developer Tools → MQTT → Listen to `#`

### Sensors Not in Home Assistant

- [ ] Check MQTT integration is configured
- [ ] Reset ESP32 to resend discovery messages
- [ ] Check Home Assistant logs for errors

### Inaccurate Weight

- [ ] Re-calibrate with known weight (see [CALIBRATION.md](CALIBRATION.md))
- [ ] Check load cell wiring
- [ ] Ensure load cell is flat and stable

### Short Battery Life

- [ ] Set `DEBUG_ENABLED` to `false` in `config.h`
- [ ] Increase `SLEEP_INTERVAL_HOURS` (4-6 hours)
- [ ] Add solar panel for indefinite operation

### Weather Alerts Not Working

- [ ] Verify weather integration is configured
- [ ] Check entity name matches `weather.home` or update automations
- [ ] Enable `storm_alerts_enabled` toggle in HA

---

## Support

- **Issues**: Open an issue on GitHub
- **Home Assistant help**: https://community.home-assistant.io/

---

## Solution Comparison

### ArduiBeeScale Variants

This project offers three DIY variants. Choose based on your needs:

| Criteria | Arduino + ESP-01 | ESP32-WROOM-32U | LoRa32 (OLED) |
|----------|------------------|-----------------|---------------|
| **Cost (1 hive + server)** | ~€95 | ~€120 | ~€116 |
| **Cost (hive only)** | ~€56 | ~€36 | ~€32 |
| **Complexity** | ⭐⭐⭐ Medium | ⭐⭐ Easy | ⭐ Easiest |
| **Setup Time** | 6-8 hours | 2-4 hours | 1-2 hours |
| **Battery Life** | 3-4 months | 6-12 months | 6-12 months |
| **WiFi Range** | ~50m | ~100m (ext. antenna) | ~50m (internal) |
| **Built-in Display** | No | No (optional LCD) | **Yes (OLED)** |
| **Server Software** | Python + Flask | Home Assistant | Home Assistant |
| **Email Alerts** | Manual | Built-in | Built-in |
| **Weather Alerts** | No | Yes | Yes |
| **Auto-Discovery** | No | Yes (MQTT) | Yes (MQTT) |
| **Deep Sleep** | ~1mA | ~20µA | ~20µA |
| **Future LoRa** | No | No | **Yes (2-10km)** |

### Detailed Comparison

#### Arduino Uno + ESP-01 (Original Version)

```
Location: ../arduino/
```

**Advantages:**
- Lower initial cost for single hive
- Uses common Arduino Uno (easy to find/replace)
- AA batteries available everywhere
- Fully custom - complete control over software
- Works without internet (100% local)
- No subscription or cloud dependency

**Disadvantages:**
- More complex wiring (2 boards + level shifter)
- ESP-01 WiFi setup can be tricky
- Custom server requires Python knowledge
- No smartphone app (web only)
- Limited community support
- Manual alert configuration
- Shorter battery life
- Weaker WiFi signal

**Best For:**
- Makers who want full control
- Budget-conscious single hive setup
- Users with Python/Linux experience
- Areas without Home Assistant infrastructure

---

#### ESP32-WROOM-32U (This Version)

```
Location: ./esp32/ (this folder)
```

**Advantages:**
- Simpler wiring (single board)
- Better WiFi range (external antenna)
- Longer battery life (true deep sleep)
- Home Assistant integration (large community)
- Auto-discovery (sensors appear automatically)
- Built-in weather/storm alerts
- Email notifications ready-to-use
- Professional dashboard with charts
- Easy to add more hives
- Better documentation

**Disadvantages:**
- Higher initial cost (includes Home Assistant server)
- Requires Home Assistant knowledge
- 18650 batteries less common than AA
- External antenna required for best range
- More complex for those new to HA

**Best For:**
- Users already using Home Assistant
- Multi-hive apiaries
- Those wanting email/weather alerts
- Beginners who want step-by-step guide
- Long-term reliable monitoring
- Hives far from WiFi AP (100m range)

---

#### DollaTek/Heltec WiFi LoRa 32 (LoRa32 Edition)

```
Location: ../esp32-lora32/
```

**Advantages:**
- **Cheapest option** (~€32 per hive)
- **Built-in OLED display** - no external screen needed
- Built-in battery charger and ADC
- Most compact form factor
- Easiest wiring (fewer components)
- Future LoRa expansion possible (2-10 km range)
- Same Home Assistant integration as ESP32

**Disadvantages:**
- Internal WiFi antenna (~50m range)
- Smaller display than LCD 1602
- Less common board (may need to order online)

**Best For:**
- Budget-conscious builds
- Hives within 50m of WiFi AP
- Users who want built-in display
- Compact installations
- Future long-range LoRa expansion plans

---

### Commercial Solutions Comparison

| Solution | Initial Cost | Monthly Cost | Battery | Data | Alerts |
|----------|-------------|--------------|---------|------|--------|
| **ArduiBeeScale LoRa32** | €116 | €0 | 6-12 mo | Local | Email + Weather |
| **ArduiBeeScale ESP32** | €120 | €0 | 6-12 mo | Local | Email + Weather |
| **ArduiBeeScale Arduino** | €95 | €0 | 3-4 mo | Local | Manual |
| BroodMinder-W (weight) | €89 | €0 | 1 year | Cloud | App |
| BroodMinder-T2 (temp) | €45 | €0 | 1 year | Cloud | App |
| Arnia (complete) | €200+ | €5-10 | Solar | Cloud | Email |
| HiveWatch | €150+ | €8 | Solar | Cloud | SMS |
| Solutionbee | €300+ | €8 | Solar | Cloud | Email |

### Decision Guide

```
START
  │
  ▼
┌─────────────────────────────────┐
│ Do you use Home Assistant?      │
└─────────────────────────────────┘
         │              │
        YES            NO
         │              │
         ▼              ▼
┌─────────────────┐  ┌─────────────────────────┐
│ Hive distance   │  │ Want email/storm alerts?│
│ from WiFi AP?   │  └─────────────────────────┘
└─────────────────┘           │           │
    │         │              YES         NO
  <50m      >50m              │           │
    │         │               ▼           ▼
    ▼         ▼        ┌──────────┐ ┌──────────────┐
┌────────┐ ┌────────┐  │ ESP32 or │ │ Arduino+ESP01│
│LoRa32  │ │ESP32   │  │ LoRa32   │ │ (simpler)    │
│(OLED)  │ │(range) │  │ + setup  │ │              │
│€32/hive│ │€36/hive│  │ Home     │ │              │
└────────┘ └────────┘  │ Assistant│ │              │
                       └──────────┘ └──────────────┘
```

### Cost Breakdown by Scenario

| Scenario | Arduino | ESP32 | LoRa32 | Best Value |
|----------|---------|-------|--------|------------|
| 1 hive + new Raspberry Pi | €95 | €120 | €116 | Arduino |
| 1 hive + new Rock Pi + SSD | €135 | €173 | €169 | Arduino |
| 1 hive + existing server | €56 | €36 | €32 | **LoRa32** |
| 2 hives + new server | €151 | €156 | €148 | **LoRa32** |
| 3 hives + new server | €207 | €192 | €180 | **LoRa32** |
| 5 hives + new server | €319 | €264 | €244 | **LoRa32** |
| 10 hives + new server | €599 | €444 | €404 | **LoRa32** |

**Conclusion**:
- **Cheapest per hive** → LoRa32 (~€32/hive)
- **Best WiFi range** → ESP32-WROOM-32U (100m)
- **Built-in display** → LoRa32 (OLED included)
- **Maximum reliability** → Any ESP32 + Rock Pi + SSD
- **Already have Home Assistant** → LoRa32 or ESP32

### Feature Matrix

| Feature | Arduino | ESP32 | LoRa32 | BroodMinder | Arnia |
|---------|---------|-------|--------|-------------|-------|
| Weight | ✅ | ✅ | ✅ | ✅ | ✅ |
| Temperature | ✅ | ✅ | ✅ | ✅ | ✅ |
| Humidity | ✅ | ✅ | ✅ | ❌ | ✅ |
| Battery Monitor | ❌ | ✅ | ✅ Built-in | ✅ | ✅ |
| WiFi Signal | ❌ | ✅ | ✅ | ❌ | ✅ |
| Email Alerts | ⚠️ Manual | ✅ | ✅ | ✅ App | ✅ |
| Weather Alerts | ❌ | ✅ | ✅ | ❌ | ❌ |
| Swarm Detection | ⚠️ Manual | ✅ | ✅ | ✅ | ✅ |
| Display | ✅ Optional | ✅ Optional | ✅ **Built-in** | ❌ | ❌ |
| WiFi Range | ~50m | ~100m | ~50m | N/A | N/A |
| Charts/History | ✅ | ✅ | ✅ | ✅ | ✅ |
| Multi-Hive | ⚠️ Complex | ✅ Easy | ✅ Easy | ✅ | ✅ |
| Data Privacy | ✅ Local | ✅ Local | ✅ Local | ❌ Cloud | ❌ Cloud |
| No Subscription | ✅ | ✅ | ✅ | ✅ | ❌ |
| Open Source | ✅ | ✅ | ✅ | ❌ | ❌ |
| DIY Repairable | ✅ | ✅ | ✅ | ❌ | ❌ |
| Future LoRa | ❌ | ❌ | ✅ 2-10km | ❌ | ❌ |

---

## License

GNU General Public License v3.0 - See [LICENSE](../LICENSE)
