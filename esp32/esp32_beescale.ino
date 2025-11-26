/**
 * ArduiBeeScale ESP32 Edition - Beehive Monitoring with Home Assistant
 *
 * ESP32-WROOM-32U based IoT system for monitoring beehive weight and environmental
 * conditions with native WiFi, deep sleep power management, and Home Assistant
 * integration via MQTT.
 *
 * Project: ArduiBeeScale ESP32 Edition
 * Original Author: Hannes Oberreiter
 * Maintained by: Jeremy JEANNE
 *
 * Version: 4.1 (2025-11)
 * - ESP32-WROOM-32U with external antenna support
 * - Native WiFi (no ESP-01 module needed)
 * - True deep sleep (10-20µA consumption)
 * - Home Assistant auto-discovery via MQTT
 * - Battery voltage monitoring with ADC calibration
 * - LCD 1602 I2C display with button trigger
 * - OTA update support (optional)
 * - Improved sensor accuracy
 *
 * Hardware:
 * - ESP32-WROOM-32U (with U.FL connector for external antenna)
 * - 3dBi External WiFi Antenna
 * - HX711 + 50kg Load Cell (weight)
 * - DHT22 / AM2302 (temperature & humidity)
 * - 18650 Li-Ion Battery + TP4056 charger
 * - Optional: LCD 1602 I2C display + push button
 * - Optional: Solar panel for charging
 *
 * License: GNU GPLv3
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "HX711.h"
#include "DHT.h"
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <Wire.h>

// LCD Display (optional)
#ifdef LCD_ENABLED
#include <LiquidCrystal_I2C.h>
#endif

// Include configuration
#include "config.h"

//============================================
// HARDWARE PIN DEFINITIONS (ESP32-WROOM-32U)
//============================================

// HX711 Load Cell Amplifier
#define HX711_DOUT_PIN       16    // GPIO16 - HX711 Data
#define HX711_SCK_PIN        17    // GPIO17 - HX711 Clock

// DHT22 Temperature & Humidity Sensor
#define DHT_PIN              4     // GPIO4 - DHT22 Data
#define DHT_TYPE             DHT22

// Battery Voltage Measurement
#define BATTERY_PIN          34    // GPIO34 (ADC1_CH6) - Battery voltage divider
#define BATTERY_SAMPLES      10    // Number of ADC samples for averaging

// Status LED (optional - use built-in or external)
#define LED_PIN              2     // GPIO2 - Built-in LED (most ESP32 boards)

// LCD 1602 I2C Display (optional)
#ifdef LCD_ENABLED
#define LCD_SDA_PIN          21    // GPIO21 - I2C SDA (default ESP32)
#define LCD_SCL_PIN          22    // GPIO22 - I2C SCL (default ESP32)
#define LCD_COLS             16    // LCD columns
#define LCD_ROWS             2     // LCD rows
#endif

// Push Button for LCD Display
#ifdef LCD_ENABLED
#define BUTTON_PIN           13    // GPIO13 - Push button (active LOW with pull-up)
#define BUTTON_DEBOUNCE_MS   50    // Debounce time in milliseconds
#endif

//============================================
// MQTT TOPICS FOR HOME ASSISTANT
//============================================

// Base topic for all beehive data
#define MQTT_BASE_TOPIC      "homeassistant"
#define MQTT_STATE_TOPIC     "beehive/" HIVE_ID "/state"
#define MQTT_AVAILABILITY    "beehive/" HIVE_ID "/availability"

// Home Assistant discovery topics
#define HA_DISCOVERY_PREFIX  "homeassistant"

//============================================
// SENSOR VALIDATION RANGES
//============================================

#define MIN_WEIGHT_KG        -10.0   // Allow negative for tare calibration
#define MAX_WEIGHT_KG        200.0   // Maximum expected hive weight
#define MIN_TEMP_C           -40.0
#define MAX_TEMP_C           85.0
#define MIN_HUMIDITY         0.0
#define MAX_HUMIDITY         100.0
#define MIN_BATTERY_V        2.5     // Li-Ion minimum
#define MAX_BATTERY_V        4.3     // Li-Ion maximum

//============================================
// TIMING CONSTANTS
//============================================

#define WIFI_CONNECT_TIMEOUT_MS   15000   // 15 seconds to connect to WiFi
#define MQTT_CONNECT_TIMEOUT_MS   10000   // 10 seconds to connect to MQTT
#define SENSOR_STABILIZE_MS       2000    // Time for sensors to stabilize
#define WIFI_RETRY_DELAY_MS       500     // Delay between WiFi retries

//============================================
// DEEP SLEEP CONFIGURATION
//============================================

// Convert hours to microseconds for deep sleep
#define uS_TO_S_FACTOR       1000000ULL
#define SLEEP_DURATION_uS    (SLEEP_INTERVAL_HOURS * 3600ULL * uS_TO_S_FACTOR)

// RTC memory to persist data across deep sleep
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int failedTransmissions = 0;

//============================================
// GLOBAL OBJECTS
//============================================

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
HX711 scale;
DHT dht(DHT_PIN, DHT_TYPE);
Preferences preferences;

// LCD Display object (optional)
#ifdef LCD_ENABLED
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);
volatile bool buttonPressed = false;
unsigned long lastButtonPress = 0;
#endif

//============================================
// SENSOR DATA STRUCTURE
//============================================

struct SensorData {
    float weight;
    float temperature;
    float humidity;
    float batteryVoltage;
    int batteryPercent;
    int rssi;
    bool valid;
};

//============================================
// DEBUG LOGGING
//============================================

#if DEBUG_ENABLED
    #define LOG_INFO(msg)        Serial.println("[INFO] " msg)
    #define LOG_INFO_F(...)      Serial.printf("[INFO] " __VA_ARGS__)
    #define LOG_ERROR(msg)       Serial.println("[ERROR] " msg)
    #define LOG_ERROR_F(...)     Serial.printf("[ERROR] " __VA_ARGS__)
    #define LOG_DEBUG(msg)       Serial.println("[DEBUG] " msg)
    #define LOG_DEBUG_F(...)     Serial.printf("[DEBUG] " __VA_ARGS__)
#else
    #define LOG_INFO(msg)
    #define LOG_INFO_F(...)
    #define LOG_ERROR(msg)
    #define LOG_ERROR_F(...)
    #define LOG_DEBUG(msg)
    #define LOG_DEBUG_F(...)
#endif

//============================================
// WIFI FUNCTIONS
//============================================

/**
 * Connect to WiFi network with timeout
 */
bool connectWiFi() {
    LOG_INFO("Connecting to WiFi...");
    LOG_INFO_F("SSID: %s\n", WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Set WiFi power for external antenna (can increase for better range)
    WiFi.setTxPower(WIFI_POWER_19_5dBm);  // Maximum power for external antenna

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT_MS) {
            LOG_ERROR("WiFi connection timeout!");
            return false;
        }
        delay(WIFI_RETRY_DELAY_MS);
        #if DEBUG_ENABLED
        Serial.print(".");
        #endif
    }

    #if DEBUG_ENABLED
    Serial.println();
    #endif

    LOG_INFO_F("WiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
    LOG_INFO_F("Signal strength (RSSI): %d dBm\n", WiFi.RSSI());

    return true;
}

/**
 * Disconnect WiFi and prepare for deep sleep
 */
void disconnectWiFi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();
    LOG_DEBUG("WiFi disconnected");
}

//============================================
// MQTT FUNCTIONS
//============================================

/**
 * Connect to MQTT broker
 */
bool connectMQTT() {
    LOG_INFO("Connecting to MQTT broker...");
    LOG_INFO_F("Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setBufferSize(1024);  // Larger buffer for HA discovery

    unsigned long startTime = millis();

    while (!mqttClient.connected()) {
        if (millis() - startTime > MQTT_CONNECT_TIMEOUT_MS) {
            LOG_ERROR("MQTT connection timeout!");
            return false;
        }

        String clientId = "beehive-" + String(HIVE_ID);

        bool connected;
        #ifdef MQTT_USER
            connected = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD,
                                           MQTT_AVAILABILITY, 0, true, "offline");
        #else
            connected = mqttClient.connect(clientId.c_str(),
                                           MQTT_AVAILABILITY, 0, true, "offline");
        #endif

        if (connected) {
            LOG_INFO("MQTT connected!");
            // Publish availability
            mqttClient.publish(MQTT_AVAILABILITY, "online", true);
            return true;
        }

        LOG_ERROR_F("MQTT connection failed, rc=%d\n", mqttClient.state());
        delay(500);
    }

    return mqttClient.connected();
}

/**
 * Publish Home Assistant MQTT Discovery configuration
 * This allows automatic entity creation in Home Assistant
 */
void publishHADiscovery() {
    LOG_INFO("Publishing Home Assistant discovery...");

    StaticJsonDocument<512> doc;
    char buffer[512];
    char topic[128];

    String deviceId = String(HIVE_ID);

    // Device information (shared by all sensors)
    JsonObject device = doc.createNestedObject("device");
    device["identifiers"][0] = "beehive_" + deviceId;
    device["name"] = String(HIVE_NAME);
    device["model"] = "ArduiBeeScale ESP32";
    device["manufacturer"] = "DIY";
    device["sw_version"] = "4.0";

    // === Weight Sensor ===
    doc.clear();
    device = doc.createNestedObject("device");
    device["identifiers"][0] = "beehive_" + deviceId;
    device["name"] = String(HIVE_NAME);
    device["model"] = "ArduiBeeScale ESP32";
    device["manufacturer"] = "DIY";

    doc["name"] = String(HIVE_NAME) + " Weight";
    doc["unique_id"] = "beehive_" + deviceId + "_weight";
    doc["state_topic"] = MQTT_STATE_TOPIC;
    doc["availability_topic"] = MQTT_AVAILABILITY;
    doc["value_template"] = "{{ value_json.weight }}";
    doc["unit_of_measurement"] = "kg";
    doc["device_class"] = "weight";
    doc["state_class"] = "measurement";
    doc["icon"] = "mdi:scale";

    snprintf(topic, sizeof(topic), "%s/sensor/beehive_%s_weight/config",
             HA_DISCOVERY_PREFIX, HIVE_ID);
    serializeJson(doc, buffer);
    mqttClient.publish(topic, buffer, true);

    // === Temperature Sensor ===
    doc.clear();
    device = doc.createNestedObject("device");
    device["identifiers"][0] = "beehive_" + deviceId;
    device["name"] = String(HIVE_NAME);
    device["model"] = "ArduiBeeScale ESP32";
    device["manufacturer"] = "DIY";

    doc["name"] = String(HIVE_NAME) + " Temperature";
    doc["unique_id"] = "beehive_" + deviceId + "_temperature";
    doc["state_topic"] = MQTT_STATE_TOPIC;
    doc["availability_topic"] = MQTT_AVAILABILITY;
    doc["value_template"] = "{{ value_json.temperature }}";
    doc["unit_of_measurement"] = "°C";
    doc["device_class"] = "temperature";
    doc["state_class"] = "measurement";

    snprintf(topic, sizeof(topic), "%s/sensor/beehive_%s_temperature/config",
             HA_DISCOVERY_PREFIX, HIVE_ID);
    serializeJson(doc, buffer);
    mqttClient.publish(topic, buffer, true);

    // === Humidity Sensor ===
    doc.clear();
    device = doc.createNestedObject("device");
    device["identifiers"][0] = "beehive_" + deviceId;
    device["name"] = String(HIVE_NAME);
    device["model"] = "ArduiBeeScale ESP32";
    device["manufacturer"] = "DIY";

    doc["name"] = String(HIVE_NAME) + " Humidity";
    doc["unique_id"] = "beehive_" + deviceId + "_humidity";
    doc["state_topic"] = MQTT_STATE_TOPIC;
    doc["availability_topic"] = MQTT_AVAILABILITY;
    doc["value_template"] = "{{ value_json.humidity }}";
    doc["unit_of_measurement"] = "%";
    doc["device_class"] = "humidity";
    doc["state_class"] = "measurement";

    snprintf(topic, sizeof(topic), "%s/sensor/beehive_%s_humidity/config",
             HA_DISCOVERY_PREFIX, HIVE_ID);
    serializeJson(doc, buffer);
    mqttClient.publish(topic, buffer, true);

    // === Battery Voltage Sensor ===
    doc.clear();
    device = doc.createNestedObject("device");
    device["identifiers"][0] = "beehive_" + deviceId;
    device["name"] = String(HIVE_NAME);
    device["model"] = "ArduiBeeScale ESP32";
    device["manufacturer"] = "DIY";

    doc["name"] = String(HIVE_NAME) + " Battery";
    doc["unique_id"] = "beehive_" + deviceId + "_battery";
    doc["state_topic"] = MQTT_STATE_TOPIC;
    doc["availability_topic"] = MQTT_AVAILABILITY;
    doc["value_template"] = "{{ value_json.battery_percent }}";
    doc["unit_of_measurement"] = "%";
    doc["device_class"] = "battery";
    doc["state_class"] = "measurement";

    snprintf(topic, sizeof(topic), "%s/sensor/beehive_%s_battery/config",
             HA_DISCOVERY_PREFIX, HIVE_ID);
    serializeJson(doc, buffer);
    mqttClient.publish(topic, buffer, true);

    // === WiFi Signal Sensor ===
    doc.clear();
    device = doc.createNestedObject("device");
    device["identifiers"][0] = "beehive_" + deviceId;
    device["name"] = String(HIVE_NAME);
    device["model"] = "ArduiBeeScale ESP32";
    device["manufacturer"] = "DIY";

    doc["name"] = String(HIVE_NAME) + " WiFi Signal";
    doc["unique_id"] = "beehive_" + deviceId + "_rssi";
    doc["state_topic"] = MQTT_STATE_TOPIC;
    doc["availability_topic"] = MQTT_AVAILABILITY;
    doc["value_template"] = "{{ value_json.rssi }}";
    doc["unit_of_measurement"] = "dBm";
    doc["device_class"] = "signal_strength";
    doc["state_class"] = "measurement";
    doc["entity_category"] = "diagnostic";

    snprintf(topic, sizeof(topic), "%s/sensor/beehive_%s_rssi/config",
             HA_DISCOVERY_PREFIX, HIVE_ID);
    serializeJson(doc, buffer);
    mqttClient.publish(topic, buffer, true);

    LOG_INFO("Home Assistant discovery published!");
    delay(100);  // Allow MQTT to process
}

/**
 * Publish sensor data to MQTT
 */
bool publishSensorData(SensorData &data) {
    LOG_INFO("Publishing sensor data...");

    StaticJsonDocument<256> doc;

    doc["weight"] = round(data.weight * 100) / 100.0;  // 2 decimal places
    doc["temperature"] = round(data.temperature * 10) / 10.0;  // 1 decimal place
    doc["humidity"] = round(data.humidity * 10) / 10.0;
    doc["battery_voltage"] = round(data.batteryVoltage * 100) / 100.0;
    doc["battery_percent"] = data.batteryPercent;
    doc["rssi"] = data.rssi;
    doc["boot_count"] = bootCount;
    doc["hive_id"] = HIVE_ID;

    char buffer[256];
    serializeJson(doc, buffer);

    LOG_DEBUG_F("Payload: %s\n", buffer);

    bool success = mqttClient.publish(MQTT_STATE_TOPIC, buffer, true);

    if (success) {
        LOG_INFO("Data published successfully!");
    } else {
        LOG_ERROR("Failed to publish data!");
    }

    return success;
}

//============================================
// SENSOR READING FUNCTIONS
//============================================

/**
 * Validate sensor value within acceptable range
 */
float validateValue(float value, float minVal, float maxVal, float defaultVal) {
    if (isnan(value) || isinf(value) || value < minVal || value > maxVal) {
        LOG_ERROR_F("Invalid value: %.2f (range: %.2f - %.2f)\n", value, minVal, maxVal);
        return defaultVal;
    }
    return value;
}

/**
 * Read battery voltage using ADC with averaging
 * Uses voltage divider: Battery -> 100K -> ADC -> 100K -> GND
 * This gives 1:2 ratio, so multiply by 2
 */
float readBatteryVoltage() {
    LOG_DEBUG("Reading battery voltage...");

    // Configure ADC
    analogReadResolution(12);  // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db);  // Full range 0-3.3V

    // Average multiple samples for stability
    long sum = 0;
    for (int i = 0; i < BATTERY_SAMPLES; i++) {
        sum += analogRead(BATTERY_PIN);
        delay(10);
    }
    float avgReading = sum / (float)BATTERY_SAMPLES;

    // Convert to voltage
    // ADC reads 0-3.3V over 0-4095
    // Voltage divider ratio is 2:1 (100K + 100K)
    float voltage = (avgReading / 4095.0) * 3.3 * BATTERY_DIVIDER_RATIO;

    // Apply calibration offset if needed
    voltage += BATTERY_CALIBRATION_OFFSET;

    LOG_DEBUG_F("ADC: %.0f, Voltage: %.2fV\n", avgReading, voltage);

    return validateValue(voltage, MIN_BATTERY_V, MAX_BATTERY_V, 3.7);
}

/**
 * Convert battery voltage to percentage (Li-Ion curve approximation)
 * 4.2V = 100%, 3.0V = 0%
 */
int batteryVoltageToPercent(float voltage) {
    if (voltage >= 4.2) return 100;
    if (voltage <= 3.0) return 0;

    // Simplified Li-Ion discharge curve
    // More accurate would be a lookup table
    if (voltage >= 4.1) return 90 + (voltage - 4.1) * 100;
    if (voltage >= 4.0) return 80 + (voltage - 4.0) * 100;
    if (voltage >= 3.9) return 60 + (voltage - 3.9) * 200;
    if (voltage >= 3.8) return 40 + (voltage - 3.8) * 200;
    if (voltage >= 3.7) return 20 + (voltage - 3.7) * 200;
    if (voltage >= 3.5) return 5 + (voltage - 3.5) * 75;

    return (int)((voltage - 3.0) / 1.2 * 5);
}

/**
 * Read weight from HX711 load cell
 */
float readWeight() {
    LOG_DEBUG("Reading weight...");

    if (!scale.is_ready()) {
        LOG_ERROR("HX711 not ready!");
        return 0.0;
    }

    // Take multiple readings and average
    float weight = scale.get_units(SCALE_SAMPLES);

    LOG_DEBUG_F("Raw weight: %.2f kg\n", weight);

    return validateValue(weight, MIN_WEIGHT_KG, MAX_WEIGHT_KG, 0.0);
}

/**
 * Read temperature and humidity from DHT22
 */
void readDHT(float &temperature, float &humidity) {
    LOG_DEBUG("Reading DHT22...");

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    temperature = validateValue(temperature, MIN_TEMP_C, MAX_TEMP_C, 0.0);
    humidity = validateValue(humidity, MIN_HUMIDITY, MAX_HUMIDITY, 0.0);

    LOG_DEBUG_F("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
}

/**
 * Read all sensors and populate data structure
 */
SensorData readAllSensors() {
    LOG_INFO("Reading all sensors...");

    SensorData data;
    data.valid = true;

    // Read weight
    data.weight = readWeight();
    if (data.weight == 0.0 && scale.is_ready()) {
        // Retry once
        delay(100);
        data.weight = readWeight();
    }

    // Read temperature and humidity
    readDHT(data.temperature, data.humidity);

    // Read battery
    data.batteryVoltage = readBatteryVoltage();
    data.batteryPercent = batteryVoltageToPercent(data.batteryVoltage);

    // WiFi signal strength
    data.rssi = WiFi.RSSI();

    // Validate we got meaningful data
    if (data.temperature == 0.0 && data.humidity == 0.0) {
        LOG_ERROR("DHT22 read failed!");
        data.valid = false;
    }

    LOG_INFO_F("Sensors: Weight=%.2fkg, Temp=%.1f°C, Hum=%.1f%%, Batt=%.2fV (%d%%)\n",
               data.weight, data.temperature, data.humidity,
               data.batteryVoltage, data.batteryPercent);

    return data;
}

//============================================
// DEEP SLEEP FUNCTIONS
//============================================

/**
 * Enter deep sleep mode for specified duration
 */
void enterDeepSleep() {
    LOG_INFO_F("Entering deep sleep for %d hours...\n", SLEEP_INTERVAL_HOURS);
    LOG_INFO("Current consumption: ~10-20µA");

    // Disconnect and power down WiFi
    disconnectWiFi();

    // Power down HX711
    scale.power_down();

    // Configure wake-up source (timer)
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION_uS);

    // Configure GPIO wake-up for button press (LCD display during sleep)
    #ifdef LCD_ENABLED
    // Enable wake-up on button press (GPIO low = button pressed)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 0);
    LOG_INFO("Button wake-up enabled (press button to display values)");
    #endif

    #if DEBUG_ENABLED
    Serial.flush();
    #endif

    // Enter deep sleep
    esp_deep_sleep_start();

    // Code never reaches here
}

//============================================
// LCD DISPLAY FUNCTIONS (Optional)
//============================================

#ifdef LCD_ENABLED

/**
 * Button interrupt handler
 * Called when button is pressed (falling edge)
 */
void IRAM_ATTR buttonISR() {
    unsigned long now = millis();
    // Debounce: ignore if pressed within debounce time
    if (now - lastButtonPress > BUTTON_DEBOUNCE_MS) {
        buttonPressed = true;
        lastButtonPress = now;
    }
}

/**
 * Initialize LCD display and button
 */
void initLCD() {
    LOG_INFO("Initializing LCD display...");

    // Initialize I2C with custom pins if needed
    Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);

    // Initialize LCD
    lcd.init();
    lcd.noBacklight();  // Start with backlight off to save power

    // Initialize button with internal pull-up
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Attach interrupt for button press (falling edge = button pressed)
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

    LOG_INFO("LCD and button initialized");
}

/**
 * Display a value on the LCD with title
 * Line 1: Title (centered)
 * Line 2: Value with unit (centered)
 */
void lcdDisplayValue(const char* title, const char* value, const char* unit) {
    lcd.clear();

    // Center title on line 1
    int titleLen = strlen(title);
    int titlePos = (LCD_COLS - titleLen) / 2;
    if (titlePos < 0) titlePos = 0;
    lcd.setCursor(titlePos, 0);
    lcd.print(title);

    // Center value + unit on line 2
    char line2[17];
    snprintf(line2, sizeof(line2), "%s %s", value, unit);
    int line2Len = strlen(line2);
    int line2Pos = (LCD_COLS - line2Len) / 2;
    if (line2Pos < 0) line2Pos = 0;
    lcd.setCursor(line2Pos, 1);
    lcd.print(line2);
}

/**
 * Display all sensor values sequentially on LCD
 * Called when button is pressed
 * Shows: Temperature -> Humidity -> Weight -> Battery
 * Each displayed for LCD_DISPLAY_TIME_MS (default 2000ms)
 */
void displaySensorValues(SensorData &data) {
    LOG_INFO("Button pressed - displaying sensor values on LCD");

    char valueStr[16];

    // Turn on backlight
    lcd.backlight();

    // Show welcome message briefly
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("ArduiBeeScale");
    lcd.setCursor(4, 1);
    lcd.print(HIVE_NAME);
    delay(1500);

    // 1. Display Temperature
    snprintf(valueStr, sizeof(valueStr), "%.1f", data.temperature);
    lcdDisplayValue("TEMPERATURE", valueStr, "\xDF" "C");  // \xDF is degree symbol
    delay(LCD_DISPLAY_TIME_MS);

    // 2. Display Humidity
    snprintf(valueStr, sizeof(valueStr), "%.1f", data.humidity);
    lcdDisplayValue("HUMIDITY", valueStr, "%");
    delay(LCD_DISPLAY_TIME_MS);

    // 3. Display Weight
    snprintf(valueStr, sizeof(valueStr), "%.2f", data.weight);
    lcdDisplayValue("WEIGHT", valueStr, "kg");
    delay(LCD_DISPLAY_TIME_MS);

    // 4. Display Battery
    snprintf(valueStr, sizeof(valueStr), "%d", data.batteryPercent);
    lcdDisplayValue("BATTERY", valueStr, "%");

    // Also show voltage on second display
    delay(LCD_DISPLAY_TIME_MS);

    snprintf(valueStr, sizeof(valueStr), "%.2f", data.batteryVoltage);
    lcdDisplayValue("BATTERY", valueStr, "V");
    delay(LCD_DISPLAY_TIME_MS);

    // Turn off backlight
    lcd.noBacklight();
    lcd.clear();

    LOG_INFO("LCD display cycle complete");
}

/**
 * Check if button was pressed and display values
 * Call this in the main loop or before sleep
 */
void checkButtonAndDisplay(SensorData &data) {
    if (buttonPressed) {
        buttonPressed = false;  // Reset flag
        displaySensorValues(data);
    }
}

#endif // LCD_ENABLED

//============================================
// INITIALIZATION
//============================================

/**
 * Initialize HX711 scale with calibration
 */
void initScale() {
    LOG_INFO("Initializing HX711 scale...");

    scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);

    // Wait for scale to be ready
    int retries = 10;
    while (!scale.is_ready() && retries > 0) {
        delay(100);
        retries--;
    }

    if (!scale.is_ready()) {
        LOG_ERROR("HX711 not responding!");
        return;
    }

    // Apply calibration
    scale.set_scale(SCALE_CALIBRATION);
    scale.set_offset(SCALE_OFFSET);

    LOG_INFO_F("Scale initialized (cal: %.2f, offset: %ld)\n",
               SCALE_CALIBRATION, SCALE_OFFSET);
}

/**
 * Blink LED to indicate status
 */
void blinkLED(int times, int duration) {
    #ifdef LED_PIN
    pinMode(LED_PIN, OUTPUT);
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(duration);
        digitalWrite(LED_PIN, LOW);
        if (i < times - 1) delay(duration);
    }
    #endif
}

//============================================
// MAIN SETUP
//============================================

void setup() {
    // Increment boot counter
    bootCount++;

    // Check wake-up reason
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

    #if DEBUG_ENABLED
    Serial.begin(115200);
    delay(100);
    Serial.println();
    Serial.println("================================================");
    Serial.println("   ArduiBeeScale ESP32 Edition v4.1");
    Serial.println("================================================");
    Serial.printf("Boot count: %d\n", bootCount);
    Serial.printf("Hive ID: %s\n", HIVE_ID);
    Serial.printf("Hive Name: %s\n", HIVE_NAME);

    // Print wake-up reason
    switch(wakeupReason) {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Wake-up: Timer (scheduled measurement)");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Wake-up: Button press (LCD display)");
            break;
        default:
            Serial.println("Wake-up: Power on / Reset");
            break;
    }
    Serial.println("------------------------------------------------");
    #endif

    // Initialize sensors
    dht.begin();
    initScale();

    // Initialize LCD display (optional)
    #ifdef LCD_ENABLED
    initLCD();
    #endif

    // Allow sensors to stabilize
    delay(SENSOR_STABILIZE_MS);

    // Read all sensors
    SensorData sensorData = readAllSensors();

    // If woken by button press, only display LCD values (no WiFi/MQTT)
    #ifdef LCD_ENABLED
    if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
        LOG_INFO("Button wake-up detected - displaying values on LCD only");
        displaySensorValues(sensorData);
        blinkLED(1, 200);
        enterDeepSleep();
        return;  // Never reached, but for clarity
    }
    #endif

    // Indicate startup (only for normal measurement cycle)
    blinkLED(2, 100);

    // Connect to WiFi
    if (!connectWiFi()) {
        LOG_ERROR("WiFi failed! Going to sleep...");
        failedTransmissions++;
        blinkLED(5, 50);  // Fast blink = error
        enterDeepSleep();
        return;
    }

    // Connect to MQTT
    if (!connectMQTT()) {
        LOG_ERROR("MQTT failed! Going to sleep...");
        failedTransmissions++;
        blinkLED(5, 50);
        enterDeepSleep();
        return;
    }

    // Publish Home Assistant discovery (only on first boot or periodically)
    if (bootCount == 1 || bootCount % 12 == 0) {  // Every 12 boots (~24h if 2h interval)
        publishHADiscovery();
    }

    // Publish sensor data
    if (sensorData.valid) {
        if (publishSensorData(sensorData)) {
            failedTransmissions = 0;  // Reset on success
            blinkLED(1, 500);  // Long blink = success
        } else {
            failedTransmissions++;
            blinkLED(3, 100);
        }
    } else {
        LOG_ERROR("Invalid sensor data, not publishing");
        blinkLED(4, 100);
    }

    // Allow MQTT to complete
    mqttClient.loop();
    delay(100);

    // Disconnect MQTT gracefully
    mqttClient.publish(MQTT_AVAILABILITY, "offline", true);
    mqttClient.disconnect();

    // Check if button was pressed during operation (LCD display)
    #ifdef LCD_ENABLED
    checkButtonAndDisplay(sensorData);
    #endif

    // Enter deep sleep
    enterDeepSleep();
}

//============================================
// MAIN LOOP (not used - ESP32 deep sleeps)
//============================================

void loop() {
    // This is never reached because we deep sleep in setup()
    // If we get here somehow, go to sleep
    LOG_ERROR("Unexpected loop() execution!");
    enterDeepSleep();
}
