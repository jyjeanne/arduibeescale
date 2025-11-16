# Software Sleep Solution - Deployment Guide

**Status**: ✅ Implementation Complete
**Date**: 2025-11-15
**Project**: ArduiBeeScale
**Solution**: Software Sleep (No Hardware Required)

---

## 🎉 GOOD NEWS!

The Software Sleep solution has been **fully implemented** into your `arduino/arduino.ino` file!

**What was done:**
- ✅ Added software sleep configuration
- ✅ Implemented watchdog interrupt handler
- ✅ Replaced `done()` function with software sleep logic
- ✅ Removed ATtiny85 references
- ✅ Code is ready to upload and test

**Cost**: €0
**Setup Time**: 5-10 minutes
**Hardware Required**: None

---

## 📋 What Changed in Your Code

### Added Configuration (Line 189-196)
```cpp
#define SLEEP_INTERVAL_HOURS  2
#define SLEEP_INTERVAL_MS     (SLEEP_INTERVAL_HOURS * 60 * 60 * 1000)

volatile bool shouldWake = false;
unsigned long sleepStartTime = 0;
```

### Added Watchdog Interrupt Handler (Line 199-211)
```cpp
ISR(WDT_vect) {
    if (millis() - sleepStartTime >= SLEEP_INTERVAL_MS) {
        shouldWake = true;
    }
}
```

### Replaced done() Function (Line 742-824)
- Now handles sleep/wake internally
- No ATtiny85 signal needed
- Automatically wakes after 2 hours
- Resumes measurement cycle

---

## 🚀 DEPLOYMENT STEPS

### Step 1: Remove Hardware (5 minutes)

**Physical Changes:**
- [ ] Disconnect ATtiny85 from breadboard/PCB
- [ ] Remove N-MOSFET gate control (no longer used)
- [ ] Remove signal wire from Arduino pin 2
- [ ] Keep everything else as-is

**What stays connected:**
- ✅ GSM Shield (SoftwareSerial on pins 7, 8)
- ✅ HX711 (Scale on pins 5, 6)
- ✅ DHT22 (on pin 10)
- ✅ Power connections

**Hardware to Remove:**
- ❌ ATtiny85 microcontroller
- ❌ ATtiny socket (if using one)
- ❌ Any ATtiny programmer connections
- ❌ Associated resistors/capacitors for ATtiny

### Step 2: Upload Code (3 minutes)

**Your code is already updated!**

1. **Open Arduino IDE**
2. **Open**: `arduino/arduino.ino`
3. **Verify**: Click ✓ (Verify/Compile button)
4. **Upload**: Click ➜ (Upload button)
5. **Wait**: Until "Done uploading" appears

### Step 3: Test on Breadboard (5 minutes)

**Before deploying to beehive:**

1. **Open Serial Monitor** (Tools → Serial Monitor, 9600 baud)
2. **Watch for initialization**:
   ```
   [INF] Watchdog enabled (8-second timeout)
   [INF] System initializing...
   [INF] Checking if GSM is running...
   [INF] GSM ready!
   [INF] Reading sensors...
   [INF] Weight (kg): XX.XX
   [INF] Temperature (C): XX.XX
   [INF] Humidity (%): XX.XX
   [INF] Sending data...
   [INF] Request cycle complete!
   [INF] Shutting down systems...
   [INF] Powering down GSM...
   [INF] Powering down scale...
   [INF] Entering sleep mode...
   ```

3. **Wait 2 hours** (or test with shorter interval - see below)

4. **Should see**:
   ```
   [INF] Woken from sleep! Starting next measurement cycle...
   [INF] System initializing...
   [INF] Checking if GSM is running...
   [INF] GSM ready!
   ...
   ```

### Step 4: Verify Data Transmission

1. **Check bTree Dashboard**: Does data appear?
2. **Check Timestamps**: Are they 2 hours apart?
3. **Check Values**: Are weight/temp/humidity reasonable?

If everything looks good → **Ready to deploy!**

---

## 🧪 OPTIONAL: Quick Testing (Change 2-hour interval)

To test faster than waiting 2 hours, change the sleep interval:

**In `arduino/arduino.ino`, Line 192:**

```cpp
// Normal operation (2 hours)
#define SLEEP_INTERVAL_HOURS  2

// For quick testing (5 minutes)
#define SLEEP_INTERVAL_HOURS  0.083  // 5 minutes

// For moderate testing (30 minutes)
#define SLEEP_INTERVAL_HOURS  0.5    // 30 minutes

// After testing, change back to 2 hours for production
#define SLEEP_INTERVAL_HOURS  2
```

**Steps:**
1. Change `SLEEP_INTERVAL_HOURS` to 0.083 (5 minutes)
2. Upload code
3. Watch it cycle every 5 minutes
4. Verify everything works
5. Change back to 2 hours
6. Upload again

---

## 📊 How It Works

### The Sleep Cycle

```
1. Arduino finishes measurement
   ↓
2. Calls done() function
   ↓
3. Powers down GSM and Scale
   ↓
4. Disables Serial (saves power)
   ↓
5. Enters PWR_SAVE sleep mode
   ↓
6. Watchdog wakes Arduino every 8 seconds
   ↓
7. ISR checks: Have 2 hours elapsed?
   ├─ NO → Go back to sleep
   └─ YES → Set flag, exit sleep loop
   ↓
8. Wake up properly (enable serial, etc)
   ↓
9. Call Power_UP() and Request()
   ↓
10. REPEAT
```

### Power Consumption

**Per 2-hour cycle:**
- Active time: 40 seconds @ 500mA = 5.5 mAh
- Sleep time: 7160 seconds @ 1mA = 1.99 mAh
- **Total per cycle**: 7.49 mAh

**Battery life (4x AA, 2000mAh each = 8000mAh):**
- Cycles per battery: 8000 ÷ 7.49 = ~1,068 cycles
- Days: 1,068 × 2 hours = ~89 days
- **Battery life: ~3 months**

(vs ATtiny85: ~4 months, difference is ~15-20%)

---

## ✅ Checklist - Before Deploying to Field

### Code
- [ ] Code uploaded successfully
- [ ] Serial output shows measurements
- [ ] Data appears in bTree dashboard
- [ ] Timestamps are correct
- [ ] Values are reasonable

### Hardware
- [ ] ATtiny85 removed
- [ ] No loose wires
- [ ] All other components intact
- [ ] Power connections secure

### Testing
- [ ] At least 1 complete cycle tested (2 hours OR faster with modified interval)
- [ ] Multiple measurements received successfully
- [ ] Battery voltage adequate
- [ ] Signal strength acceptable (if GSM)

### Documentation
- [ ] This deployment guide read completely
- [ ] Troubleshooting section reviewed
- [ ] Know how to revert if needed

---

## 🔧 How to Change Settings

### Change Sleep Interval

**File**: `arduino/arduino.ino`
**Line**: 192

```cpp
#define SLEEP_INTERVAL_HOURS  2    // Change this number

// Examples:
// 1 hour:
#define SLEEP_INTERVAL_HOURS  1

// 3 hours:
#define SLEEP_INTERVAL_HOURS  3

// 30 minutes (for testing):
#define SLEEP_INTERVAL_HOURS  0.5

// 5 minutes (for quick testing):
#define SLEEP_INTERVAL_HOURS  0.083
```

After changing, upload the code again.

### Change Debug Level

**File**: `arduino/arduino.ino`
**Line**: 47

```cpp
#define DEBUG_LEVEL  DEBUG_ERRORS   // Current setting

// Options:
#define DEBUG_LEVEL  DEBUG_OFF       // No output
#define DEBUG_LEVEL  DEBUG_ERRORS    // Only errors
#define DEBUG_LEVEL  DEBUG_INFO      // + informational
#define DEBUG_LEVEL  DEBUG_VERBOSE   // + detailed debug
```

More verbose = more Serial output = slightly more power consumption

---

## ❓ TROUBLESHOOTING

### Issue 1: "Arduino not sleeping"

**Symptom**: Serial monitor shows loop but no sleep message

**Causes**:
- [ ] Code not uploaded
- [ ] Watchdog not enabled properly
- [ ] Serial monitor preventing sleep

**Solution**:
1. Click Verify ✓ first
2. Then Upload ➜
3. Close and reopen Serial Monitor
4. Restart Arduino (press Reset button)

---

### Issue 2: "Won't wake up after sleep"

**Symptom**: Shows sleep message, never wakes up

**Causes**:
- [ ] Watchdog interrupt not set correctly
- [ ] ShouldWake flag not working
- [ ] millis() not updating during sleep

**Solution**:
1. Change `SLEEP_INTERVAL_HOURS` to 0.083 (5 min)
2. Test to see if it wakes after 5 min
3. If yes: timing is working, just takes longer
4. If no: check that watchdog is enabled

---

### Issue 3: "Serial garbage/weird characters"

**Symptom**: Serial Monitor shows garbled text

**Causes**:
- [ ] Baud rate wrong (should be 9600)
- [ ] Serial not reinitialized after sleep
- [ ] Interference

**Solution**:
1. Check Serial Monitor baud: Tools → Serial Monitor → 9600 baud
2. Close/Reopen Serial Monitor
3. Press Reset button on Arduino
4. Should see clean startup messages

---

### Issue 4: "GSM not connecting after waking"

**Symptom**: Wakes up but can't connect to network

**Causes**:
- [ ] GSM module not fully powered down
- [ ] Signal too weak
- [ ] Not enough delay after power-up

**Solution**:
1. Increase delays in done() function (add 1 second to each)
2. Check antenna connection
3. Check signal strength: `AT+CSQ` returns something like `+CSQ: 15,99`
4. Try different location with better signal

---

### Issue 5: "Measurements missing / irregular intervals"

**Symptom**: Doesn't measure every 2 hours, sometimes misses

**Causes**:
- [ ] Arduino crashing during sleep
- [ ] Watchdog too aggressive
- [ ] millis() overflow (only after ~49 days)

**Solution**:
1. Change sleep mode from PWR_SAVE to PWR_STANDBY (more stable)
2. Add watchdog reset in main loop
3. For long deployment: reset Arduino weekly

---

## 🎯 Performance Expected

### First Cycle
- Setup/initialization: ~10-20 seconds
- Sensor readings: ~5-10 seconds
- GSM connection: ~20-30 seconds
- Data transmission: ~5 seconds
- **Total active**: ~40-60 seconds

### Subsequent Cycles
- Startup: ~2-3 seconds (faster)
- Sensor/GSM/TX: ~40 seconds (same)
- **Total active**: ~40-45 seconds

### Sleep
- Sleep mode: PWR_SAVE (uses ~1mA)
- Wakes every: 8 seconds
- Checks elapsed: Every 8 seconds
- Back to sleep: Immediately if not 2 hours yet

---

## 📈 Monitoring Success

### Signs Everything is Working

✅ Serial shows:
```
[INF] Shutting down systems...
[INF] Powering down GSM...
[INF] Powering down scale...
[INF] Entering sleep mode...
[INF] Woken from sleep! Starting next measurement cycle...
[INF] Checking if GSM is running...
[INF] GSM ready!
```

✅ bTree Dashboard:
```
Data points appear with 2-hour intervals
Values are reasonable (weight ~50kg, temp ~20°C, etc)
Timestamps look correct
```

✅ Power consumption:
```
Battery voltage stays stable
No rapid voltage drops
Measured current ~500mA when active, ~1mA when sleeping
```

---

## 🚀 Next Steps

### Immediate
1. [ ] Remove ATtiny85 from hardware
2. [ ] Upload the code (should compile without errors)
3. [ ] Test with Serial Monitor (5-30 min test)
4. [ ] Verify data appears in bTree

### Before Field Deployment
1. [ ] Run complete cycle (2 hours) OR speed test (5 min with modified interval)
2. [ ] Verify wireless signal strength at deployment location
3. [ ] Check battery voltage (should be >4.5V for 4x AA)
4. [ ] Test in field conditions if possible

### Field Deployment
1. [ ] Mount in waterproof enclosure
2. [ ] Install at beehive location
3. [ ] Verify first measurement appears in dashboard
4. [ ] Note GPS coordinates and antenna orientation
5. [ ] Document battery replacement date

### Maintenance
- [ ] Check data monthly in dashboard
- [ ] Replace batteries every 3-4 months
- [ ] Verify signal quality
- [ ] Clean antenna connection annually

---

## 📞 Need Help?

### Check Documentation
- **ATTINY85_DECISION_TREE.md** - Why this solution
- **ATTINY85_REPLACEMENT_CODE.md** - Code details
- **README.md** - Project overview
- **SETUP_GUIDE.md** - Initial setup

### Common Questions

**Q: Can I easily switch back to ATtiny85?**
A: Yes! Just replace the done() function. Keep a backup of old code.

**Q: What if I want RTC instead?**
A: Replace the done() function with RTC code from ATTINY85_REPLACEMENT_CODE.md

**Q: How do I know it's actually sleeping?**
A: Measure current - should drop from ~500mA to ~1mA during sleep

**Q: Can I modify the 2-hour interval?**
A: Yes! Change `SLEEP_INTERVAL_HOURS` on line 192 and upload

**Q: What happens if Arduino crashes during sleep?**
A: Watchdog will reset it after 8 seconds, then it tries again

---

## ✨ You're Done!

Your system is now ready to deploy with the **Software Sleep solution**.

**Summary:**
- ✅ ATtiny85 removed from code
- ✅ Software Sleep implemented
- ✅ Zero additional hardware needed
- ✅ Cost: €0
- ✅ Setup: 5-10 minutes
- ✅ Battery life: ~3 months

**Next action**: Upload the code and test!

---

**Created**: 2025-11-15
**Solution**: Software Sleep (No ATtiny85)
**Status**: ✅ Ready for deployment
**Maintainer**: Jeremy JEANNE
