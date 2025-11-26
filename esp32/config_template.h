/**
 * ArduiBeeScale ESP32 Configuration Template
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
 */

#ifndef CONFIG_H
#define CONFIG_H

//============================================
// WiFi CONFIGURATION
//============================================
// Your home WiFi network credentials

#define WIFI_SSID            "YourWiFiSSID"        // CHANGE THIS: Your WiFi network name
#define WIFI_PASSWORD        "YourWiFiPassword"    // CHANGE THIS: Your WiFi password

//============================================
// MQTT BROKER CONFIGURATION
//============================================
// Home Assistant Mosquitto Broker add-on settings
// Go to: Settings -> Add-ons -> Mosquitto Broker -> Configuration

#define MQTT_BROKER          "192.168.1.100"       // CHANGE THIS: Your Raspberry Pi IP address
                                                    // Find it: Settings -> System -> Network
                                                    // Or run: hostname -I on the Pi

#define MQTT_PORT            1883                  // Default MQTT port (no SSL)
                                                    // Use 8883 for SSL (requires certificate setup)

// MQTT Authentication (if enabled in Mosquitto add-on)
// Create a user in Home Assistant: Settings -> People -> Users
// Or use a local user created in Mosquitto configuration
#define MQTT_USER            "mqtt_user"           // CHANGE THIS: MQTT username
#define MQTT_PASSWORD        "mqtt_password"       // CHANGE THIS: MQTT password

// Comment out the above two lines if authentication is disabled:
// #define MQTT_USER         "mqtt_user"
// #define MQTT_PASSWORD     "mqtt_password"

//============================================
// HIVE IDENTIFICATION
//============================================
// Unique identifiers for this beehive
// IMPORTANT: Each hive must have a unique HIVE_ID

#define HIVE_ID              "hive01"              // CHANGE THIS: Unique ID (no spaces, lowercase)
                                                    // Examples: "hive01", "hive02", "garden_hive"
                                                    // This appears in MQTT topics and HA entity IDs

#define HIVE_NAME            "Beehive 1"           // CHANGE THIS: Friendly name shown in Home Assistant
                                                    // Examples: "Garden Hive", "Apiary North #1"

//============================================
// SCALE CALIBRATION
//============================================
// You MUST calibrate your scale for accurate weight readings
// See CALIBRATION.md for step-by-step instructions

#define SCALE_CALIBRATION    -21500.0              // CHANGE THIS: Your calibration factor
                                                    // Positive or negative depending on load cell wiring
                                                    // Typical range: -25000 to 25000

#define SCALE_OFFSET         0L                    // CHANGE THIS: Your scale offset (tare value)
                                                    // Raw reading when scale is empty (use L suffix for long)

#define SCALE_SAMPLES        10                    // Number of readings to average
                                                    // Higher = more stable, slower

//============================================
// BATTERY CONFIGURATION
//============================================
// Settings for Li-Ion battery monitoring
// Uses voltage divider: Battery+ -> R1 (100K) -> ADC -> R2 (100K) -> GND

#define BATTERY_DIVIDER_RATIO      2.0             // Voltage divider ratio (R1 + R2) / R2
                                                    // For equal resistors (100K + 100K): ratio = 2.0
                                                    // For 200K + 100K: ratio = 3.0

#define BATTERY_CALIBRATION_OFFSET 0.0             // Fine-tune ADC reading (volts)
                                                    // Measure actual battery voltage and compare
                                                    // with reported value, add/subtract difference

//============================================
// MEASUREMENT INTERVAL
//============================================
// How often to wake up and send data

#define SLEEP_INTERVAL_HOURS 2                     // Hours between measurements
                                                    // Recommended values:
                                                    // 1  = High frequency (shorter battery life)
                                                    // 2  = Balanced (default, good for most cases)
                                                    // 4  = Low frequency (longer battery life)
                                                    // 6  = Very low frequency (maximum battery life)
                                                    //
                                                    // Battery life estimates (18650 3000mAh):
                                                    // 1 hour  interval: ~3-4 months
                                                    // 2 hour  interval: ~6-8 months
                                                    // 4 hour  interval: ~12+ months

//============================================
// LCD 1602 I2C DISPLAY (Optional)
//============================================
// Enable LCD display with button to show sensor values on-demand
// Press the button to cycle through: Temp -> Humidity -> Weight -> Battery

#define LCD_ENABLED                                // Comment out this line to DISABLE LCD support
                                                    // Saves ~5mA when LCD backlight is off
                                                    // Saves code space if LCD not used

#ifdef LCD_ENABLED
#define LCD_I2C_ADDRESS      0x27                  // I2C address of LCD (0x27 or 0x3F typical)
                                                    // Run I2C scanner if unsure:
                                                    // File -> Examples -> Wire -> i2c_scanner

#define LCD_DISPLAY_TIME_MS  2000                  // Time each value is displayed (milliseconds)
                                                    // 2000 = 2 seconds per screen
                                                    // Total display time = 4 screens × 2s = 8 seconds
#endif

// Button behavior:
// - During normal operation: press to display values on LCD
// - During deep sleep: press to wake up and display values (no WiFi connection)
// - After display cycle: ESP32 returns to deep sleep

//============================================
// DEBUG SETTINGS
//============================================
// Enable serial output for debugging
// DISABLE in production to save power

#define DEBUG_ENABLED        1                     // Set to 0 for production
                                                    // 1 = Serial output enabled (115200 baud)
                                                    // 0 = Silent operation (saves ~1mA)

//============================================
// ADVANCED SETTINGS
//============================================
// Normally you don't need to change these

// WiFi TX Power (for external antenna)
// Higher = better range but more power consumption
// WIFI_POWER_19_5dBm = Maximum (default, good for external antenna)
// WIFI_POWER_15dBm   = Medium
// WIFI_POWER_8_5dBm  = Low (save power if hive is close to router)

// Home Assistant MQTT Discovery prefix
// Only change if you modified Home Assistant's default
#define HA_DISCOVERY_PREFIX  "homeassistant"

//============================================
// PIN MAPPING REFERENCE (ESP32-WROOM-32U)
//============================================
/*
 * Default pin assignments (change in main .ino if needed):
 *
 * SENSORS:
 * GPIO16 - HX711 DOUT (Data)
 * GPIO17 - HX711 SCK  (Clock)
 * GPIO4  - DHT22 Data
 * GPIO34 - Battery voltage (ADC, input only)
 * GPIO2  - LED (optional status indicator)
 *
 * LCD 1602 I2C (Optional):
 * GPIO21 - I2C SDA (LCD data)
 * GPIO22 - I2C SCL (LCD clock)
 * GPIO13 - Push button (to GND, uses internal pull-up)
 *
 * Power connections:
 * VIN/5V - USB power input (programming only)
 * 3V3    - Power to DHT22, HX711, LCD
 * GND    - Common ground
 *
 * External Antenna:
 * U.FL   - Connect 3dBi antenna via U.FL to SMA pigtail
 *
 * Battery:
 * VIN    - Can be powered from TP4056 output (3.7-4.2V)
 *          Use onboard regulator or add LDO for stability
 */

//============================================
// STEP-BY-STEP SETUP CHECKLIST
//============================================
/*
 * Before uploading:
 *
 * [ ] 1. Install ESP32 board in Arduino IDE
 *        File -> Preferences -> Additional Board URLs:
 *        https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *
 * [ ] 2. Install required libraries:
 *        - PubSubClient (by Nick O'Leary)
 *        - ArduinoJson (by Benoit Blanchon)
 *        - DHT sensor library (by Adafruit)
 *        - HX711 Arduino Library (by Bogdan Necula)
 *        - LiquidCrystal I2C (by Frank de Brabander) - if using LCD
 *
 * [ ] 3. Select board:
 *        Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module
 *
 * [ ] 4. Select correct port:
 *        Tools -> Port -> (your COM port)
 *
 * [ ] 5. Update this config file:
 *        - WiFi credentials
 *        - MQTT broker IP
 *        - MQTT credentials
 *        - Hive ID and name
 *        - Scale calibration (after calibration procedure)
 *        - LCD settings (if using LCD display)
 *
 * [ ] 6. Upload code
 *
 * [ ] 7. Monitor Serial output (115200 baud) to verify connection
 *
 * [ ] 8. Check Home Assistant for new entities
 *
 * [ ] 9. (Optional) Disable DEBUG_ENABLED for production
 *
 * [ ] 10. (Optional) Test LCD button - press to display sensor values
 */

#endif // CONFIG_H
