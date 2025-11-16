/**
 * ArduiBeeScale - Arduino Beehive Scale System with WiFi + MQTT + LCD Display
 *
 * An Arduino-based IoT system for monitoring beehive weight and environmental conditions
 * with ESP-01 WiFi connectivity, local MQTT publishing, and 16x2 I2C LCD display.
 *
 * Project: ArduiBeeScale (Arduino + Beehive + Scale)
 * Original Author: Hannes Oberreiter
 * Maintained by: Jeremy JEANNE
 *
 * Version: 3.1 (2025-11) - Added LCD1602 I2C Display + Push Button
 * - Complete WiFi + MQTT implementation
 * - Software Sleep power management (€0 cost)
 * - 16x2 LCD I2C Display (€2-3)
 * - Push button for on-demand status display (€0.50)
 * - Battery percentage calculation
 * - Removed GSM/SIM900 (cellular no longer used)
 * - Removed Cloud API / bTree integration (local only)
 *
 * License: GNU GPLv3
 */

#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include "DHT.h"

//============================================
// CONFIGURATION - WiFi + MQTT Settings
//============================================

// WiFi Configuration
#define WIFI_SSID            "your_ssid"           // Change to your WiFi network name
#define WIFI_PASSWORD        "your_password"       // Change to your WiFi password

// MQTT Configuration
#define MQTT_BROKER          "192.168.1.100"       // Raspberry Pi IP address
#define MQTT_PORT            1883
#define MQTT_CLIENT_ID       "beehive-001"         // Unique ID for this hive
#define MQTT_TOPIC           "beehive/hive-001"    // Topic format: beehive/hive-XXX
#define MQTT_KEEPALIVE       60                    // Seconds

// ESP-01 Serial Communication
#define ESP_RX_PIN           8                     // Arduino RX (connects to ESP-01 TX)
#define ESP_TX_PIN           9                     // Arduino TX (connects to ESP-01 RX via level shifter)
SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN); // RX, TX for ESP-01

//============================================
// LCD I2C CONFIGURATION
//============================================

// I2C address for LCD1602 (default: 0x27, some boards use 0x3F)
#define LCD_I2C_ADDRESS      0x27                  // Check your LCD's address
#define LCD_COLUMNS          16
#define LCD_ROWS             2

// Create LCD object
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

//============================================
// PUSH BUTTON & DISPLAY CONFIGURATION
//============================================

#define BUTTON_PIN           4                     // Push button input pin
#define LCD_BACKLIGHT_PIN    3                     // PWM pin for LCD backlight (optional)
#define DISPLAY_DURATION_MS  2000                  // 2 seconds per screen
#define BUTTON_DEBOUNCE_MS   50                    // 50ms debounce time
#define BATTERY_FULL_VOLTAGE 6.0                   // 4x AA fresh = 6V
#define BATTERY_EMPTY_VOLTAGE 3.0                  // 4x AA minimum safe = 3V

// Display states
enum DisplayState {
  STATE_OFF = 0,
  STATE_TEMP_HUMID = 1,
  STATE_WEIGHT = 2,
  STATE_BATTERY = 3
};

volatile bool buttonPressed = false;              // Button press flag
unsigned long lastButtonPress = 0;                // For debouncing
unsigned long displayStartTime = 0;               // Track display timing
DisplayState currentDisplayState = STATE_OFF;

//============================================
// HARDWARE PIN DEFINITIONS
//============================================
#define HX711_DOUT_PIN       5                     // HX711 DT (Data)
#define HX711_CLK_PIN        6                     // HX711 CLK (Clock)
#define DHTPIN               10                    // DHT22 data pin
#define BATTERY_PIN          A0                    // Battery voltage measurement pin

//============================================
// CONFIGURATION CONSTANTS & LIMITS
//============================================
#define MAX_RETRY_ATTEMPTS   3                     // Maximum WiFi/MQTT connection attempts
#define RETRY_DELAY_MS       2000                  // Delay between retries (ms)
#define TIMEOUT_NORMAL       5000                  // Timeout for WiFi operations (ms)

// Watchdog timer configuration
#define WATCHDOG_TIMEOUT     WDTO_8S               // 8-second watchdog timeout
#define WATCHDOG_RESET_MS    7000                  // Reset watchdog every 7 seconds

// Sleep interval
#define SLEEP_INTERVAL_HOURS 2                     // Time between measurements (hours)
#define SLEEP_INTERVAL_MS    (SLEEP_INTERVAL_HOURS * 60 * 60 * 1000)

// Sensor validation ranges
#define MIN_WEIGHT_KG        0.0
#define MAX_WEIGHT_KG        500.0
#define MIN_TEMP_C           -40.0
#define MAX_TEMP_C           85.0
#define MIN_HUMIDITY         0.0
#define MAX_HUMIDITY         100.0
#define MIN_BATTERY_V        3.0
#define MAX_BATTERY_V        6.0

// Debug logging levels
#define DEBUG_OFF            0
#define DEBUG_ERRORS         1
#define DEBUG_INFO           2
#define DEBUG_VERBOSE        3
#define DEBUG_LEVEL          DEBUG_INFO            // Change for more/less output

// Buffer sizes
#define RESPONSE_BUF_SIZE    256                   // WiFi response buffer
#define JSON_PAYLOAD_SIZE    128                   // JSON message buffer

//============================================
// DEBUG LOGGING MACROS
//============================================
#if DEBUG_LEVEL >= DEBUG_ERRORS
  #define LOG_ERROR(msg) Serial.print(F("[ERR] ")); Serial.println(F(msg))
  #define LOG_ERROR_VAL(msg, val) do { Serial.print(F("[ERR] ")); Serial.print(F(msg)); Serial.println(val); } while(0)
#else
  #define LOG_ERROR(msg)
  #define LOG_ERROR_VAL(msg, val)
#endif

#if DEBUG_LEVEL >= DEBUG_INFO
  #define LOG_INFO(msg) Serial.print(F("[INF] ")); Serial.println(F(msg))
  #define LOG_INFO_VAL(msg, val) do { Serial.print(F("[INF] ")); Serial.print(F(msg)); Serial.println(val); } while(0)
#else
  #define LOG_INFO(msg)
  #define LOG_INFO_VAL(msg, val)
#endif

#if DEBUG_LEVEL >= DEBUG_VERBOSE
  #define LOG_VERBOSE(msg) Serial.print(F("[VRB] ")); Serial.println(F(msg))
  #define LOG_VERBOSE_VAL(msg, val) do { Serial.print(F("[VRB] ")); Serial.print(F(msg)); Serial.println(val); } while(0)
#else
  #define LOG_VERBOSE(msg)
  #define LOG_VERBOSE_VAL(msg, val)
#endif

//============================================
// GLOBAL VARIABLES
//============================================

// Sensor data
char weight[16];
char temp1[16];
char hum[16];
float batteryVoltage = 0.0;
float batteryPercent = 0.0;
float currentTemp = 0.0;
float currentHumidity = 0.0;
float currentWeight = 0.0;

// Scale calibration
HX711 scale(HX711_DOUT_PIN, HX711_CLK_PIN);
float SCALE = -19689.35;      // Calibration factor
long offset = -145680;         // Calibration offset

// DHT22 sensor
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Watchdog management
static unsigned long lastWatchdogReset = 0;

// Software Sleep
volatile bool shouldWake = false;
unsigned long sleepStartTime = 0;

// ESP-01 WiFi state
bool wifiConnected = false;
bool mqttConnected = false;

//============================================
// INTERRUPT HANDLER - BUTTON PRESS
//============================================

void buttonPressISR() {
  unsigned long now = millis();

  // Debounce: ignore if pressed within 50ms of last press
  if (now - lastButtonPress < BUTTON_DEBOUNCE_MS) {
    return;
  }

  lastButtonPress = now;
  buttonPressed = true;

  LOG_INFO("Button pressed!");
}

//============================================
// WATCHDOG MANAGEMENT
//============================================

void resetWatchdog() {
  unsigned long now = millis();
  if(now - lastWatchdogReset >= WATCHDOG_RESET_MS) {
    wdt_reset();
    lastWatchdogReset = now;
    LOG_VERBOSE("Watchdog reset");
  }
}

//============================================
// SOFTWARE SLEEP ISR
//============================================

ISR(WDT_vect) {
  // Check if we've slept long enough
  if (millis() - sleepStartTime >= SLEEP_INTERVAL_MS) {
    shouldWake = true;
  }
}

//============================================
// SENSOR VALIDATION FUNCTIONS
//============================================

float validateSensorValue(float value, float minVal, float maxVal, float defaultVal) {
  if(isnan(value) || isinf(value) || value < minVal || value > maxVal) {
    LOG_ERROR_VAL("Invalid sensor value: ", value);
    return defaultVal;
  }
  return value;
}

float readBatteryVoltage() {
  int rawValue = analogRead(BATTERY_PIN);
  float voltage = (rawValue / 1023.0) * 5.0 * 5.0;  // Convert to actual voltage
  return validateSensorValue(voltage, MIN_BATTERY_V, MAX_BATTERY_V, 4.5);
}

/**
 * Calculate battery percentage (0-100%)
 * Linear interpolation between EMPTY and FULL voltages
 */
float calculateBatteryPercent(float voltage) {
  float percent = ((voltage - BATTERY_EMPTY_VOLTAGE) / (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE)) * 100.0;

  // Clamp to 0-100%
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;

  return percent;
}

bool readAllSensors() {
  LOG_INFO("Reading sensors...");
  resetWatchdog();

  // Read weight from HX711
  if(!scale.is_ready()) {
    LOG_ERROR("HX711 not ready");
    return false;
  }
  float weight_val = scale.get_units(10) / 1000.0;  // Convert grams to kg
  weight_val = validateSensorValue(weight_val, MIN_WEIGHT_KG, MAX_WEIGHT_KG, 0.0);
  dtostrf(weight_val, 0, 2, weight);
  currentWeight = weight_val;
  LOG_INFO_VAL("Weight (kg): ", weight);

  // Read DHT22
  float temp_val = dht.readTemperature();
  temp_val = validateSensorValue(temp_val, MIN_TEMP_C, MAX_TEMP_C, 0.0);
  dtostrf(temp_val, 5, 2, temp1);
  currentTemp = temp_val;
  LOG_INFO_VAL("Temperature (C): ", temp1);

  float humidity_val = dht.readHumidity();
  humidity_val = validateSensorValue(humidity_val, MIN_HUMIDITY, MAX_HUMIDITY, 0.0);
  dtostrf(humidity_val, 5, 2, hum);
  currentHumidity = humidity_val;
  LOG_INFO_VAL("Humidity (%): ", hum);

  // Read battery voltage
  batteryVoltage = readBatteryVoltage();
  batteryPercent = calculateBatteryPercent(batteryVoltage);
  LOG_INFO_VAL("Battery (V): ", batteryVoltage);
  LOG_INFO_VAL("Battery (%): ", batteryPercent);

  resetWatchdog();
  return true;
}

//============================================
// LCD DISPLAY FUNCTIONS
//============================================

/**
 * Initialize LCD display
 */
void initializeLCD() {
  LOG_INFO("Initializing LCD...");

  Wire.begin();
  lcd.init();
  lcd.backlight();

  // Clear and display welcome message
  lcd.clear();
  lcd.print("BeezScale v3.1");
  lcd.setCursor(0, 1);
  lcd.print("Press button");

  LOG_INFO("LCD initialized");
}

/**
 * Turn LCD backlight on
 */
void lcdBacklightOn() {
  lcd.backlight();
  LOG_VERBOSE("LCD backlight ON");
}

/**
 * Turn LCD backlight off
 */
void lcdBacklightOff() {
  lcd.noBacklight();
  LOG_VERBOSE("LCD backlight OFF");
}

/**
 * Display temperature and humidity screen
 */
void displayTempHumidity() {
  lcd.clear();

  // First line: Temperature
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(currentTemp, 1);
  lcd.print((char)223);  // Degree symbol
  lcd.print("C");

  // Second line: Humidity
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(currentHumidity, 1);
  lcd.print("%");

  LOG_VERBOSE("Displaying: Temp/Humidity");
}

/**
 * Display weight screen
 */
void displayWeight() {
  lcd.clear();

  // First line: Label
  lcd.setCursor(0, 0);
  lcd.print("Weight (kg)");

  // Second line: Weight value
  lcd.setCursor(0, 1);
  lcd.print(currentWeight, 2);
  lcd.print(" kg");

  LOG_VERBOSE("Displaying: Weight");
}

/**
 * Display battery percentage screen
 */
void displayBattery() {
  lcd.clear();

  // First line: Label
  lcd.setCursor(0, 0);
  lcd.print("Battery");

  // Second line: Percentage
  lcd.setCursor(0, 1);
  lcd.print(batteryPercent, 0);
  lcd.print("% ");
  lcd.print(batteryVoltage, 2);
  lcd.print("V");

  LOG_VERBOSE("Displaying: Battery");
}

/**
 * Handle the display state machine
 * Cycles through: Temp/Humidity -> Weight -> Battery, then turns off
 */
void updateLCDDisplay() {
  unsigned long elapsed = millis() - displayStartTime;

  // Check if we should move to next state (2 seconds per screen)
  if (elapsed >= DISPLAY_DURATION_MS) {
    currentDisplayState = (DisplayState)((int)currentDisplayState + 1);
    displayStartTime = millis();

    // All screens shown, turn off
    if (currentDisplayState > STATE_BATTERY) {
      lcdBacklightOff();
      currentDisplayState = STATE_OFF;
      return;
    }
  }

  // Display current state
  switch(currentDisplayState) {
    case STATE_TEMP_HUMID:
      displayTempHumidity();
      break;
    case STATE_WEIGHT:
      displayWeight();
      break;
    case STATE_BATTERY:
      displayBattery();
      break;
    case STATE_OFF:
    default:
      break;
  }
}

//============================================
// ESP-01 WIFI + MQTT FUNCTIONS
//============================================

bool sendESPCommand(const char* command, const char* expectedResponse, unsigned long timeout) {
  LOG_VERBOSE(command);
  espSerial.println(command);

  unsigned long startTime = millis();
  String response = "";

  while(millis() - startTime < timeout) {
    resetWatchdog();

    if(espSerial.available()) {
      char c = espSerial.read();
      response += c;

      if(response.indexOf(expectedResponse) != -1) {
        LOG_VERBOSE_VAL("Response: ", response.c_str());
        return true;
      }
    }
  }

  LOG_ERROR_VAL("Timeout or no response for: ", command);
  return false;
}

bool connectToWiFi() {
  LOG_INFO("Connecting to WiFi...");
  resetWatchdog();

  LOG_INFO("Resetting ESP-01...");
  if(!sendESPCommand("AT+RST", "ready", TIMEOUT_NORMAL)) {
    LOG_ERROR("ESP-01 reset failed");
    return false;
  }
  delay(2000);
  resetWatchdog();

  if(sendESPCommand("AT+CWJAP?", WIFI_SSID, 1000)) {
    LOG_INFO("Already connected to WiFi");
    wifiConnected = true;
    return true;
  }

  char wifiCmd[128];
  snprintf(wifiCmd, sizeof(wifiCmd), "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASSWORD);

  if(!sendESPCommand(wifiCmd, "OK", TIMEOUT_NORMAL)) {
    LOG_ERROR("WiFi connection failed");
    return false;
  }

  LOG_INFO("WiFi connected!");
  wifiConnected = true;
  resetWatchdog();
  return true;
}

bool connectToMQTT() {
  if(!wifiConnected) {
    LOG_ERROR("WiFi not connected, cannot connect to MQTT");
    return false;
  }

  LOG_INFO("Connecting to MQTT broker...");
  resetWatchdog();

  char mqttCmd[256];
  snprintf(mqttCmd, sizeof(mqttCmd),
           "AT+MQTTCONN=\"%s\",%d,\"%s\",%d",
           MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID, MQTT_KEEPALIVE);

  if(!sendESPCommand(mqttCmd, "+MQTTCONNECTED:", TIMEOUT_NORMAL)) {
    LOG_ERROR("MQTT connection failed");
    return false;
  }

  LOG_INFO("MQTT connected!");
  mqttConnected = true;
  resetWatchdog();
  return true;
}

bool publishToMQTT() {
  if(!mqttConnected) {
    LOG_ERROR("MQTT not connected");
    return false;
  }

  LOG_INFO("Publishing data to MQTT...");
  resetWatchdog();

  char payload[JSON_PAYLOAD_SIZE];
  snprintf(payload, sizeof(payload),
           "{\"temperature\":%.2f,\"humidity\":%.2f,\"weight\":%.2f,\"battery_voltage\":%.2f,\"battery_percent\":%.0f}",
           currentTemp, currentHumidity, currentWeight, batteryVoltage, batteryPercent);

  char pubCmd[256];
  snprintf(pubCmd, sizeof(pubCmd), "AT+MQTTPUB=\"%s\",\"%s\"", MQTT_TOPIC, payload);

  if(!sendESPCommand(pubCmd, "OK", TIMEOUT_NORMAL)) {
    LOG_ERROR("MQTT publish failed");
    return false;
  }

  LOG_INFO("Data published successfully!");
  resetWatchdog();
  return true;
}

void disconnectAll() {
  LOG_INFO("Disconnecting from services...");
  resetWatchdog();

  if(mqttConnected) {
    sendESPCommand("AT+MQTTDISCONN", "OK", TIMEOUT_NORMAL);
    mqttConnected = false;
  }

  if(wifiConnected) {
    sendESPCommand("AT+CWQAP", "OK", TIMEOUT_NORMAL);
    wifiConnected = false;
  }

  LOG_INFO("Disconnected");
  resetWatchdog();
}

//============================================
// MEASUREMENT CYCLE
//============================================

void Request() {
  LOG_INFO("========== Starting Measurement Cycle ==========");
  resetWatchdog();

  if(!readAllSensors()) {
    LOG_ERROR("Sensor reading failed");
    return;
  }

  espSerial.begin(115200);
  delay(500);
  resetWatchdog();

  bool wifiOK = false;
  for(int i = 0; i < MAX_RETRY_ATTEMPTS; i++) {
    if(connectToWiFi()) {
      wifiOK = true;
      break;
    }
    delay(RETRY_DELAY_MS);
    resetWatchdog();
  }

  if(!wifiOK) {
    LOG_ERROR("Failed to connect to WiFi after retries");
    disconnectAll();
    return;
  }

  bool mqttOK = false;
  for(int i = 0; i < MAX_RETRY_ATTEMPTS; i++) {
    if(connectToMQTT()) {
      mqttOK = true;
      break;
    }
    delay(RETRY_DELAY_MS);
    resetWatchdog();
  }

  if(!mqttOK) {
    LOG_ERROR("Failed to connect to MQTT after retries");
    disconnectAll();
    return;
  }

  if(!publishToMQTT()) {
    LOG_ERROR("Failed to publish data");
  }

  disconnectAll();
  espSerial.end();

  LOG_INFO("========== Measurement Cycle Complete ==========");
  resetWatchdog();
}

//============================================
// POWER MANAGEMENT - SOFTWARE SLEEP
//============================================

void goToSleep() {
  LOG_INFO("Entering sleep mode for 2 hours...");
  LOG_INFO("System consuming ~1mA during sleep");

  // Turn off LCD
  lcdBacklightOff();
  currentDisplayState = STATE_OFF;

  // Disable serial communication to save power
  Serial.end();

  // Record start time
  sleepStartTime = millis();
  shouldWake = false;

  // Configure watchdog timer
  wdt_enable(WATCHDOG_TIMEOUT);
  WDTCSR |= _BV(WDIE);

  // Configure sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sleep_enable();

  // Sleep until interval has elapsed
  while(!shouldWake) {
    sleep_mode();
  }

  // Time is up! Wake up properly
  sleep_disable();
  wdt_disable();

  // Reinitialize serial
  Serial.begin(9600);
  delay(500);

  LOG_INFO("Woken from sleep! Starting next measurement cycle...");
}

//============================================
// SETUP
//============================================

void setup() {
  // Initialize watchdog
  wdt_enable(WATCHDOG_TIMEOUT);
  wdt_reset();
  lastWatchdogReset = millis();

  // Initialize serial
  Serial.begin(9600);
  delay(500);

  LOG_INFO("===========================================");
  LOG_INFO("ArduiBeeScale - WiFi + MQTT + LCD Edition");
  LOG_INFO("===========================================");
  LOG_INFO("System initializing...");

  // Initialize push button with interrupt
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressISR, FALLING);
  LOG_INFO("Button initialized on pin: " + String(BUTTON_PIN));

  // Initialize LCD
  initializeLCD();

  // Initialize sensors
  scale.begin();
  scale.set_scale(SCALE);
  scale.set_offset(offset);
  dht.begin();

  LOG_INFO("Sensors initialized");
  LOG_INFO_VAL("WiFi SSID: ", WIFI_SSID);
  LOG_INFO_VAL("MQTT Broker: ", MQTT_BROKER);
  LOG_INFO_VAL("MQTT Topic: ", MQTT_TOPIC);

  resetWatchdog();

  delay(1000);
  LOG_INFO("Starting first measurement...");
}

//============================================
// MAIN LOOP
//============================================

void loop() {
  resetWatchdog();

  // Handle button press and LCD display
  if (buttonPressed) {
    buttonPressed = false;

    // Only start display if currently off
    if (currentDisplayState == STATE_OFF) {
      LOG_INFO("Starting display sequence...");

      // Read fresh sensor data for display
      readAllSensors();

      // Start display sequence
      lcdBacklightOn();
      currentDisplayState = STATE_TEMP_HUMID;
      displayStartTime = millis();
    }
  }

  // Update LCD if in display sequence
  if (currentDisplayState != STATE_OFF) {
    updateLCDDisplay();
  }

  // Check if it's time for measurement cycle
  // (This would be integrated with your sleep cycle)
  // For now, perform measurement and sleep
  Request();

  goToSleep();
}
