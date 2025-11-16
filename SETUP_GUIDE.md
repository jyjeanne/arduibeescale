# bScale Setup Guide - With Critical Fixes

This guide walks you through setting up the bScale project with the new critical security and reliability fixes.

---

## Quick Start

### 1. Clone/Download the Project
```bash
git clone <your-repo-url>
cd bScale
```

### 2. Set Up Configuration (IMPORTANT! 🔐)

#### Step 1: Copy the template
```bash
cd arduino
cp config_template.h config.h
```

#### Step 2: Edit config.h with your credentials
Open `arduino/config.h` in your text editor and fill in:

```cpp
// Your API key from https://beta.btree.at/setting/profile/api
const char API_KEY[] PROGMEM = "YOUR_ACTUAL_API_KEY_HERE";

// Your cellular provider's APN
// Examples:
// - Vodafone Austria: "vodafone.at"
// - HOT/T-Mobile Austria: "webaut"
// - A1: "A1.net"
const char APN_CONFIG[] PROGMEM = "webaut";

// Device identifier (must match one created in bTree dashboard)
const char DEVICE_IDENT[] PROGMEM = "ScaleHouse";

// Timezone for server timestamps (PHP format)
const char TIMEZONE_CONFIG[] PROGMEM = "Europe/Vienna";
```

**⚠️ CRITICAL SECURITY NOTE:**
- `config.h` is **NOT tracked by git** (see `.gitignore`)
- Never share or commit `config.h`
- Treat it like a password file

### 3. Verify Arduino IDE Setup

1. **Install required libraries** (Arduino IDE → Sketch → Include Library → Manage Libraries):
   - HX711 (Bogde's library)
   - DHT (Adafruit DHT)
   - Adafruit Unified Sensor

2. **Select Board**: Arduino → Tools → Board → Arduino UNO

3. **Select Port**: Arduino → Tools → Port → COMx (your Arduino port)

### 4. Upload Code

1. Open `arduino/arduino.ino` in Arduino IDE
2. Click **Verify** (checkmark) to compile
3. Click **Upload** (arrow) to upload to board
4. Open **Serial Monitor** (Tools → Serial Monitor) at 9600 baud

### 5. First Run Diagnostics

When the Arduino boots, you should see serial output like:

```
Checking if GSM is running...
GSM ready!
Checking signal strength...
...
Connecting ....
Setup finished!
Reading DHT22 sensor...
Weight: 25.45
Temp: 22.50
Humidity: 65.30
Sending: weight=25.45&temp1=22.50&temp2=0.00&hum=65.30&rain=0.00
Request cycle complete!
Shutting down systems...
Entering sleep mode...
```

---

## Configuration Files Explained

### `config.h` - User Credentials (YOUR FILE)
- **Location**: `arduino/config.h`
- **Created from**: `arduino/config_template.h`
- **Should contain**: Your actual API key and credentials
- **Git status**: ❌ NOT tracked (see `.gitignore`)
- **Sharing**: ⚠️ NEVER share this file

### `config_template.h` - Setup Instructions (Provided)
- **Location**: `arduino/config_template.h`
- **Purpose**: Template with comments and examples
- **Git status**: ✅ Tracked (safe to share)
- **What to do**: Copy to `config.h` and customize

### `.gitignore` - Git Ignore Rules
- **Updated to exclude**: `arduino/config.h`
- **Prevents accidental commits** of sensitive files
- **Includes**: Build artifacts, IDE files, temp files

---

## What Was Fixed?

### Fix #1: Stack Overflow Risk
**Problem**: System could crash due to unbounded recursion on network failures.

**Solution**: Replaced recursive calls with proper retry logic:
- Maximum 3 retries per operation
- Graceful failure handling
- Clear error messages

**Impact**: More reliable in poor network conditions.

### Fix #2: API Key Security
**Problem**: API key was hardcoded in source code, risking exposure via version control.

**Solution**:
- Moved credentials to `config.h`
- `config.h` is git-ignored
- Safe to push to public repositories

**Impact**: Protected against accidental key exposure.

### Fix #3: Buffer Safety
**Problem**: String buffers weren't null-terminated, causing potential crashes.

**Solution**:
- Always null-terminate after each character
- Proper buffer overflow detection
- Larger buffers for URL construction

**Impact**: No more undefined behavior from string operations.

---

## Troubleshooting

### Issue: "fatal error: config.h: No such file or directory"
**Solution**: You must create `config.h` from `config_template.h`:
```bash
cd arduino
cp config_template.h config.h
```

### Issue: "GSM not responding"
**Check:**
1. SIM900 module is powered
2. Antenna is connected to SIM900
3. SIM card is inserted and has credit
4. RX/TX pins are correct (7, 8)

### Issue: "Failed to read from DHT sensor"
**Check:**
1. DHT22 is wired correctly
2. Pull-up resistor (4.7k) is present
3. Data pin is correct (pin 10)
4. DHT library is installed

### Issue: "API returns 401 Unauthorized"
**Check:**
1. API key in `config.h` is correct
2. API key is active in bTree dashboard
3. Device identifier matches bTree device name
4. No extra spaces in config values

### Issue: "Could not get IP address"
**Check:**
1. APN in `config.h` is correct for your provider
2. GSM has signal (check with AT+CSQ command)
3. GPRS plan is active on your SIM

---

## Hardware Pin Configuration

If you need to change pins, edit these lines in `arduino.ino`:

```cpp
#define GSM_POWER_PIN       9     // GSM Shield power toggle
#define HX711_DOUT_PIN      5     // Scale data pin
#define HX711_CLK_PIN       6     // Scale clock pin
#define DHTPIN              10    // Temperature sensor
#define FINISHED            2     // ATtiny completion signal
```

---

## Power Calibration

Before first use, calibrate the scale:

1. Upload `calibrate/calibrate.ino` to Arduino
2. Open Serial Monitor at 9600 baud
3. Follow the on-screen prompts
4. Record the `SCALE` and `offset` values
5. Edit these values in `arduino.ino`:
   ```cpp
   float SCALE = -19689.35;      // Your calibrated scale
   long offset = -145680;         // Your calibrated offset
   ```
6. Upload `arduino/arduino.ino` to Arduino

---

## API Integration

### Send Data to Your Own Server

To send data to a custom endpoint instead of bTree:

1. Edit `arduino/config.h`:
   ```cpp
   const char API_URL[] PROGMEM = "your-server.com";
   const char API_PATH[] PROGMEM = "/api/v1/scale";
   ```

2. Your server will receive HTTP GET request like:
   ```
   http://your-server.com/api/v1/scale/ScaleHouse?API_KEY&CREATE&weight=25.45&temp1=22.50&temp2=0.00&hum=65.30&rain=0.00
   ```

3. Implement a handler to receive and store the data.

### Testing with RequestBin

To test without bTree:

1. Create a RequestBin at https://requestbin.com
2. Copy the unique URL
3. Edit `arduino/config.h`:
   ```cpp
   const char API_URL[] PROGMEM = "requestbin.com";
   const char API_PATH[] PROGMEM = "/your-unique-url";
   ```

---

## Security Best Practices

1. **Protect Your API Key**
   - Treat `config.h` like a password
   - Don't share it via email or chat
   - Rotate keys if you suspect exposure

2. **Version Control Safety**
   - Always check `.gitignore` excludes `config.h`
   - Before pushing, verify with: `git status`
   - Never commit `config.h`

3. **Network Security**
   - Use HTTPS when possible (SIM900 limitations may apply)
   - Validate server certificates if possible
   - Consider firewall rules on your API endpoint

4. **Device Security**
   - Keep Arduino in secure location
   - Encrypt SIM card with PIN if possible
   - Monitor data usage for unauthorized activity

---

## Performance Notes

- **Power Consumption**: ~500mA active, <10µA sleep (depends on ATtiny interval)
- **Data Per Request**: ~100-150 bytes
- **Request Frequency**: Configurable via ATtiny sleep cycles (default: 2 hours)
- **Battery Life**: 6-12 months with 4x AA batteries (varies by configuration)

---

## Next Steps

1. ✅ Configure `config.h`
2. ✅ Upload and test
3. ⏳ (Coming soon) Add watchdog timer for reliability
4. ⏳ (Coming soon) Implement data validation
5. ⏳ (Coming soon) Add debug logging modes

---

## Support

For issues with:
- **Sensor calibration**: See `CALIBRATION.md`
- **Code architecture**: See `ARCHITECTURE.md`
- **Critical fixes**: See `CRITICAL_FIXES_SUMMARY.md`
- **API integration**: See bTree documentation at https://beta.btree.at

---

## Version Info

- **Project**: bScale
- **Critical Fixes**: v1.1
- **Date**: November 2025
- **Status**: Pre-production (use with caution)

---

**Last Updated**: 2025-11-15
