# Quick Start - WiFi + MQTT Local Monitoring System

**Project**: ArduiBeeScale with Local MQTT
**Time Required**: 6-8 hours setup + 24-hour deployment test
**Cost**: €71-109 for 1-hive complete system
**Complexity**: Medium (doable for anyone with soldering experience)

---

## 60-Second Overview

You're building a **local beehive monitoring system** that:
- ✅ Runs entirely on your **home WiFi** (no cloud, no fees)
- ✅ Sends data to a **Raspberry Pi** every 2 hours
- ✅ Displays live data on a **web dashboard**
- ✅ Keeps all data **private and offline**
- ✅ Uses ~€15/year in electricity (vs €60-240/year cloud fees)

**Architecture**: Arduino + ESP-01 → WiFi → Mosquitto MQTT → Raspberry Pi → SQLite + Flask → Dashboard

---

## What You Have Now

After the previous work, you have:
- ✅ Critical security fixes implemented
- ✅ Software Sleep power management (€0 cost)
- ✅ Debug logging framework
- ✅ Complete WiFi+MQTT code ready
- ✅ Raspberry Pi setup guide
- ✅ Python services (MQTT subscriber, Flask server)
- ✅ Real-time dashboard
- ✅ Deployment checklist

**Missing**: Hardware procurement and final assembly

---

## 4-Step Implementation Path

### Step 1: Procure Hardware (1-2 days, €71-109)

**BeezScale Hardware** (€41-64):
```
□ Arduino UNO or Mega             €20-30
□ ESP-01 WiFi Module              €3-5
□ 3.3V Voltage Regulator           €1-2
□ Level Shifter TXB0104            €1-3    ← CRITICAL for ESP-01 protection!
□ SHTC3 or DHT22 Sensor            €4-5
□ HX711 + Load Cell                €10-15
□ SoftwareSerial cables            €2-3
□ Total                            €41-64
```

**Server Hardware** (€30-45):
```
□ Raspberry Pi Zero W              €15-20
□ microSD Card 32GB                €8-12
□ USB Power 5V/2A                  €5-8
□ Total                            €28-40
```

**Networking** (€0-10):
```
□ WiFi network (you have this)     €0
□ Ethernet cable (optional)        €2-3
```

**Total System Cost**: €71-109 ✅ (vs €60-240/year cloud)

### Step 2: Set Up Raspberry Pi (2-3 hours)

**On your home computer:**
```bash
# 1. Download Raspberry Pi Imager
#    https://www.raspberrypi.com/software/

# 2. Flash OS to microSD
#    - OS: Raspberry Pi OS Lite
#    - Enable SSH
#    - Set WiFi credentials
#    - Insert microSD

# 3. SSH into Pi
ssh pi@raspberrypi.local
```

**On Raspberry Pi:**
```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install Mosquitto MQTT broker
sudo apt install -y mosquitto mosquitto-clients

# Install Python dependencies
pip3 install paho-mqtt flask flask-socketio flask-cors

# Create project directory
mkdir -p ~/beehive-monitor/templates

# Find and note your Pi's IP address
hostname -I
# Example output: 192.168.1.100
# Use this IP for MQTT_BROKER in Arduino config
```

### Step 3: Deploy Python Services (1-2 hours)

**Copy files to Raspberry Pi:**
```bash
# From your computer:
scp mqtt_subscriber.py pi@raspberrypi.local:~/beehive-monitor/
scp app.py pi@raspberrypi.local:~/beehive-monitor/
scp templates/dashboard.html pi@raspberrypi.local:~/beehive-monitor/templates/
```

**On Raspberry Pi:**
```bash
# Make scripts executable
chmod +x ~/beehive-monitor/mqtt_subscriber.py

# Test MQTT subscriber
python3 ~/beehive-monitor/mqtt_subscriber.py
# Press Ctrl+C to stop

# Test Flask server
cd ~/beehive-monitor
python3 app.py
# Open browser: http://raspberrypi.local:5000
# Press Ctrl+C to stop

# Install as auto-start services
sudo cp beehive-subscriber.service /etc/systemd/system/
sudo cp beehive-web.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable beehive-subscriber
sudo systemctl enable beehive-web
sudo systemctl start beehive-subscriber
sudo systemctl start beehive-web

# Verify services running
sudo systemctl status beehive-subscriber
sudo systemctl status beehive-web
```

### Step 4: Upload Arduino Code & Deploy (2-3 hours)

**Arduino Configuration:**
```cpp
// Create config_wifi.h from config_wifi_template.h
#define WIFI_SSID            "your_ssid"           // ← Update
#define WIFI_PASSWORD        "your_password"       // ← Update
#define MQTT_BROKER          "192.168.1.100"       // ← Use Pi's IP from Step 2
#define MQTT_CLIENT_ID       "beehive-001"
#define MQTT_TOPIC           "beehive/hive-001"
```

**Hardware Wiring (CRITICAL: Use Level Shifter!):**
```
Arduino ↔ Level Shifter ↔ ESP-01 (3.3V)
├─ TX/RX through level shifter (5V→3.3V)
├─ GND shared
└─ ESP-01 VCC from 3.3V regulator (NOT Arduino 5V!)

HX711 Load Cell:
├─ DT → Arduino pin 5
└─ CLK → Arduino pin 6

DHT22 Sensor:
└─ Data → Arduino pin 10

Battery Voltage:
└─ Voltage Divider → Arduino A0
```

**Arduino Upload:**
```
1. Open Arduino IDE
2. Load: arduino_wifi_mqtt.ino
3. Verify & Upload
4. Open Serial Monitor (9600 baud)
5. Should see WiFi+MQTT connection messages
```

**Field Deployment:**
```
1. Place Raspberry Pi at home (on WiFi & power)
2. Power up Arduino system at hive location
3. Open dashboard: http://raspberrypi.local:5000
4. Select hive-001 from dropdown
5. Wait 2 hours for first measurement
6. Should see temperature, humidity, weight, battery
```

---

## Verification Checklist

### Before Deployment
- [ ] Raspberry Pi accessible via SSH
- [ ] Mosquitto running: `sudo systemctl status mosquitto`
- [ ] Python services running: Check dashboard loads
- [ ] Arduino code compiles without errors
- [ ] WiFi and MQTT credentials correct in config

### During First Run
- [ ] Serial Monitor shows WiFi connection
- [ ] Serial Monitor shows MQTT publish success
- [ ] Dashboard shows new data arriving
- [ ] Database file created: `ls ~/beehive-monitor/beehive_data.db`

### After 24-Hour Test
- [ ] 12 measurements (every 2 hours)
- [ ] Dashboard shows historical charts
- [ ] Battery voltage stable
- [ ] No error messages in logs

---

## File Structure

```
Your Computer:
├── arduino_wifi_mqtt.ino         ← New WiFi+MQTT code
├── config_wifi_template.h        ← Configuration template
└── server/
    ├── mqtt_subscriber.py        ← Subscribes to MQTT
    ├── app.py                    ← Flask web server
    ├── beehive-subscriber.service ← Auto-start service
    ├── beehive-web.service       ← Auto-start service
    ├── templates/
    │   └── dashboard.html        ← Real-time dashboard
    └── beehive_data.db           ← SQLite (auto-created)

Raspberry Pi (/home/pi/beehive-monitor/):
├── mqtt_subscriber.py            ← Running as service
├── app.py                        ← Running as service
├── beehive_data.db               ← Data storage
├── templates/
│   └── dashboard.html            ← Web UI
└── backups/
    └── beehive_*.db              ← Backups (optional)
```

---

## Common Issues & Solutions

| Issue | Solution |
|-------|----------|
| "Can't connect to WiFi" | Check SSID/password, signal strength, ESP-01 power |
| "No MQTT connection" | Verify Mosquitto running, check broker IP in config |
| "No data in dashboard" | Check Arduino serial output, verify MQTT topic |
| "Dashboard won't load" | Verify Flask service running: `sudo systemctl status beehive-web` |
| "ESP-01 gets hot/dies" | You're missing the level shifter! 5V will destroy it. |

---

## What Happens Next

After deployment:
1. **Every 2 hours**: Arduino wakes, reads sensors, publishes to MQTT
2. **Immediately**: Python subscriber receives data, stores in SQLite
3. **Real-time**: Flask server broadcasts to dashboard via WebSocket
4. **24/7**: Dashboard displays live data and historical trends
5. **Battery**: Lasts 3+ months on fresh AA batteries

---

## Costs & Timeline

| Item | Time | Cost |
|------|------|------|
| Hardware procurement | 1-2 days | €71-109 |
| Raspberry Pi setup | 2-3 hours | €0 (setup only) |
| Python services deploy | 1-2 hours | €0 (deployment only) |
| Arduino code upload | 1-2 hours | €0 (upload only) |
| 24-hour test | Passive | €0 (testing) |
| **Total** | **6-8 hours** | **€71-109** |

Compared to cloud:
- **Cloud**: €60-240/year + €200+ hardware
- **Local**: €71-109 one-time + €15/year electricity ✅

---

## Next Resources

- **Detailed Setup**: See `RASPBERRY_PI_SETUP_GUIDE.md` (12 phases)
- **Deployment**: See `DEPLOYMENT_CHECKLIST.md` (6 phases)
- **Architecture**: See `LOCAL_MONITORING_ARCHITECTURE.md` (system design)
- **Troubleshooting**: See `DEPLOYMENT_CHECKLIST.md` (Phase 6)

---

## Questions?

For help, refer to:
1. **Arduino WiFi issues**: ESP-01 datasheet, Level Shifter wiring
2. **MQTT not working**: `mosquitto_pub -h localhost -t "test" -m "test"`
3. **Dashboard problems**: `sudo journalctl -u beehive-web -f`
4. **Data not arriving**: `mosquitto_sub -h localhost -t "beehive/#" -v`

---

**Ready to Deploy? Start with Step 1: Hardware Procurement** 🚀

---

**Version**: 1.0
**Last Updated**: 2025-11-16
**Project**: ArduiBeeScale with WiFi + Local MQTT
**Status**: Ready for Implementation ✅
