/**
 * ArduiBeeScale - DollaTek/Heltec WiFi LoRa 32 Edition
 *
 * Beehive monitoring system with built-in OLED display
 * Connects to Home Assistant via WiFi + MQTT
 *
 * Hardware: DollaTek WiFi LoRa 32 868MHz / Heltec WiFi LoRa 32 V2
 * - ESP32 with WiFi (LoRa not used in this version)
 * - Built-in SSD1306 OLED 0.96" (128x64)
 * - Built-in battery management
 * - HX711 + 50kg Load Cell
 * - DHT22 temperature/humidity sensor
 *
 * Version: 4.1-LoRa32 (2025-11)
 * License: GNU GPLv3
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "HX711.h"
#include "DHT.h"
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <Wire.h>

// OLED Display (SSD1306)
#ifdef OLED_ENABLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

// Include configuration
#include "config.h"

//============================================
// HARDWARE PIN DEFINITIONS - LoRa32 Board
//============================================

// HX711 Load Cell Amplifier
#define HX711_DOUT_PIN       13    // GPIO13 - HX711 Data
#define HX711_SCK_PIN        12    // GPIO12 - HX711 Clock

// DHT22 Temperature & Humidity Sensor
#define DHT_PIN              17    // GPIO17 - DHT22 Data
#define DHT_TYPE             DHT22

// Battery Voltage (built-in on LoRa32)
#define BATTERY_PIN          37    // GPIO37 - Built-in battery ADC
#define BATTERY_SAMPLES      10    // Number of ADC samples

// Built-in LED
#define LED_PIN              25    // GPIO25 - Built-in LED

// OLED Display (built-in)
#ifdef OLED_ENABLED
#define OLED_SDA_PIN         4     // GPIO4 - OLED I2C SDA (fixed)
#define OLED_SCL_PIN         15    // GPIO15 - OLED I2C SCL (fixed)
#define OLED_RST_PIN         16    // GPIO16 - OLED Reset (fixed)
#define VEXT_PIN             21    // GPIO21 - Vext power control
#endif

// Button for display (uses PRG button)
#define BUTTON_PIN           0     // GPIO0 - PRG button (active LOW)
#define BUTTON_DEBOUNCE_MS   50

//============================================
// MQTT TOPICS
//============================================

#define MQTT_STATE_TOPIC     "beehive/" HIVE_ID "/state"
#define MQTT_AVAILABILITY    "beehive/" HIVE_ID "/availability"

//============================================
// SENSOR VALIDATION RANGES
//============================================

#define MIN_WEIGHT_KG        -10.0
#define MAX_WEIGHT_KG        200.0
#define MIN_TEMP_C           -40.0
#define MAX_TEMP_C           85.0
#define MIN_HUMIDITY         0.0
#define MAX_HUMIDITY         100.0
#define MIN_BATTERY_V        2.5
#define MAX_BATTERY_V        4.3

//============================================
// TIMING CONSTANTS
//============================================

#define WIFI_CONNECT_TIMEOUT_MS   15000
#define MQTT_CONNECT_TIMEOUT_MS   10000
#define SENSOR_STABILIZE_MS       2000
#define WIFI_RETRY_DELAY_MS       500

//============================================
// DEEP SLEEP CONFIGURATION
//============================================

#define uS_TO_S_FACTOR       1000000ULL
#define SLEEP_DURATION_uS    (SLEEP_INTERVAL_HOURS * 3600ULL * uS_TO_S_FACTOR)

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int failedTransmissions = 0;

//============================================
// GLOBAL OBJECTS
//============================================

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
HX711 scale;
DHT dht(DHT_PIN, DHT_TYPE);

// OLED Display object
#ifdef OLED_ENABLED
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RST_PIN);
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
// VEXT POWER CONTROL (LoRa32 specific)
//============================================

#ifdef OLED_ENABLED
/**
 * Control Vext power rail (powers OLED and external sensors)
 * LOW = ON, HIGH = OFF (active low on LoRa32)
 */
void setVextPower(bool on) {
    pinMode(VEXT_PIN, OUTPUT);
    digitalWrite(VEXT_PIN, on ? LOW : HIGH);
    if (on) delay(50);  // Allow power to stabilize
}
#endif

//============================================
// WIFI FUNCTIONS
//============================================

bool connectWiFi() {
    LOG_INFO("Connecting to WiFi...");
    LOG_INFO_F("SSID: %s\n", WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

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

void disconnectWiFi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();
    LOG_DEBUG("WiFi disconnected");
}

//============================================
// MQTT FUNCTIONS
//============================================

bool connectMQTT() {
    LOG_INFO("Connecting to MQTT broker...");
    LOG_INFO_F("Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setBufferSize(1024);

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
            mqttClient.publish(MQTT_AVAILABILITY, "online", true);
            return true;
        }

        LOG_ERROR_F("MQTT connection failed, rc=%d\n", mqttClient.state());
        delay(500);
    }

    return mqttClient.connected();
}

void publishHADiscovery() {
    LOG_INFO("Publishing Home Assistant discovery...");

    StaticJsonDocument<512> doc;
    char buffer[512];
    char topic[128];

    String deviceId = String(HIVE_ID);

    // Device information
    JsonObject device = doc.createNestedObject("device");
    device["identifiers"][0] = "beehive_" + deviceId;
    device["name"] = String(HIVE_NAME);
    device["model"] = "ArduiBeeScale LoRa32";
    device["manufacturer"] = "DIY";
    device["sw_version"] = "4.1-LoRa32";

    // === Weight Sensor ===
    doc.clear();
    device = doc.createNestedObject("device");
    device["identifiers"][0] = "beehive_" + deviceId;
    device["name"] = String(HIVE_NAME);
    device["model"] = "ArduiBeeScale LoRa32";
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
    device["model"] = "ArduiBeeScale LoRa32";
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
    device["model"] = "ArduiBeeScale LoRa32";
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

    // === Battery Sensor ===
    doc.clear();
    device = doc.createNestedObject("device");
    device["identifiers"][0] = "beehive_" + deviceId;
    device["name"] = String(HIVE_NAME);
    device["model"] = "ArduiBeeScale LoRa32";
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
    device["model"] = "ArduiBeeScale LoRa32";
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
    delay(100);
}

bool publishSensorData(SensorData &data) {
    LOG_INFO("Publishing sensor data...");

    StaticJsonDocument<256> doc;

    doc["weight"] = round(data.weight * 100) / 100.0;
    doc["temperature"] = round(data.temperature * 10) / 10.0;
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

float validateValue(float value, float minVal, float maxVal, float defaultVal) {
    if (isnan(value) || isinf(value) || value < minVal || value > maxVal) {
        return defaultVal;
    }
    return value;
}

float readBatteryVoltage() {
    LOG_DEBUG("Reading battery voltage...");

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    long sum = 0;
    for (int i = 0; i < BATTERY_SAMPLES; i++) {
        sum += analogRead(BATTERY_PIN);
        delay(10);
    }
    float avgReading = sum / (float)BATTERY_SAMPLES;

    // LoRa32 has built-in voltage divider (220K/100K ratio ~3.2)
    float voltage = (avgReading / 4095.0) * 3.3 * BATTERY_DIVIDER_RATIO;
    voltage += BATTERY_CALIBRATION_OFFSET;

    LOG_DEBUG_F("ADC: %.0f, Voltage: %.2fV\n", avgReading, voltage);

    return validateValue(voltage, MIN_BATTERY_V, MAX_BATTERY_V, 3.7);
}

int batteryVoltageToPercent(float voltage) {
    if (voltage >= 4.2) return 100;
    if (voltage <= 3.0) return 0;

    if (voltage >= 4.1) return 90 + (voltage - 4.1) * 100;
    if (voltage >= 4.0) return 80 + (voltage - 4.0) * 100;
    if (voltage >= 3.9) return 60 + (voltage - 3.9) * 200;
    if (voltage >= 3.8) return 40 + (voltage - 3.8) * 200;
    if (voltage >= 3.7) return 20 + (voltage - 3.7) * 200;
    if (voltage >= 3.5) return 5 + (voltage - 3.5) * 75;

    return (int)((voltage - 3.0) / 1.2 * 5);
}

float readWeight() {
    LOG_DEBUG("Reading weight...");

    if (!scale.is_ready()) {
        LOG_ERROR("HX711 not ready!");
        return 0.0;
    }

    float weight = scale.get_units(SCALE_SAMPLES);
    LOG_DEBUG_F("Raw weight: %.2f kg\n", weight);

    return validateValue(weight, MIN_WEIGHT_KG, MAX_WEIGHT_KG, 0.0);
}

void readDHT(float &temperature, float &humidity) {
    LOG_DEBUG("Reading DHT22...");

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    temperature = validateValue(temperature, MIN_TEMP_C, MAX_TEMP_C, 0.0);
    humidity = validateValue(humidity, MIN_HUMIDITY, MAX_HUMIDITY, 0.0);

    LOG_DEBUG_F("Temperature: %.1f C, Humidity: %.1f%%\n", temperature, humidity);
}

SensorData readAllSensors() {
    LOG_INFO("Reading all sensors...");

    SensorData data;
    data.valid = true;

    data.weight = readWeight();
    if (data.weight == 0.0 && scale.is_ready()) {
        delay(100);
        data.weight = readWeight();
    }

    readDHT(data.temperature, data.humidity);

    data.batteryVoltage = readBatteryVoltage();
    data.batteryPercent = batteryVoltageToPercent(data.batteryVoltage);

    data.rssi = WiFi.RSSI();

    if (data.temperature == 0.0 && data.humidity == 0.0) {
        LOG_ERROR("DHT22 read failed!");
        data.valid = false;
    }

    LOG_INFO_F("Sensors: Weight=%.2fkg, Temp=%.1fC, Hum=%.1f%%, Batt=%.2fV (%d%%)\n",
               data.weight, data.temperature, data.humidity,
               data.batteryVoltage, data.batteryPercent);

    return data;
}

//============================================
// OLED DISPLAY FUNCTIONS
//============================================

#ifdef OLED_ENABLED

void IRAM_ATTR buttonISR() {
    unsigned long now = millis();
    if (now - lastButtonPress > BUTTON_DEBOUNCE_MS) {
        buttonPressed = true;
        lastButtonPress = now;
    }
}

void initOLED() {
    LOG_INFO("Initializing OLED display...");

    // Enable Vext power for OLED
    setVextPower(true);

    // Initialize I2C with LoRa32 specific pins
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
        LOG_ERROR("SSD1306 OLED initialization failed!");
        return;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();

    // Initialize button with internal pull-up
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

    LOG_INFO("OLED and button initialized");
}

void oledShowSplash() {
    display.clearDisplay();
    display.setTextSize(1);

    // Title
    display.setCursor(10, 5);
    display.println("ArduiBeeScale");

    // Bee icon (simple)
    display.setCursor(50, 20);
    display.setTextSize(2);
    display.println("*");

    // Hive name
    display.setTextSize(1);
    display.setCursor(20, 45);
    display.println(HIVE_NAME);

    // Version
    display.setCursor(30, 56);
    display.println("v4.1-LoRa32");

    display.display();
}

void oledShowValue(const char* title, float value, const char* unit, int decimals) {
    display.clearDisplay();

    // Title at top
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(title);

    // Horizontal line
    display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

    // Large value in center
    display.setTextSize(3);
    char valueStr[16];
    if (decimals == 0) {
        snprintf(valueStr, sizeof(valueStr), "%d", (int)value);
    } else if (decimals == 1) {
        snprintf(valueStr, sizeof(valueStr), "%.1f", value);
    } else {
        snprintf(valueStr, sizeof(valueStr), "%.2f", value);
    }

    // Center the value
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, 22);
    display.println(valueStr);

    // Unit at bottom
    display.setTextSize(2);
    display.getTextBounds(unit, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, 50);
    display.println(unit);

    display.display();
}

void oledShowStatus(const char* line1, const char* line2) {
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(0, 20);
    display.println(line1);

    display.setCursor(0, 40);
    display.println(line2);

    display.display();
}

void displaySensorValues(SensorData &data) {
    LOG_INFO("Button pressed - displaying sensor values on OLED");

    // Enable display power
    setVextPower(true);
    delay(50);

    // Show splash
    oledShowSplash();
    delay(1500);

    // 1. Temperature
    oledShowValue("TEMPERATURE", data.temperature, "C", 1);
    delay(OLED_DISPLAY_TIME_MS);

    // 2. Humidity
    oledShowValue("HUMIDITY", data.humidity, "%", 1);
    delay(OLED_DISPLAY_TIME_MS);

    // 3. Weight
    oledShowValue("WEIGHT", data.weight, "kg", 2);
    delay(OLED_DISPLAY_TIME_MS);

    // 4. Battery
    oledShowValue("BATTERY", (float)data.batteryPercent, "%", 0);
    delay(OLED_DISPLAY_TIME_MS);

    // 5. Battery voltage
    oledShowValue("BATTERY", data.batteryVoltage, "V", 2);
    delay(OLED_DISPLAY_TIME_MS);

    // 6. WiFi signal
    oledShowValue("WIFI SIGNAL", (float)data.rssi, "dBm", 0);
    delay(OLED_DISPLAY_TIME_MS);

    // Clear and power off
    display.clearDisplay();
    display.display();
    setVextPower(false);

    LOG_INFO("OLED display cycle complete");
}

void oledShowConnecting() {
    setVextPower(true);
    delay(50);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 25);
    display.println("Connecting...");
    display.setCursor(30, 40);
    display.println(WIFI_SSID);
    display.display();
}

void oledShowSuccess() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20, 20);
    display.println("DATA");
    display.setCursor(30, 40);
    display.println("SENT");
    display.display();
    delay(1000);

    display.clearDisplay();
    display.display();
    setVextPower(false);
}

void oledShowError(const char* error) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("ERROR:");
    display.setCursor(0, 35);
    display.println(error);
    display.display();
    delay(2000);

    display.clearDisplay();
    display.display();
    setVextPower(false);
}

void checkButtonAndDisplay(SensorData &data) {
    if (buttonPressed) {
        buttonPressed = false;
        displaySensorValues(data);
    }
}

#endif // OLED_ENABLED

//============================================
// DEEP SLEEP FUNCTIONS
//============================================

void enterDeepSleep() {
    LOG_INFO_F("Entering deep sleep for %d hours...\n", SLEEP_INTERVAL_HOURS);

    disconnectWiFi();
    scale.power_down();

    #ifdef OLED_ENABLED
    setVextPower(false);  // Turn off OLED power
    #endif

    esp_sleep_enable_timer_wakeup(SLEEP_DURATION_uS);

    #ifdef OLED_ENABLED
    // Enable wake on button press
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 0);
    LOG_INFO("Button wake-up enabled");
    #endif

    #if DEBUG_ENABLED
    Serial.flush();
    #endif

    esp_deep_sleep_start();
}

//============================================
// INITIALIZATION
//============================================

void initScale() {
    LOG_INFO("Initializing HX711 scale...");

    scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);

    int retries = 10;
    while (!scale.is_ready() && retries > 0) {
        delay(100);
        retries--;
    }

    if (!scale.is_ready()) {
        LOG_ERROR("HX711 not responding!");
        return;
    }

    scale.set_scale(SCALE_CALIBRATION);
    scale.set_offset(SCALE_OFFSET);

    LOG_INFO_F("Scale initialized (cal: %.2f, offset: %ld)\n",
               SCALE_CALIBRATION, SCALE_OFFSET);
}

void blinkLED(int times, int duration) {
    pinMode(LED_PIN, OUTPUT);
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(duration);
        digitalWrite(LED_PIN, LOW);
        if (i < times - 1) delay(duration);
    }
}

//============================================
// MAIN SETUP
//============================================

void setup() {
    bootCount++;

    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

    #if DEBUG_ENABLED
    Serial.begin(115200);
    delay(100);
    Serial.println();
    Serial.println("================================================");
    Serial.println("   ArduiBeeScale LoRa32 Edition v4.1");
    Serial.println("   DollaTek/Heltec WiFi LoRa 32");
    Serial.println("================================================");
    Serial.printf("Boot count: %d\n", bootCount);
    Serial.printf("Hive ID: %s\n", HIVE_ID);
    Serial.printf("Hive Name: %s\n", HIVE_NAME);

    switch(wakeupReason) {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Wake-up: Timer (scheduled measurement)");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Wake-up: Button press (OLED display)");
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

    // Initialize OLED display
    #ifdef OLED_ENABLED
    initOLED();
    #endif

    // Allow sensors to stabilize
    delay(SENSOR_STABILIZE_MS);

    // Read all sensors
    SensorData sensorData = readAllSensors();

    // If woken by button press, only display OLED values (no WiFi/MQTT)
    #ifdef OLED_ENABLED
    if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
        LOG_INFO("Button wake-up - displaying values on OLED only");
        displaySensorValues(sensorData);
        blinkLED(1, 200);
        enterDeepSleep();
        return;
    }
    #endif

    blinkLED(2, 100);

    // Show connecting status on OLED
    #ifdef OLED_ENABLED
    oledShowConnecting();
    #endif

    // Connect to WiFi
    if (!connectWiFi()) {
        LOG_ERROR("WiFi failed! Going to sleep...");
        failedTransmissions++;
        #ifdef OLED_ENABLED
        oledShowError("WiFi failed!");
        #endif
        blinkLED(5, 50);
        enterDeepSleep();
        return;
    }

    // Connect to MQTT
    if (!connectMQTT()) {
        LOG_ERROR("MQTT failed! Going to sleep...");
        failedTransmissions++;
        #ifdef OLED_ENABLED
        oledShowError("MQTT failed!");
        #endif
        blinkLED(5, 50);
        enterDeepSleep();
        return;
    }

    // Publish Home Assistant discovery
    if (bootCount == 1 || bootCount % 12 == 0) {
        publishHADiscovery();
    }

    // Publish sensor data
    if (sensorData.valid) {
        if (publishSensorData(sensorData)) {
            failedTransmissions = 0;
            #ifdef OLED_ENABLED
            oledShowSuccess();
            #endif
            blinkLED(1, 500);
        } else {
            failedTransmissions++;
            #ifdef OLED_ENABLED
            oledShowError("Publish failed!");
            #endif
            blinkLED(3, 100);
        }
    } else {
        LOG_ERROR("Invalid sensor data");
        #ifdef OLED_ENABLED
        oledShowError("Sensor error!");
        #endif
        blinkLED(4, 100);
    }

    mqttClient.loop();
    delay(100);

    mqttClient.publish(MQTT_AVAILABILITY, "offline", true);
    mqttClient.disconnect();

    #ifdef OLED_ENABLED
    checkButtonAndDisplay(sensorData);
    #endif

    enterDeepSleep();
}

//============================================
// MAIN LOOP (not used - deep sleep)
//============================================

void loop() {
    LOG_ERROR("Unexpected loop() execution!");
    enterDeepSleep();
}
