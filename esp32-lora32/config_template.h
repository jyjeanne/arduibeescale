/**
 * ArduiBeeScale ESP32 LoRa32 Configuration Template
 *
 * FOR: DollaTek WiFi LoRa 32 868MHz / Heltec WiFi LoRa 32 V2
 *      with built-in OLED 0.96" SSD1306 display
 *
 * INSTRUCTIONS:
 * 1. Copy this file to 'config.h' in the same folder
 * 2. Update all values marked with "CHANGE THIS" comments
 * 3. Do NOT commit config.h to version control (add to .gitignore)
 *
 * This file configures:
 * - WiFi network credentials
 * - MQTT broker connection (Home Assistant Mosquitto add-on)
 * - Hive identification
 * - Scale calibration
 * - Power management
 * - Built-in OLED display settings
 */

#ifndef CONFIG_H
#define CONFIG_H

//============================================
// BOARD: DollaTek / Heltec WiFi LoRa 32 V2
//============================================
// Features:
// - ESP32 with WiFi + LoRa 868MHz (SX1276)
// - Built-in OLED 0.96" SSD1306 (128x64)
// - Built-in battery management with ADC
// - LED on GPIO25
// - PRG button on GPIO0

//============================================
// WiFi CONFIGURATION
//============================================
// Your home WiFi network OR Rock Pi beehive WiFi AP

#define WIFI_SSID            "BeehiveNet"         // CHANGE THIS: Your WiFi network name
                                                   // Use "BeehiveNet" if using Rock Pi AP
#define WIFI_PASSWORD        "YourWiFiPassword"   // CHANGE THIS: Your WiFi password

//============================================
// MQTT BROKER CONFIGURATION
//============================================
// Home Assistant Mosquitto Broker (on Rock Pi or Raspberry Pi)

#define MQTT_BROKER          "192.168.4.1"        // CHANGE THIS: Your Rock Pi IP address
                                                   // Rock Pi AP default: 192.168.4.1
                                                   // Home network: Your server IP (e.g., 192.168.1.100)

#define MQTT_PORT            1883                 // Default MQTT port (no SSL)

// MQTT Authentication
#define MQTT_USER            "beehive"            // CHANGE THIS: MQTT username
#define MQTT_PASSWORD        "mqtt_password"      // CHANGE THIS: MQTT password

//============================================
// HIVE IDENTIFICATION
//============================================
// Unique identifiers for this beehive
// IMPORTANT: Each hive must have a unique HIVE_ID

#define HIVE_ID              "hive01"             // CHANGE THIS: Unique ID (no spaces, lowercase)
                                                   // Examples: "hive01", "hive02", "garden_hive"

#define HIVE_NAME            "Beehive 1"          // CHANGE THIS: Friendly name shown in Home Assistant

//============================================
// SCALE CALIBRATION
//============================================
// You MUST calibrate your scale for accurate weight readings
// See CALIBRATION.md for step-by-step instructions

#define SCALE_CALIBRATION    -21500.0             // CHANGE THIS: Your calibration factor
#define SCALE_OFFSET         0L                   // CHANGE THIS: Your scale offset (tare value)
#define SCALE_SAMPLES        10                   // Number of readings to average

//============================================
// BATTERY CONFIGURATION (Built-in on LoRa32)
//============================================
// The LoRa32 board has a built-in voltage divider on GPIO37
// Connected to battery via 220K/100K divider (ratio ~3.2)

#define BATTERY_DIVIDER_RATIO      3.2            // LoRa32 built-in divider ratio
                                                   // Adjust if readings are inaccurate

#define BATTERY_CALIBRATION_OFFSET 0.0            // Fine-tune ADC reading (volts)

//============================================
// MEASUREMENT INTERVAL
//============================================
// How often to wake up and send data

#define SLEEP_INTERVAL_HOURS 2                    // Hours between measurements
                                                   // Battery life estimates (18650 3000mAh):
                                                   // 1 hour  interval: ~3-4 months
                                                   // 2 hour  interval: ~6-8 months
                                                   // 4 hour  interval: ~12+ months

//============================================
// OLED DISPLAY CONFIGURATION (Built-in)
//============================================
// The LoRa32 has a built-in SSD1306 OLED 0.96" (128x64)
// Display shows sensor values when button is pressed

#define OLED_ENABLED                              // Comment out to disable OLED completely

#ifdef OLED_ENABLED
#define OLED_WIDTH           128                  // OLED display width in pixels
#define OLED_HEIGHT          64                   // OLED display height in pixels
#define OLED_I2C_ADDRESS     0x3C                 // I2C address (0x3C for SSD1306)
#define OLED_DISPLAY_TIME_MS 2500                 // Time each value is displayed (ms)
#endif

//============================================
// DEBUG SETTINGS
//============================================
#define DEBUG_ENABLED        1                    // Set to 0 for production
                                                   // 1 = Serial output enabled (115200 baud)
                                                   // 0 = Silent operation (saves power)

//============================================
// ADVANCED SETTINGS
//============================================
// Home Assistant MQTT Discovery prefix
#define HA_DISCOVERY_PREFIX  "homeassistant"

//============================================
// PIN MAPPING - DollaTek/Heltec WiFi LoRa 32 V2
//============================================
/*
 * IMPORTANT: This board has specific pin assignments!
 *
 * BUILT-IN (do not change):
 * GPIO4  - OLED SDA (I2C data)
 * GPIO15 - OLED SCL (I2C clock)
 * GPIO16 - OLED RST (reset)
 * GPIO21 - Vext (OLED/sensor power control)
 * GPIO25 - LED (active HIGH)
 * GPIO37 - Battery ADC (built-in voltage divider)
 * GPIO0  - PRG Button (active LOW, boot mode)
 *
 * LoRa SX1276 (optional, not used for WiFi mode):
 * GPIO5  - LoRa SCK
 * GPIO19 - LoRa MISO
 * GPIO27 - LoRa MOSI
 * GPIO18 - LoRa CS
 * GPIO14 - LoRa RST
 * GPIO26 - LoRa DIO0
 *
 * AVAILABLE FOR SENSORS:
 * GPIO13 - HX711 DOUT (Data)
 * GPIO12 - HX711 SCK (Clock)
 * GPIO17 - DHT22 Data
 * GPIO0  - Display button (uses PRG button)
 * GPIO2  - Alternative button (active LOW)
 *
 * AVOID USING:
 * GPIO34-39 - Input only (no pull-up)
 * GPIO6-11  - Connected to flash (do not use!)
 */

//============================================
// NETWORK ARCHITECTURE
//============================================
/*
 * This board connects to your Rock Pi server via WiFi:
 *
 * ┌─────────────────────┐         ┌─────────────────────┐
 * │  BEEHIVE            │  WiFi   │  ROCK PI 6          │
 * │  ┌───────────────┐  │ ~~~~~~> │  ┌───────────────┐  │
 * │  │ LoRa32 + OLED │  │         │  │ Home Assistant│  │
 * │  │ + Sensors     │  │         │  │ + Mosquitto   │  │
 * │  └───────────────┘  │         │  └───────────────┘  │
 * │  Battery powered    │         │  WiFi AP: BeehiveNet│
 * └─────────────────────┘         │  IP: 192.168.4.1    │
 *                                 └─────────────────────┘
 *
 * The LoRa radio is NOT used - we use WiFi for simplicity.
 * LoRa could be added later for very long range (2-10 km).
 */

#endif // CONFIG_H
