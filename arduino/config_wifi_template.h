/**
 * ArduiBeeScale Configuration Template - WiFi + MQTT Edition
 *
 * Copy this file to config_wifi.h and update with your settings.
 * This file should NOT be committed to version control.
 *
 * IMPORTANT: Update these values for your specific setup:
 */

#ifndef CONFIG_WIFI_H
#define CONFIG_WIFI_H

// ==========================================
// WiFi Configuration
// ==========================================
// Your local WiFi network credentials

#define WIFI_SSID            "your_ssid"           // Your WiFi network name (SSID)
#define WIFI_PASSWORD        "your_password"       // Your WiFi password

// ==========================================
// MQTT Broker Configuration
// ==========================================
// Raspberry Pi local MQTT broker settings

#define MQTT_BROKER          "192.168.1.100"       // Your Raspberry Pi IP address
                                                    // Find it with: hostname -I
                                                    // Usually: 192.168.1.X (check your router)

#define MQTT_PORT            1883                  // Standard MQTT port (no TLS/SSL)

#define MQTT_CLIENT_ID       "beehive-001"         // Unique identifier for this hive
                                                    // Should match Arduino name/number
                                                    // Example: "beehive-001", "beehive-002"

#define MQTT_TOPIC           "beehive/hive-001"    // Topic path for this hive's data
                                                    // Format: beehive/hive-NNN
                                                    // Python subscriber listens to: beehive/#

#define MQTT_KEEPALIVE       60                    // Seconds between keep-alive pings

// ==========================================
// Hardware Configuration
// ==========================================
// Arduino pin assignments for sensors

#define HX711_DOUT_PIN       5                     // HX711 data pin (yellow wire)
#define HX711_CLK_PIN        6                     // HX711 clock pin (white wire)
#define DHTPIN               10                    // DHT22 data pin
#define BATTERY_PIN          A0                    // Battery voltage measurement pin

#define ESP_RX_PIN           8                     // Arduino RX pin (to ESP-01 TX)
#define ESP_TX_PIN           9                     // Arduino TX pin (to ESP-01 RX)
                                                    // IMPORTANT: Use level shifter for 5V->3.3V!

// ==========================================
// Scale Calibration
// ==========================================
// Customize these values for your load cell
// See: https://github.com/bogde/HX711/wiki for calibration procedure

#define SCALE_CALIBRATION    -19689.35             // Calibration factor
                                                    // Steps:
                                                    // 1. Upload code with known weight
                                                    // 2. Place 1kg on scale
                                                    // 3. Note the raw reading
                                                    // 4. SCALE = raw_reading / 1000

#define SCALE_OFFSET         -145680               // Calibration offset
                                                    // With no weight on scale:
                                                    // OFFSET = average_raw_reading

// ==========================================
// Measurement Settings
// ==========================================
// Measurement cycle timing

#define SLEEP_INTERVAL_HOURS 2                     // Time between measurements (hours)
                                                    // 2 hours = good for monitoring
                                                    // 1 hour = more frequent updates (more power)
                                                    // 4 hours = longer battery life

// ==========================================
// LCD1602 I2C Display Configuration
// ==========================================
// 16x2 character display with I2C backpack

#define ENABLE_LCD              true                // Set to true to enable LCD display
#define LCD_I2C_ADDRESS         0x27                // LCD I2C address (0x27 or 0x3F)
                                                    // Find yours: Upload I2C_SCANNER.ino first
                                                    // See: LCD_HARDWARE_SETUP.md for details

#define BUTTON_PIN              2                   // Push button digital pin
                                                    // Uses INT0 interrupt (Pin 2 or 3)

#define DISPLAY_DURATION_MS     2000                // How long each screen displays (ms)
                                                    // 2000 = 2 seconds per screen
                                                    // 1000 = 1 second per screen

// ==========================================
// Debug Settings
// ==========================================
// Serial output control

#define DEBUG_LEVEL          DEBUG_INFO            // Log level:
                                                    // DEBUG_OFF      - No output
                                                    // DEBUG_ERRORS   - Errors only
                                                    // DEBUG_INFO     - Errors + info
                                                    // DEBUG_VERBOSE  - All messages

// ==========================================
// STEP-BY-STEP SETUP GUIDE
// ==========================================
/*
 * 1. Find Your Raspberry Pi IP Address
 *    ├─ SSH into Pi: ssh pi@raspberrypi.local
 *    ├─ Run: hostname -I
 *    ├─ Example output: 192.168.1.100
 *    └─ Use this IP for MQTT_BROKER
 *
 * 2. Verify WiFi Network
 *    ├─ SSID = Your WiFi network name (visible in WiFi list)
 *    └─ PASSWORD = Your WiFi password
 *
 * 3. Assign MQTT Client ID and Topic
 *    ├─ MQTT_CLIENT_ID "beehive-001" = First hive
 *    ├─ MQTT_CLIENT_ID "beehive-002" = Second hive
 *    ├─ Match the numbers: beehive-001 and beehive/hive-001
 *    └─ Python subscriber listens to beehive/# (all hives)
 *
 * 4. Calibrate Your Scale
 *    ├─ Follow HX711 calibration procedure
 *    ├─ Update SCALE_CALIBRATION value
 *    └─ Update SCALE_OFFSET value
 *
 * 5. Configure LCD Display (if using)
 *    ├─ Find LCD I2C address: Upload I2C_SCANNER.ino first
 *    ├─ Update LCD_I2C_ADDRESS (0x27 or 0x3F)
 *    ├─ Connect button to BUTTON_PIN (default pin 2)
 *    ├─ See LCD_HARDWARE_SETUP.md for wiring details
 *    └─ Adjust DISPLAY_DURATION_MS if needed (2000ms = 2 seconds)
 *
 * 6. Test Connection
 *    ├─ Upload this code to Arduino
 *    ├─ Open Serial Monitor (9600 baud)
 *    ├─ Check logs for WiFi/MQTT connection
 *    ├─ On Raspberry Pi, run: mosquitto_sub -h localhost -t "beehive/#" -v
 *    └─ Should see messages arriving
 *
 * 7. Access Dashboard
 *    ├─ On your computer, open: http://raspberrypi.local:5000
 *    ├─ Or: http://192.168.1.100:5000
 *    ├─ Select your hive from dropdown
 *    └─ View real-time data and charts
 */

#endif // CONFIG_WIFI_H
