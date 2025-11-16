# WiFi Relay 500m Setup - Quick Start Guide

**Project**: ArduiBeeScale Long Range Extension
**Solution**: WiFi Relay + External Antenna (Most Practical)
**Target Range**: 500 meters
**Setup Time**: 3-4 hours
**Cost**: €200

---

## ✅ Quick Decision

**Choose this solution if:**
- ✅ You need 500m range reliably
- ✅ You want minimal code changes
- ✅ You prefer simple hardware
- ✅ Budget ~€200
- ✅ Don't mind placing relay boxes outdoors

**Skip if:**
- ❌ You need 10+ km (use LoRa instead)
- ❌ Extremely budget-conscious (<€100)
- ❌ Can't access relay placement locations
- ❌ Want wireless-only (LoRa better)

---

## 📦 Materials Checklist

### Arduino/Beehive End (€36)
```
□ 9dBi external WiFi antenna        €15
□ SMA male-to-U.FL adapter          €8
□ RG-174 coaxial cable (1m)         €3
□ Antenna mounting bracket          €10
──────────────────────────
Subtotal:                           €36
```

### Relay Equipment (€120-150 for 2 relays)
```
□ TP-Link TL-WA855RE WiFi Extender  €25-30 (×2)
□ USB 5V/2A Power Supply            €10 (×2)
□ Outdoor Weatherproof Box (20cm)   €15 (×2)
□ Mounting Pole or Stand            €10 (×2)
□ Weatherproof connectors           €5
──────────────────────────
Subtotal:                           €120-150
```

### Networking (€10-15)
```
□ Ethernet cable 100m               €10-15
□ Connectors/splitters              €5
──────────────────────────
Subtotal:                           €15
```

**TOTAL: €170-200**

---

## 🎯 Simple 3-Step Solution

### Step 1: Add External Antenna to Arduino (30 min)

**What You're Doing**:
Boosting ESP-01 signal from internal antenna to external 9dBi antenna

**How**:
```
1. Open Arduino enclosure
2. Locate U.FL connector on ESP-01 (tiny circular connector)
3. Snap SMA adapter onto connector
4. Connect coaxial cable
5. Attach external antenna
6. Mount antenna vertically on top of enclosure
7. Close enclosure
```

**Wiring Diagram**:
```
ESP-01 ─ U.FL ─ Coax Cable ─ External Antenna
        Adapter   (30cm)      (vertical mount)
```

**Result**: Range improves from 50m to ~100m

---

### Step 2: Place WiFi Relay #1 at 80-100m (1 hour)

**What You're Doing**:
Setting up first relay to extend signal from Arduino toward home

**Location**:
- Distance from Arduino: 80-100m
- Elevation: 2-3m above ground (on pole)
- Line of sight: Preferable but not required

**How**:

```bash
1. Unbox WiFi Relay (TP-Link TL-WA855RE)
2. Power it on (USB)
3. On phone/laptop, scan WiFi networks
4. Find "TP-LINK_855RE" network
5. Connect to it (password on back: usually "12345678")
6. Open browser: 192.168.0.254
7. Login: admin / admin
8. Configure:
   └─ Settings → Wireless → Site Survey
      └─ Select your home WiFi network (SSID)
      └─ Enter password
      └─ Mode: "Extend" (not "Isolate")
      └─ Save
9. Wait 30 seconds for connection
10. Check WiFi LED (should be solid blue)
11. Place relay in weatherproof box
12. Mount on pole using bracket
13. Position antenna upward
```

**Result**: Range extends from Arduino to ~200m total

---

### Step 3: Place WiFi Relay #2 at 100-150m Further (1 hour)

**What You're Doing**:
Setting up second relay to bridge from first relay to home

**Location**:
- Distance from Relay #1: 100-150m
- Direction: Toward home
- Elevation: 2-3m above ground

**How**:
Same procedure as Relay #1:
1. Unbox, power on
2. Scan WiFi, connect to Relay #1 extended network
3. Configure as repeater
4. Mount in weatherproof box
5. Position antenna upward

**Result**: Range extends from home to ~450-500m total

---

## 🔌 Complete Range Path

```
Arduino + Antenna (€36)
    ↓ WiFi signal
    │ Range: 100m (with antenna)
    │
Relay #1 (€60)
    ↓ Extends and repeats
    │ Range: +150m more
    │
Relay #2 (€60)
    ↓ Extends and repeats
    │ Range: +150m more
    │
Home MQTT Server
    ↓
Dashboard (monitoring)

TOTAL RANGE: ~450-500m
```

---

## 📍 Physical Layout Example

```
                 500m Total Distance
    ┌────────────────────────────────┐

Bee Location                     Home Location
   (hive)                      (Raspberry Pi)
    │                              │
    │ 0m                    ~150m  │
    ├─── Arduino + Antenna  ├─ Home WiFi
    │    (€36)              │  (good signal)
    │    (~100m range)      │
    │                       │
    │ 80m                   100m
    ├─── Relay #1           ├─ Relay #2
    │    (€60)              │  (€60)
    │    (on pole)          │  (on pole)
    │    (extends 150m)     │  (extends home)
    │                       │
    └────────────────────────────────┘

Legend:
① Arduino = Hive location with ESP-01 + antenna
② Relay #1 = Intermediate signal booster
③ Relay #2 = Bridge to home network
④ Home = Raspberry Pi with MQTT broker
```

---

## ⚡ Quick Configuration Reference

### Arduino Side (No code changes needed!)

Your existing Arduino code works as-is:
```cpp
#define MQTT_BROKER "192.168.1.100"  // Home MQTT IP
#define MQTT_TOPIC "beehive/hive-001"

// System automatically finds Relay #1
// Relay #1 automatically extends to Relay #2
// Relay #2 reaches home MQTT broker
// No Arduino code changes required!
```

### Relay #1 Configuration

```
SSID:           [Select your home WiFi network]
Password:       [Your WiFi password]
Mode:           Extend (NOT Isolate)
Channel:        Auto (or 6 if crowded)
Antenna:        Vertical orientation
Power:          USB 5V/2A continuous
```

### Relay #2 Configuration

```
SSID:           [Same as Relay #1 settings]
Password:       [Same WiFi password]
Mode:           Extend (creates chain)
Channel:        Auto
Antenna:        Vertical orientation
Power:          USB 5V/2A continuous
```

### Home MQTT (No changes needed!)

```
Mosquitto running on Raspberry Pi
├─ IP: 192.168.1.100 (or your home Pi IP)
├─ Port: 1883
├─ SSID: Your home WiFi
└─ Connection: Automatic (via Relay #2)
```

---

## 🔍 Verification Checklist

### After Setting Up Antenna

```
□ Arduino boots successfully
□ Serial Monitor shows WiFi connecting message
□ WiFi connection succeeds (look for "WiFi connected!")
□ MQTT publishes messages (check logs)
□ Check RSSI signal strength:
  └─ Expected: -60 to -70 dBm (good)
  └─ Minimum: -75 dBm (still works)
```

### After Setting Up Relay #1

```
□ Relay #1 LED shows blue (connected)
□ Arduino WiFi signal improves (check RSSI)
□ From home, WiFi signal extends to Relay #1 area
□ Test phone at Relay #1 location: Should see WiFi
```

### After Setting Up Relay #2

```
□ Relay #2 LED shows blue (connected)
□ Relay #2 creates bridge to home
□ Arduino WiFi still works
□ MQTT messages arrive at home server
□ Dashboard shows live data
```

### Final System Test

```
□ Arduino publishes data every 2 hours
□ Relay #1 forwards signal
□ Relay #2 bridges to home
□ MQTT broker receives messages
□ Dashboard displays data in real-time
□ Mobile access works (if Home Assistant)
```

---

## 🐛 Troubleshooting Quick Reference

### Arduino Can't Find WiFi

**Symptom**: Arduino stuck on "Connecting to WiFi..."

**Solution**:
```
1. Check antenna is connected properly (wiggle SMA connector)
2. Verify coax cable isn't pinched
3. Check antenna is vertical (not horizontal)
4. Restart Arduino (wait 10 seconds, power on)
5. Move Relay #1 closer if needed
```

### Relay #1 Not Connecting

**Symptom**: Relay LED not blue, can't find relay WiFi

**Solution**:
```
1. Power off/on relay (wait 30 seconds)
2. Check USB power supply (should show LED on relay)
3. Try different USB port/cable
4. Factory reset relay if stuck (hold reset 10 seconds)
5. Reconfigure with default credentials
```

### Data Not Arriving at Dashboard

**Symptom**: Relay connects but no MQTT messages

**Solution**:
```
1. Check Relay #2 connected to home WiFi
2. Verify Relay #2 IP shows on home router
3. Check Mosquitto running on Raspberry Pi
4. Verify Arduino IP in logs (should show 192.168.x.x)
5. Restart relays in sequence (Relay #1 first, wait 30s, then Relay #2)
```

### Intermittent Connection Loss

**Symptom**: Data arrives sometimes, disappears other times

**Solution**:
```
1. Move relays higher (use taller pole)
2. Check line of sight (remove obstacles if possible)
3. Use directional antenna (point at next location)
4. Change WiFi channel (try channel 1, 6, or 11)
5. Add third relay if distance too far
```

---

## 📊 Expected Results

### Signal Strength at Each Point

```
Arduino (with antenna):
├─ RSSI: -65 to -75 dBm (from Relay #1)
└─ Status: ✅ Good, connects successfully

Relay #1 Position:
├─ Signal from Arduino: -70 to -80 dBm
├─ Signal to home: -60 to -70 dBm
└─ Status: ✅ Extends range 150m

Relay #2 Position:
├─ Signal from Relay #1: -70 to -80 dBm
├─ Signal to home: -50 to -65 dBm
└─ Status: ✅ Bridges to home network

Home Reception:
├─ RSSI from Relay #2: -55 to -65 dBm
├─ MQTT messages: Every 2 hours
└─ Status: ✅ Excellent

Overall Range: 450-500m ✅
```

### Data Arrival Pattern

```
Time    Arduino      Relay #1    Relay #2    MQTT
────────────────────────────────────────────────────
00:00   Measuring... Idle        Idle        Idle
        Publishing  Extended    Extended    Receiving
02:00   ✅ Done      ✅ Repost   ✅ Forward  ✅ Stored

04:00   Measuring... Idle        Idle        Idle
        Publishing  Extended    Extended    Receiving
06:00   ✅ Done      ✅ Repost   ✅ Forward  ✅ Stored

Pattern: Every 2 hours, data successfully arrives
Expected: 100% delivery (if configured correctly)
```

---

## 🎯 Success Criteria

You have a working 500m system when:

```
✅ WiFi Signal
   └─ RSSI at Arduino: -70 dBm or better
   └─ RSSI at Relay #1: -75 dBm or better
   └─ RSSI at Relay #2: -65 dBm or better

✅ MQTT Messages
   └─ Arrive every 2 hours (on schedule)
   └─ Contain correct sensor data
   └─ No corruption or lost messages

✅ Dashboard
   └─ Shows live temperature/humidity/weight
   └─ Updates in real-time when new data arrives
   └─ No "unavailable" statuses

✅ Reliability
   └─ 99%+ message delivery over 24 hours
   └─ No spontaneous disconnects
   └─ Works in rain/wind/cold

✅ Range
   └─ Works at 500m+ distance
   └─ Signal stays above -75 dBm
   └─ No dead zones
```

---

## 📝 Setup Timeline

```
Day 1: Preparation (1 hour)
├─ Purchase all materials
├─ Unbox relays and antenna
├─ Review this guide
└─ Plan locations on map

Day 2: Arduino Modification (1-2 hours)
├─ Install antenna on ESP-01 (30 min)
├─ Test Arduino locally (30 min)
├─ Verify MQTT working (30 min)
└─ Prepare enclosure (30 min)

Day 3: Relay Installation (2-3 hours)
├─ Position Relay #1 at 80-100m (60 min)
├─ Configure and test Relay #1 (30 min)
├─ Position Relay #2 at 100m further (60 min)
├─ Configure and test Relay #2 (30 min)
└─ Full system test (30 min)

Day 4: Verification (1-2 hours)
├─ Check signal strength at all points
├─ Monitor MQTT message arrival (2+ hours)
├─ Document configuration
└─ System ready for production

TOTAL TIME: 4-8 hours over 4 days
```

---

## 💡 Pro Tips

**Installation**:
- Install antenna BEFORE placing Arduino at hive (easier to work with)
- Vertical antenna gives best omnidirectional range
- Height matters: 2-3m above ground is ideal

**Relays**:
- Place on poles, not on ground (much better range)
- Keep antenna upright (even 45° loses ~3dB)
- Away from metal buildings (they block signal)
- In shade if possible (heat reduces performance)

**Power**:
- Use quality USB power supplies (not cheap cables)
- Keep power supply in weatherproof box
- Protect cables from weather
- Consider solar panels for remote relays

**Testing**:
- Test each relay independently before final placement
- Monitor RSSI values first 24 hours (for troubleshooting)
- Walk the entire path with WiFi analyzer app
- Document working configuration

---

## 🎓 What to Do If It Doesn't Work

### Signal Strength Poor Everywhere

```
1. Check antenna connections (most common issue)
   └─ Wiggle SMA connector
   └─ Verify coax cable attached

2. Verify antenna orientation
   └─ Should be vertical
   └─ Not bent or damaged

3. Test with phone WiFi analyzer
   └─ Download WiFi Analyzer app
   └─ Check signal strength at each location
   └─ Compare with expected values

4. Move relay closer (temporary test)
   └─ Reduce distance by 20m
   └─ Check if signal improves
   └─ If improves: Place relay further back
   └─ If no improvement: Check antenna
```

### Relay Works Locally But Not at Distance

```
1. Check WiFi channel
   └─ Use WiFi Analyzer to find clear channel
   └─ Reconfigure relay for that channel

2. Reduce WiFi interference
   └─ Move relay away from microwave
   └─ Change to different channel
   └─ Try 5GHz if relay supports

3. Check line of sight
   └─ Remove obstacles if possible
   └─ Use directional antenna (Yagi)
   └─ Test from different angles

4. Add third relay
   └─ If two relays can't bridge gap
   └─ Place intermediate relay
   └─ Creates stronger bridge
```

### MQTT Messages Inconsistent

```
1. Check all relay power supplies
   └─ Each relay needs continuous power
   └─ USB power might be dropping
   └─ Use proper 5V regulated supply

2. Verify Mosquitto running
   └─ SSH into Pi: systemctl status mosquitto
   └─ Should show "active (running)"

3. Check Arduino battery
   └─ Low battery causes WiFi issues
   └─ Monitor battery voltage in logs
   └─ Replace batteries if needed

4. Monitor WiFi connection
   └─ Add RSSI logging to Arduino
   └─ Check logs for "WiFi disconnected"
   └─ Pattern may indicate interference
```

---

## ✅ Final Checklist Before Deployment

```
ARDUINO SIDE:
□ External antenna installed correctly
□ Antenna is vertical
□ All connections solid
□ Arduino boots successfully
□ Logs show "WiFi connected!"
□ MQTT publishes (check with mosquitto_sub)
□ No error messages in logs

RELAY #1:
□ Positioned 80-100m from Arduino
□ 2-3m above ground
□ Antenna upright
□ LED shows solid blue
□ WiFi network visible to phone
□ Power supply secured

RELAY #2:
□ Positioned 100m from Relay #1
□ 2-3m above ground
□ Antenna upright
□ LED shows solid blue
□ Connected to home network
□ Power supply secured

SYSTEM:
□ Arduino can reach Relay #1
□ Relay #1 can reach Relay #2
□ Relay #2 connects to home
□ MQTT messages arrive (check log)
□ Dashboard shows live data
□ No error messages
□ Ran 24-hour test (100% message delivery)

FINAL TEST:
□ Walk entire 500m path
□ Check WiFi coverage continuous
□ Monitor for dead zones
□ Verify RSSI stays above -75 dBm
□ System ready for production!
```

---

## 🎉 You're Done!

Your ArduiBeeScale system now has:
- ✅ 500m WiFi range
- ✅ Reliable MQTT connection
- ✅ Real-time monitoring from home
- ✅ Professional dashboard (if using Home Assistant)
- ✅ Mobile app access (if using Home Assistant)

**Next**: Monitor your beehive remotely and enjoy the data!

---

**Version**: 1.0
**Date**: 2025-11-16
**Status**: Complete ✅

