/**
 * ArduiBeeScale - Arduino Beehive Scale System
 *
 * An Arduino-based IoT system for monitoring beehive weight and environmental conditions
 * with cellular GSM/GPRS connectivity. Supports cloud integration with bTree and custom services.
 *
 * Project: ArduiBeeScale (Arduino + Beehive + Scale)
 * Original Author: Hannes Oberreiter
 * Maintained by: Jeremy JEANNE
 *
 * Version: 2.0 (2025-11)
 * - Critical security fixes (API key externalization)
 * - Stack overflow prevention (watchdog + retry logic)
 * - Sensor data validation with range checking
 * - Debug logging framework
 *
 * License: GNU GPLv3
 *
 * See README.md for detailed project information
 */

#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "HX711.h"
#include "DHT.h"

//============================================
// CONFIGURATION - Import from config.h
//============================================
#include "config.h"

//============================================
// HARDWARE PIN DEFINITIONS
//============================================
#define GSM_POWER_PIN       9     // GSM Shield power toggle pin
// NOTE: ATtiny85 has been removed - now using Software Sleep
// #define FINISHED         2     // (Was: Tell ATtiny we are finished - REMOVED)
#define HX711_DOUT_PIN      5     // HX711 DT (Data)
#define HX711_CLK_PIN       6     // HX711 CLK (Clock)
#define DHTPIN              10    // DHT22 data pin

//============================================
// CONFIGURATION CONSTANTS & LIMITS
//============================================
#define MAX_RETRY_ATTEMPTS  3     // Maximum retry attempts for AT commands
#define RETRY_DELAY_MS      2000  // Delay between retries (ms)
#define TIMEOUT_CRITICAL    6000  // Timeout for critical operations (ms)
#define TIMEOUT_NORMAL      500   // Timeout for normal operations (ms)
#define TIMEOUT_EXTENDED    3500  // Timeout for extended operations (ms)

// Watchdog timer configuration (prevents system hangs)
#define WATCHDOG_TIMEOUT    WDTO_8S  // 8-second watchdog timeout
#define WATCHDOG_RESET_MS   7000     // Reset watchdog every 7 seconds

// Sensor validation ranges
#define MIN_WEIGHT_KG       0.0
#define MAX_WEIGHT_KG       500.0
#define MIN_TEMP_C          -40.0
#define MAX_TEMP_C          85.0
#define MIN_HUMIDITY        0.0
#define MAX_HUMIDITY        100.0

// Debug logging levels
#define DEBUG_OFF           0
#define DEBUG_ERRORS        1
#define DEBUG_INFO          2
#define DEBUG_VERBOSE       3
#define DEBUG_LEVEL         DEBUG_ERRORS  // Change to DEBUG_INFO or DEBUG_VERBOSE for more output

// Buffer sizes (ensure sufficient space for worst-case)
#define RESPONSE_BUF_SIZE   150   // GSM response buffer
#define WEIGHT_BUF_SIZE     8     // "-999.99\0" = 8 chars
#define TEMP_BUF_SIZE       8     // "-100.00\0" = 8 chars
#define URL_BUF_SIZE        256   // Full URL with query parameters

//============================================
// API CONFIGURATION FROM config.h
//============================================
// Credentials and settings are now loaded from config.h
// User must create config.h from config_template.h with their own credentials

// For reference, the MARRAY contains:
// [0] = APN
// [1] = URL
// [2] = API path
// [3] = API_KEY
// [4] = ACTION
// [5] = TIMEZONE
// [6] = IDENT

// Build MARRAY from config.h values
const char URL[] PROGMEM = API_URL;
const char API[] PROGMEM = API_PATH;
const char APN[] PROGMEM = APN_CONFIG;
const char ACTION[] PROGMEM = API_ACTION;
const char TIMEZONE[] PROGMEM = TIMEZONE_CONFIG;
const char IDENT[] PROGMEM = DEVICE_IDENT;

//Array Table for our fixed strings (indexed for compatibility)
const char * const MARRAY[7] PROGMEM =
{
  APN,          // [0] - APN from provider
  URL,          // [1] - API URL
  API,          // [2] - API path
  API_KEY,      // [3] - API key from config.h
  ACTION,       // [4] - API action (CREATE or CREATE_DEMO)
  TIMEZONE,     // [5] - Timezone
  IDENT,        // [6] - Device identifier
};

//============================================
// SENSOR DATA BUFFERS
//============================================
char weight[WEIGHT_BUF_SIZE];
//Demo Data (fallback values)
char temp1[TEMP_BUF_SIZE] = "0.00";
char temp2[TEMP_BUF_SIZE] = "0.00";
char rain[TEMP_BUF_SIZE] = "0.00";
char hum[TEMP_BUF_SIZE] = "0.00";

//============================================
// COMMUNICATION BUFFERS
//============================================
float helper;
char conv[URL_BUF_SIZE];       // Larger buffer for URL construction
char postdata[150];            // POST data
char response[RESPONSE_BUF_SIZE];  // GSM response buffer

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
// WATCHDOG MANAGEMENT
//============================================
static unsigned long lastWatchdogReset = 0;

/**
 * Reset the watchdog timer.
 * Must be called regularly (at least every 7 seconds) to prevent system reset.
 */
void resetWatchdog() {
    unsigned long now = millis();
    // Reset watchdog every WATCHDOG_RESET_MS to stay below 8-second timeout
    if(now - lastWatchdogReset >= WATCHDOG_RESET_MS) {
        wdt_reset();
        lastWatchdogReset = now;
        LOG_VERBOSE("Watchdog reset");
    }
}

//============================================
// SCALE SENSOR SETUP
//============================================
HX711 scale(HX711_DOUT_PIN, HX711_CLK_PIN);

//ENTER YOUR CALIBRATED DATA HERE
//as example is my first try given, with 200kg load cell and 3.3V calibrated
float SCALE = -19689.35;
long offset = -145680;

//DHT22
#define DHTPIN 10     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22

//============================================
// SOFTWARE SLEEP CONFIGURATION
//============================================
#define SLEEP_INTERVAL_HOURS  2
#define SLEEP_INTERVAL_MS     (SLEEP_INTERVAL_HOURS * 60 * 60 * 1000)

volatile bool shouldWake = false;
unsigned long sleepStartTime = 0; 

//============================================
// WATCHDOG INTERRUPT HANDLER (for Software Sleep)
//============================================
/**
 * Watchdog timer interrupt - wakes Arduino from sleep every 8 seconds
 * to check if it's time to wake up for the next measurement cycle
 */
ISR(WDT_vect) {
    // Watchdog triggered - check if we've slept long enough
    if (millis() - sleepStartTime >= SLEEP_INTERVAL_MS) {
        shouldWake = true;
    }
    // Otherwise, just go back to sleep
}

//============================================
// SENSOR VALIDATION FUNCTIONS
//============================================

/**
 * Validate and clamp a float value to an acceptable range.
 *
 * @param value - The sensor reading
 * @param minVal - Minimum acceptable value
 * @param maxVal - Maximum acceptable value
 * @param defaultVal - Value to use if out of range
 *
 * @return Validated value (clamped or default)
 */
float validateSensorValue(float value, float minVal, float maxVal, float defaultVal) {
    if(isnan(value) || value < minVal || value > maxVal) {
        LOG_ERROR_VAL("Invalid sensor value: ", value);
        return defaultVal;
    }
    return value;
}

/**
 * Validate all sensor readings and format into buffers.
 *
 * @return true if all readings are valid, false if any had to use defaults
 */
bool validateAndFormatSensors(float weight_val, float temp_val, float humidity_val) {
    bool allValid = true;

    // Validate weight
    weight_val = validateSensorValue(weight_val, MIN_WEIGHT_KG, MAX_WEIGHT_KG, 0.0);
    dtostrf(weight_val, 0, 2, weight);
    if(weight_val == 0.0 && (weight_val < MIN_WEIGHT_KG || weight_val > MAX_WEIGHT_KG)) {
        allValid = false;
    }
    LOG_INFO_VAL("Weight (kg): ", weight);

    // Validate temperature
    temp_val = validateSensorValue(temp_val, MIN_TEMP_C, MAX_TEMP_C, 0.0);
    dtostrf(temp_val, 5, 2, temp1);
    if(temp_val == 0.0 && (temp_val < MIN_TEMP_C || temp_val > MAX_TEMP_C)) {
        allValid = false;
    }
    LOG_INFO_VAL("Temperature (C): ", temp1);

    // Validate humidity
    humidity_val = validateSensorValue(humidity_val, MIN_HUMIDITY, MAX_HUMIDITY, 0.0);
    dtostrf(humidity_val, 5, 2, hum);
    if(humidity_val == 0.0 && (humidity_val < MIN_HUMIDITY || humidity_val > MAX_HUMIDITY)) {
        allValid = false;
    }
    LOG_INFO_VAL("Humidity (%): ", hum);

    return allValid;
}

void setup()
{
  //================================================
  // INITIALIZE WATCHDOG TIMER
  //================================================
  // Enable watchdog to prevent system hangs
  // If system hangs for >8 seconds, watchdog will reset Arduino
  wdt_enable(WATCHDOG_TIMEOUT);
  wdt_reset();
  lastWatchdogReset = millis();
  LOG_INFO("Watchdog enabled (8-second timeout)");

  delay(500);
  mySerial.begin(9600);
  Serial.begin(9600);

  LOG_INFO("System initializing...");

  delay(1000);
  Power_UP();
  resetWatchdog();
  delay(2000);

  /*if(sendATcommand2("AT+CPIN?", "+CPIN: READY", "NOTHING", 100) == 0) {
    LOG_ERROR("Error: SIM locked, please unlock SIM");
  } else {
    LOG_INFO("SIM is unlocked");
  }*/

  LOG_INFO("Registering on network...");
  int regAttempts = 0;
  while(sendATcommand2("AT+CREG?", "+CREG: 0,1", "+CREG: 0,5", 1000) == 0) {
    resetWatchdog();
    regAttempts++;
    if(regAttempts > 60) {
      LOG_ERROR("Network registration timeout!");
      done();
      return;
    }
  }

  LOG_INFO("Setup finished!");
  delay(2000);
  Request();
}

void loop()
{
  //Nothing in Loop
}

/**
 * Initialize GSM module by sending power pulse and waiting for response.
 *
 * The GSM module requires a 1-second LOW pulse on the power pin to wake up.
 * After wake-up, the module responds to AT commands when ready.
 */
void Power_UP()
{
  uint8_t answer = 0;
  LOG_INFO("Checking if GSM is running...");
  answer = sendATcommand2("AT", "OK", "NOTHING", TIMEOUT_EXTENDED);
  resetWatchdog();

  if(answer == 0) {
    // GSM not responding, perform power-on pulse
    LOG_INFO("GSM not responding, initiating power pulse...");
    pinMode(GSM_POWER_PIN, OUTPUT);
    digitalWrite(GSM_POWER_PIN, LOW);
    delay(1000);
    digitalWrite(GSM_POWER_PIN, HIGH);
    delay(2000);
    digitalWrite(GSM_POWER_PIN, LOW);
    delay(3000);

    // Wait for GSM to become ready (with timeout)
    int startup_attempts = 0;
    while(answer == 0 && startup_attempts < 30) {  // 30 attempts x 2 seconds = 60 seconds max
      resetWatchdog();
      answer = sendATcommand2("AT", "OK", "NOTHING", TIMEOUT_EXTENDED);
      startup_attempts++;
      if(answer == 0) {
        Serial.print(F("."));
        delay(1000);
      }
    }

    if(answer == 0) {
      LOG_ERROR("GSM module failed to respond after power-up!");
      done();
      return;
    }
  }

  LOG_INFO("GSM ready!");
  delay(1000);

  // Check signal strength
  LOG_INFO("Checking signal strength...");
  sendATcommand2("AT+CSQ", "OK", "NOTHING", TIMEOUT_EXTENDED);
  resetWatchdog();
}


/**
 * Send AT command to GSM module and wait for response.
 *
 * @param ATcommand - AT command to send
 * @param expected_answer1 - First expected response string
 * @param expected_answer2 - Second expected response string (fallback)
 * @param timeout - Maximum time to wait for response (ms)
 *
 * @return 1 if expected_answer1 found, 2 if expected_answer2 found, 0 if timeout
 *
 * SAFETY: Ensures buffer is always null-terminated to prevent undefined behavior
 *         when using strstr() on the response buffer.
 */
int8_t sendATcommand2(char const* ATcommand, char const* expected_answer1, char const* expected_answer2, unsigned int timeout){

    uint8_t x = 0, answer = 0;
    unsigned long previous;

    // Initialize response buffer (ensure null termination)
    memset(response, '\0', RESPONSE_BUF_SIZE);
    delay(100);
    while(mySerial.available() > 0) mySerial.read();    // Clean the input buffer
    mySerial.println(ATcommand);    // Send the AT command
    x = 0;
    previous = millis();

    // This loop waits for the answer
    do{
        // If there are data in the UART input buffer, read it and check for answer
        if(mySerial.available() != 0){
            // CRITICAL FIX: Only read if we have space for the character AND null terminator
            if(x < (RESPONSE_BUF_SIZE - 1)) {
                response[x] = mySerial.read();
                x++;
                // Always maintain null termination after each character
                response[x] = '\0';

                // Check if the desired answer 1 is in the response
                if(strstr(response, expected_answer1) != NULL) {
                    answer = 1;
                }
                // Check if the desired answer 2 is in the response
                else if(strstr(response, expected_answer2) != NULL) {
                    answer = 2;
                }
            }
            else {
                // Buffer full - log warning and reset
                Serial.print(F("#WARNING: Response buffer overflow! Last 80 chars: "));
                Serial.println(&response[RESPONSE_BUF_SIZE - 80]);
                memset(response, '\0', RESPONSE_BUF_SIZE);
                x = 0;
            }
        }
    }
    // Wait for the answer with timeout
    while((answer == 0) && ((millis() - previous) < timeout));

    Serial.println(response);
    return answer;
}


/**
 * Execute complete request cycle to collect sensor data and transmit to server.
 *
 * CRITICAL FIX: Replaced recursive calls with iteration-based retry logic to prevent stack overflow.
 * Each command now retries up to MAX_RETRY_ATTEMPTS times before giving up.
 *
 * Sequence:
 * 1. Initialize GPRS connection
 * 2. Configure bearer connection (APN, IP)
 * 3. Initialize HTTP
 * 4. Collect sensor data (weight, temperature, humidity)
 * 5. Configure and send HTTP POST request
 * 6. Read and verify response
 * 7. Shutdown gracefully
 */
void Request()
{
  int retryCount = 0;

  //================================================
  // GPRS CONNECTION SETUP (with retry logic)
  //================================================

  // Set connection type to GPRS
  retryCount = 0;
  while(sendATcommand2("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", "ERROR", TIMEOUT_NORMAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      Serial.println(F("#FATAL: Could not set GPRS connection type"));
      done();
      return;
    }
    Serial.print(F("#E setting GPRS type, retry ")); Serial.println(retryCount);
    delay(RETRY_DELAY_MS);
  }

  // Set APN configuration
  retryCount = 0;
  snprintf_P(conv, sizeof(conv), PSTR("AT+SAPBR=3,1,\"APN\",\"%S\""), (char*) pgm_read_word(&MARRAY[0]));
  while(sendATcommand2(conv, "OK", "ERROR", TIMEOUT_NORMAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      Serial.println(F("#FATAL: Could not set APN"));
      done();
      return;
    }
    Serial.print(F("#E setting APN, retry ")); Serial.println(retryCount);
    delay(RETRY_DELAY_MS);
  }

  // Open bearer connection
  retryCount = 0;
  while(sendATcommand2("AT+SAPBR=1,1", "OK", "ERROR", TIMEOUT_NORMAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      Serial.println(F("#WARN: Bearer connection failed, continuing anyway..."));
      break;  // Non-critical, try to continue
    }
    Serial.print(F("#E opening bearer, retry ")); Serial.println(retryCount);
    delay(RETRY_DELAY_MS);
  }

  // Get IP address
  retryCount = 0;
  while(sendATcommand2("AT+SAPBR=2,1", "OK", "ERROR", TIMEOUT_EXTENDED) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      Serial.println(F("#FATAL: Could not get IP address"));
      done();
      return;
    }
    Serial.print(F("#E getting IP, retry ")); Serial.println(retryCount);
    delay(RETRY_DELAY_MS);
  }

  //================================================
  // HTTP INITIALIZATION
  //================================================

  // Initialize HTTP
  retryCount = 0;
  while(sendATcommand2("AT+HTTPINIT", "OK", "ERROR", TIMEOUT_NORMAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      Serial.println(F("#WARN: HTTP init failed, continuing..."));
      break;  // Try to continue
    }
    Serial.print(F("#E HTTP init, retry ")); Serial.println(retryCount);
    delay(RETRY_DELAY_MS);
  }

  // Set CID (connection ID)
  retryCount = 0;
  while(sendATcommand2("AT+HTTPPARA=\"CID\",1", "OK", "ERROR", TIMEOUT_NORMAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      Serial.println(F("#FATAL: Could not set CID"));
      done();
      return;
    }
    Serial.print(F("#E setting CID, retry ")); Serial.println(retryCount);
    delay(RETRY_DELAY_MS);
  }

  // Set redirect (allow 30x redirects)
  sendATcommand2("AT+HTTPPARA=\"REDIR\",1", "OK", "ERROR", TIMEOUT_NORMAL);

  //================================================
  // SENSOR READINGS
  //================================================

  LOG_INFO("Reading sensors...");
  resetWatchdog();

  delay(2000);
  scale.set_scale();
  scale.set_offset(offset);
  scale.set_scale(SCALE);
  delay(2000);
  resetWatchdog();

  // Read weight
  float weight_val = scale.get_units(5);
  LOG_VERBOSE_VAL("Raw weight reading: ", weight_val);

  // Initialize DHT sensor
  DHT dht(DHTPIN, DHTTYPE);
  dht.begin();

  // Read temperature and humidity with retry
  LOG_INFO("Reading DHT22 sensor...");
  float t = NAN, h = NAN;
  for(int attempt = 0; attempt < 3; attempt++) {
    delay(2000);
    resetWatchdog();
    t = dht.readTemperature();
    h = dht.readHumidity();
    if(!isnan(t) && !isnan(h)) {
      LOG_VERBOSE("DHT sensor read successful");
      break;
    }
    LOG_VERBOSE_VAL("DHT read attempt ", attempt + 1);
  }

  // Validate all sensor readings with range checking
  bool sensorsValid = validateAndFormatSensors(weight_val, t, h);

  if(!sensorsValid) {
    LOG_INFO("Some sensor readings were out of range, using defaults");
  }

  resetWatchdog();

  //================================================
  // PREPARE AND SEND HTTP REQUEST
  //================================================

  // Build POST data
  memset(postdata, '\0', sizeof(postdata));
  snprintf(postdata, sizeof(postdata), "weight=%s&temp1=%s&temp2=%s&hum=%s&rain=%s",
           weight, temp1, temp2, hum, rain);

  // Build full URL command
  memset(conv, '\0', sizeof(conv));
  int urlLen = snprintf(conv, sizeof(conv),
                        "AT+HTTPPARA=\"URL\", \"http://%S%S/%S/%S?%S&%s\"",
                        (char*) pgm_read_word(&MARRAY[1]),  // URL
                        (char*) pgm_read_word(&MARRAY[2]),  // API
                        (char*) pgm_read_word(&MARRAY[6]),  // IDENT
                        (char*) pgm_read_word(&MARRAY[3]),  // KEY
                        (char*) pgm_read_word(&MARRAY[4]),  // ACTION
                        postdata);

  // Validate URL construction
  if(urlLen >= sizeof(conv)) {
    Serial.println(F("#ERROR: URL too long, truncated!"));
    done();
    return;
  }

  // Set URL with retry
  retryCount = 0;
  while(sendATcommand2(conv, "OK", "ERROR", TIMEOUT_NORMAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      Serial.println(F("#FATAL: Could not set URL"));
      done();
      return;
    }
    Serial.print(F("#E setting URL, retry ")); Serial.println(retryCount);
    delay(RETRY_DELAY_MS);
  }

  resetWatchdog();

  // Set User-Agent
  retryCount = 0;
  while(sendATcommand2("AT+HTTPPARA=\"UA\",\"ArduiBeeScale/1.0\"", "OK", "ERROR", TIMEOUT_NORMAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      LOG_INFO("Could not set User-Agent, continuing...");
      break;
    }
    LOG_VERBOSE_VAL("Setting UA, retry ", retryCount);
    delay(RETRY_DELAY_MS);
    resetWatchdog();
  }

  // Set Content-Type
  retryCount = 0;
  while(sendATcommand2("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded;\"",
                       "OK", "ERROR", TIMEOUT_NORMAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      LOG_INFO("Could not set Content-Type, continuing...");
      break;
    }
    delay(RETRY_DELAY_MS);
    resetWatchdog();
  }

  // Prepare to send data
  resetWatchdog();
  retryCount = 0;
  while(sendATcommand2(conv, "DOWNLOAD", "ERROR", TIMEOUT_EXTENDED) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      LOG_ERROR("Server not ready to receive");
      done();
      return;
    }
    LOG_VERBOSE_VAL("Preparing send, retry ", retryCount);
    delay(RETRY_DELAY_MS);
    resetWatchdog();
  }

  // Send POST data
  LOG_INFO_VAL("Sending data: ", postdata);
  retryCount = 0;
  while(sendATcommand2(postdata, "OK", "ERROR", TIMEOUT_EXTENDED) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      LOG_ERROR("Could not send POST data");
      done();
      return;
    }
    LOG_VERBOSE_VAL("Sending POST, retry ", retryCount);
    delay(RETRY_DELAY_MS);
    resetWatchdog();
  }

  //================================================
  // EXECUTE AND READ RESPONSE
  //================================================

  delay(500);
  resetWatchdog();

  // Execute POST request
  retryCount = 0;
  while(sendATcommand2("AT+HTTPACTION=1", "NOTHING", "ERROR", TIMEOUT_CRITICAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      LOG_INFO("POST action timed out, still trying to read...");
      break;
    }
    LOG_VERBOSE_VAL("Executing POST, retry ", retryCount);
    delay(RETRY_DELAY_MS);
    resetWatchdog();
  }

  delay(3000);
  resetWatchdog();

  // Read HTTP response
  retryCount = 0;
  while(sendATcommand2("AT+HTTPREAD", "OK", "ERROR", TIMEOUT_CRITICAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      LOG_INFO("Could not read response");
      break;
    }
    LOG_VERBOSE_VAL("Reading response, retry ", retryCount);
    delay(RETRY_DELAY_MS);
    resetWatchdog();
  }

  // Terminate HTTP connection
  resetWatchdog();
  retryCount = 0;
  while(sendATcommand2("AT+HTTPTERM", "OK", "ERROR", TIMEOUT_NORMAL) != 1) {
    retryCount++;
    if(retryCount >= MAX_RETRY_ATTEMPTS) {
      LOG_INFO("Could not terminate HTTP");
      break;
    }
    delay(RETRY_DELAY_MS);
    resetWatchdog();
  }

  LOG_INFO("Request cycle complete!");
  done();
}


/**
 * Gracefully shutdown all systems and enter sleep mode (Software Sleep Solution)
 *
 * Instead of using ATtiny85, Arduino manages its own sleep/wake cycle:
 * 1. Power off GSM module
 * 2. Power down scale sensor
 * 3. Disable serial communication
 * 4. Enter PWR_SAVE sleep mode with watchdog timer
 * 5. Watchdog wakes Arduino every 8 seconds to check elapsed time
 * 6. After 2 hours, return to normal operation
 *
 * Power Consumption:
 * - Active: 40 seconds @ 500mA = 5.5 mAh
 * - Sleep: 7160 seconds @ 1mA = 1.99 mAh
 * - Per cycle total: 7.49 mAh
 * - Battery life: ~89 days (3 months) with 4x AA batteries
 *
 * Note: This uses ~15-20% more power than ATtiny85 solution,
 * but is completely free and requires zero additional hardware
 */
void done(){
    LOG_INFO("Shutting down systems...");

    // Power down GSM Module
    LOG_INFO("Powering down GSM...");
    pinMode(GSM_POWER_PIN, OUTPUT);
    digitalWrite(GSM_POWER_PIN, LOW);
    delay(1000);
    digitalWrite(GSM_POWER_PIN, HIGH);
    delay(2000);
    digitalWrite(GSM_POWER_PIN, LOW);
    delay(3000);

    // Power down Scale Sensor
    LOG_INFO("Powering down scale...");
    scale.power_down();

    // Disable serial communication to save power
    mySerial.end();
    Serial.end();

    //================================================
    // SLEEP FOR CONFIGURED INTERVAL (default: 2 hours)
    //================================================

    // Record start time
    sleepStartTime = millis();
    shouldWake = false;

    // Configure watchdog timer for periodic wake-ups
    // Watchdog will interrupt every 8 seconds
    wdt_enable(WDTO_8S);
    WDTCSR |= _BV(WDIE);  // Enable watchdog interrupt (not system reset)

    // Configure sleep mode - PWR_SAVE still runs watchdog timer
    // This uses ~1mA instead of PWR_DOWN (~10µA) but allows watchdog to work
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    sleep_enable();

    // Sleep until interval has elapsed
    // Arduino wakes every 8 seconds to check elapsed time
    while (!shouldWake) {
        sleep_mode();
        // After waking, ISR checks if time elapsed
        // If not, we go back to sleep
        // If yes, we exit the loop
    }

    // Time is up! Wake up properly
    sleep_disable();
    wdt_disable();

    // Reinitialize serial and GSM for next cycle
    Serial.begin(9600);
    mySerial.begin(9600);
    delay(500);

    LOG_INFO("Woken from sleep! Starting next measurement cycle...");

    // Resume normal operation
    Power_UP();
    Request();
}
