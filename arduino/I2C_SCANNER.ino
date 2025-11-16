/**
 * ArduiBeeScale - I2C Device Scanner
 *
 * Purpose: Find the I2C address of your LCD1602 display
 *
 * Instructions:
 * 1. Connect your LCD1602 I2C module to Arduino (VCC, GND, SDA on A4, SCL on A5)
 * 2. Upload this sketch to Arduino
 * 3. Open Serial Monitor (9600 baud)
 * 4. Look for: "I2C device found at address 0xXX"
 * 5. Note the address (usually 0x27 or 0x3F)
 * 6. Update LCD_I2C_ADDRESS in config_wifi.h with this address
 *
 * Expected Output:
 * ┌──────────────────────────────────────┐
 * │ Scanning I2C bus...                  │
 * │ I2C device found at address 0x27     │
 * │ Scanning done.                       │
 * └──────────────────────────────────────┘
 */

#include <Wire.h>

void setup() {
  Serial.begin(9600);

  // Wait for Serial to be ready
  while (!Serial) {
    delay(10);
  }

  delay(1000);
  Serial.println("\n\nI2C Scanner");
  Serial.println("====================");
  Serial.println("Scanning I2C bus...\n");

  scanI2C();

  Serial.println("\nScan complete!");
  Serial.println("====================");
  Serial.println("\nNote the address above (usually 0x27 or 0x3F)");
  Serial.println("Update LCD_I2C_ADDRESS in config_wifi.h");
}

void loop() {
  // Run scan once on startup only
  delay(5000);
}

/**
 * Scan I2C bus for connected devices
 *
 * I2C addresses range from 0x01 to 0x7F
 * Some addresses are reserved for special purposes
 */
void scanI2C() {
  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    // Begin transmission with address
    Wire.beginTransmission(address);

    // End transmission and check for ACK
    error = Wire.endTransmission();

    if (error == 0) {
      // Device found!
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(" !");

      // Check if this looks like an LCD address
      if (address == 0x27 || address == 0x3F) {
        Serial.println("  ^ This looks like your LCD1602 address!");
      }

      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found");
    Serial.println("\nTroubleshooting:");
    Serial.println("- Check VCC (5V) connection to LCD");
    Serial.println("- Check GND connection to LCD");
    Serial.println("- Verify A4 (SDA) connection");
    Serial.println("- Verify A5 (SCL) connection");
    Serial.println("- Try power cycling the LCD");
    Serial.println("- Check for broken wires or cold solder joints");
  } else {
    Serial.print("\nFound ");
    Serial.print(nDevices);
    Serial.println(" device(s)");
  }
}

/**
 * Alternative I2C Address Reference
 *
 * Common LCD1602 I2C Addresses:
 * ├─ 0x27 (Most common - blue backlight modules)
 * ├─ 0x3F (Some modules - green backlight)
 * ├─ 0x20 (Less common)
 * ├─ 0x38 (Rare)
 * └─ 0x30 (Rare - some Chinese modules)
 *
 * The address is set by:
 * - PCF8574 or PCF8574A chip on the backpack
 * - A0, A1, A2 address pins on the chip
 * - Jumpers or solder bridges on the module
 *
 * PCF8574:  Base address is 0x20, add A0+A1+A2 for offset
 * PCF8574A: Base address is 0x38, add A0+A1+A2 for offset
 *
 * For most users:
 * - PCF8574 with no jumpers set = 0x27
 * - PCF8574A with no jumpers set = 0x3F
 */
