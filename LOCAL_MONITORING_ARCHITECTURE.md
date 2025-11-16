# Local Raspberry Pi Monitoring System - Complete Architecture Guide

**Project**: ArduiBeeScale with Local MQTT Monitoring
**Date**: 2025-11-16
**Architecture**: WiFi + MQTT + Raspberry Pi
**Status**: Planning & Implementation Phase

---

## 🎯 System Overview

Instead of sending data to cloud services (bTree.at), your beehive data will be collected, stored, and displayed on a **local Raspberry Pi** that you control completely.

### System Architecture Diagram

```
┌──────────────────────────────────────────────────────────────────┐
│                    YOUR LOCAL NETWORK (WiFi)                     │
├──────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌─────────────────────┐                ┌──────────────────────┐ │
│  │  Beehive #1         │                │  Beehive #2          │ │
│  │  (ESP-01 + Arduino) │                │  (ESP-01 + Arduino)  │ │
│  │  MQTT Publish       │                │  MQTT Publish        │ │
│  │  → beehive/hive-001 │                │  → beehive/hive-002  │ │
│  └──────────┬──────────┘                └──────────┬───────────┘ │
│             │                                      │               │
│             └──────────────────┬───────────────────┘               │
│                                │                                   │
│                    ┌───────────▼──────────────┐                   │
│                    │   Mosquitto MQTT Broker  │                   │
│                    │   (on Raspberry Pi)      │                   │
│                    │   Port: 1883             │                   │
│                    └───────────┬──────────────┘                   │
│                                │                                   │
│                    ┌───────────▼──────────────┐                   │
│                    │   Python MQTT Subscriber │                   │
│                    │   → SQLite Database      │                   │
│                    │   (/home/pi/beehive_data.db) │               │
│                    └───────────┬──────────────┘                   │
│                                │                                   │
│                    ┌───────────▼──────────────┐                   │
│                    │   Flask Web Server       │                   │
│                    │   + WebSocket (real-time)│                   │
│                    │   http://raspberrypi:5000│                   │
│                    └───────────┬──────────────┘                   │
│                                │                                   │
│                    ┌───────────▼──────────────┐                   │
│                    │  Real-time Dashboard     │                   │
│                    │  Chart.js Visualizations │                   │
│                    │  Modern Web UI           │                   │
│                    └──────────────────────────┘                   │
│                                                                   │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │  Raspberry Pi (Runs Everything)                             │ │
│  │  ├─ Mosquitto MQTT Broker                                  │ │
│  │  ├─ Python MQTT Subscriber                                 │ │
│  │  ├─ Flask Web Server                                       │ │
│  │  └─ SQLite Database                                        │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                                                                   │
│  Access from your phone/laptop:                                  │
│  → http://raspberrypi.local:5000                                │
│  → http://192.168.x.x:5000                                      │
│                                                                   │
└──────────────────────────────────────────────────────────────────┘

INTERNET CONNECTION: NOT REQUIRED ✅
Data stays on your local network!
```

---

## ✅ Key Benefits

### 🔐 Privacy
- ✅ All data stays on your network
- ✅ No cloud uploads
- ✅ No external company has access
- ✅ Complete data ownership

### 💰 Cost
- ✅ Zero monthly fees
- ✅ No cloud subscription
- ✅ One-time Raspberry Pi cost (~€20-40)
- ✅ Pays for itself vs cloud in 3-6 months

### 🔌 Offline Operation
- ✅ Works without internet connection
- ✅ Data stored locally
- ✅ No cloud dependency
- ✅ Continues monitoring during internet outages

### ⚡ Performance
- ✅ Real-time updates (WebSocket, not polling)
- ✅ <100ms latency (vs seconds with cloud)
- ✅ Fast dashboard loading
- ✅ Instant notifications possible

### 📊 Scalability
- ✅ Add dozens of beehives easily
- ✅ Single Raspberry Pi handles 10-20 hives
- ✅ Upgrade to Pi 3/4 for 50-100+ hives
- ✅ Unlimited local storage

---

## 🏗️ Component Architecture

### 1. Beehive Hardware (Arduino + ESP-01)

**Your Beehive System**:
```
Arduino UNO R3
├─ Measures: Temperature, Humidity, Weight
├─ Connects via: WiFi (ESP-01 module)
├─ Communicates: MQTT (not HTTP)
├─ Publishes to: beehive/hive-001
├─ Interval: Every 2 hours (configurable)
└─ Data: {"temperature":25.5,"humidity":60.2,"weight":45.8,"battery_voltage":5.8}
```

**WiFi Connection**:
- ESP-01 connects to your home WiFi network
- Arduino communicates with ESP-01 via SoftwareSerial
- MQTT client publishes data to Raspberry Pi

### 2. Raspberry Pi (MQTT Server + Database + Web Server)

**Runs 3 Services**:

**A) Mosquitto MQTT Broker**
```
- Receives data from beehives via MQTT
- Topic: beehive/hive-001, beehive/hive-002, etc.
- Lightweight (~5 MB RAM)
- Standard port: 1883
```

**B) Python MQTT Subscriber**
```
- Listens to all MQTT messages
- Stores data in SQLite database
- Validates and logs readings
- Handles errors and retries
```

**C) Flask Web Server + WebSocket**
```
- REST API endpoints for data queries
- WebSocket for real-time updates
- Serves dashboard HTML/CSS/JavaScript
- Handles both historical and live data
```

### 3. Database (SQLite)

**Local Storage**:
```
beehive_data.db
├─ Table: beehive_readings
│  ├─ hive_id (which beehive)
│  ├─ timestamp (when measured)
│  ├─ temperature (°C)
│  ├─ humidity (%)
│  ├─ weight (kg)
│  ├─ battery_voltage (V)
│  └─ rssi (WiFi signal strength)
│
├─ Index: hive_id + timestamp
└─ Storage: ~1 MB per hive per year
```

### 4. Dashboard (Web UI)

**Real-time Visualization**:
```
- Modern responsive web interface
- Live sensor readings
- Temperature chart (last 24 hours)
- Weight chart (last 24 hours)
- Statistics (min, max, average)
- Battery status monitoring
- Multi-hive support
- Automatic refresh via WebSocket
```

---

## 📋 Data Flow Example

**When Your Beehive Measures Temperature**:

```
1. Arduino measures sensors
   ↓
2. ESP-01 formats JSON: {"temperature":25.5, "humidity":60.2, ...}
   ↓
3. Arduino publishes via MQTT: TOPIC="beehive/hive-001"
   ↓
4. Mosquitto receives on Raspberry Pi
   ↓
5. Python subscriber reads message
   ↓
6. Data stored in SQLite: beehive_readings table
   ↓
7. Flask web server queries database
   ↓
8. WebSocket broadcasts to all connected browsers
   ↓
9. JavaScript updates dashboard in real-time
   ↓
10. You see live data: "Temperature: 25.5°C" ✅
```

**Total latency**: ~100-200 milliseconds (vs seconds with cloud)

---

## 🔧 Technology Stack

### Arduino/ESP-01 Firmware
```
Language: C++ (Arduino IDE)
Libraries:
  - SoftwareSerial (ESP-01 communication)
  - PubSubClient (MQTT publishing)
  - SHTC3 or DHT22 (sensors)
  - HX711 (load cell)
Protocol: MQTT (lightweight, binary)
```

### Raspberry Pi Services
```
Operating System: Raspberry Pi OS (Linux)
Mosquitto MQTT Broker:
  - Language: C
  - Function: Message broker
  - Lightweight, well-established

Python Services:
  - Python 3.x
  - Libraries: paho-mqtt, flask, flask-socketio
  - Runs as systemd services (auto-restart)

Web Server:
  - Flask (lightweight Python web framework)
  - WebSocket (real-time communication)
  - CORS (cross-origin resource sharing)
```

### Frontend Dashboard
```
HTML5 + CSS3 + JavaScript
Libraries:
  - Chart.js (beautiful charts)
  - Socket.io (WebSocket client)
  - Fetch API (REST requests)
Browser Support: All modern browsers (Chrome, Firefox, Safari, Edge)
```

### Database
```
SQLite3
- Single-file database
- No server needed
- Built into Python
- ~1 MB per hive per year
```

---

## 💻 Hardware Requirements

### Minimum (Recommended)
```
Raspberry Pi Zero W (~€15-20)
├─ CPU: ARMv6 1 GHz
├─ RAM: 512 MB
├─ Storage: microSD 32 GB (~€8-12)
├─ Supports: 10-20 beehives
├─ Power: USB 5V 2A (~€5-8)
└─ Total Cost: €28-40 (one-time)

Runs Everything:
✅ Mosquitto MQTT Broker
✅ Python MQTT Subscriber
✅ Flask Web Server
✅ SQLite Database
✅ Dashboard UI
```

### Recommended for More Hives
```
Raspberry Pi 3 or 4 (~€35-75)
├─ CPU: ARMv7/ARMv8 1.2-1.8 GHz
├─ RAM: 1-4 GB
├─ Storage: microSD 64 GB or SSD
├─ Supports: 50-100+ beehives
├─ Better: Built-in WiFi, Gigabit Ethernet
└─ Total Cost: €45-95 (one-time)

Upgrade later if needed!
```

---

## 🚀 Quick Start Path

### Step 1: Hardware Setup (2-3 hours)
```
☐ Order Raspberry Pi + microSD + power supply
☐ Flash Raspberry Pi OS to microSD
☐ Connect to network (WiFi or Ethernet)
☐ SSH into Raspberry Pi
```

### Step 2: Mosquitto Installation (30 minutes)
```
☐ Update system (apt update)
☐ Install Mosquitto broker
☐ Configure for external connections
☐ Test MQTT publishing
```

### Step 3: Python Services (1 hour)
```
☐ Install Python dependencies
☐ Create MQTT subscriber script
☐ Create Flask web server
☐ Set up systemd services
```

### Step 4: Arduino Firmware (2-3 hours)
```
☐ Order ESP-01 + level shifter + regulator (~€5-8)
☐ Receive components
☐ Wire ESP-01 to Arduino
☐ Update Arduino code for WiFi + MQTT
☐ Configure WiFi credentials
☐ Configure MQTT broker IP
☐ Upload and test
```

### Step 5: Dashboard (30 minutes)
```
☐ Create dashboard HTML/CSS/JavaScript
☐ Place in Flask templates folder
☐ Open browser: http://raspberrypi.local:5000
☐ View real-time data ✅
```

**Total Implementation Time: ~6-8 hours**
**Cost: €28-40 (Raspberry Pi) + €25-38 (Arduino/ESP-01)**
**Monthly Cost: €0 (vs €5-20 with cloud)**

---

## 📊 Comparison: Local vs Cloud

```
╔═════════════════════════════════════════════════════════╗
║     LOCAL MQTT vs CLOUD SERVICES COMPARISON             ║
╠═════════════════════════════════════════════════════════╣
║                                                         ║
║  COST                                                   ║
║  ├─ Local MQTT:  €28-40 (one-time)                     ║
║  └─ Cloud:       €60-240/year (monthly fees)           ║
║     Savings:     €20-200/year ✅                        ║
║                                                         ║
║  PRIVACY                                                ║
║  ├─ Local MQTT:  100% data ownership ✅                 ║
║  └─ Cloud:       Company has your data                 ║
║                                                         ║
║  PERFORMANCE                                            ║
║  ├─ Local MQTT:  ~100-200ms latency ✅                  ║
║  └─ Cloud:       1-5+ seconds (network dependent)      ║
║                                                         ║
║  OFFLINE OPERATION                                      ║
║  ├─ Local MQTT:  Works without internet ✅              ║
║  └─ Cloud:       Requires internet connection          ║
║                                                         ║
║  SCALABILITY                                            ║
║  ├─ Local MQTT:  Easy to add hives ✅                   ║
║  └─ Cloud:       May increase costs                    ║
║                                                         ║
║  CUSTOMIZATION                                          ║
║  ├─ Local MQTT:  Full control ✅                        ║
║  └─ Cloud:       Limited by service provider           ║
║                                                         ║
║  SETUP COMPLEXITY                                       ║
║  ├─ Local MQTT:  Moderate (6-8 hours) ✅                ║
║  └─ Cloud:       Easy (30 min) but less control        ║
║                                                         ║
╚═════════════════════════════════════════════════════════╝
```

---

## 🎯 System Capabilities

### Real-Time Monitoring
- ✅ Live temperature, humidity, weight updates
- ✅ WebSocket instant notifications
- ✅ Battery voltage monitoring
- ✅ WiFi signal strength (RSSI) tracking

### Historical Data Analysis
- ✅ 24-hour trending charts
- ✅ Weekly/monthly statistics
- ✅ Min/max/average calculations
- ✅ Weight change detection
- ✅ Temperature pattern analysis

### Multi-Hive Support
- ✅ Monitor 1 to 100+ beehives
- ✅ Individual hive dashboards
- ✅ Comparative hive analysis
- ✅ Batch operations

### Data Management
- ✅ Automatic data logging (every 2 hours)
- ✅ Local SQLite storage
- ✅ Backup functionality
- ✅ Data export (CSV, JSON)
- ✅ Retention policies (auto-cleanup)

### System Monitoring
- ✅ Beehive connectivity status
- ✅ Battery voltage alerts
- ✅ Temperature anomalies
- ✅ System health dashboard
- ✅ Logging and debugging

---

## 🔐 Security Considerations

### Network Security
```
Option 1: Closed Network (Most Secure)
├─ Raspberry Pi only accessible from local WiFi
├─ No external internet access
├─ Mosquitto: no authentication required
├─ Best for: Home networks only

Option 2: Optional Password Protection
├─ Mosquitto with authentication
├─ Default: username/password
├─ MQTT client must provide credentials
├─ Better for: Shared networks

Option 3: Advanced Security (Optional)
├─ VPN access to Raspberry Pi
├─ SSL/TLS encryption
├─ Firewall rules
├─ Best for: Remote monitoring (external network)
```

### Data Protection
```
✅ Data stays on your network (no cloud upload)
✅ SQLite database is local
✅ Backup your database regularly
✅ microSD card is your responsibility
✅ No encryption by default (local only)
```

---

## 📈 Performance Expectations

### System Resources (Raspberry Pi Zero W)

```
Service                 RAM      CPU      Disk
─────────────────────────────────────────────
Mosquitto             5 MB    <1%      10 MB
Python Subscriber    15 MB    <1%      - MB
Flask Web Server     30 MB    5%       - MB
SQLite Database      Variable  <1%      ~1 MB/year/hive
WebSocket Clients    10 MB   <1%      - MB (per client)
─────────────────────────────────────────────
TOTAL              ~60 MB    ~6%      Expandable
Available          512 MB   99%      Plenty!
```

### Scalability
```
1-5 Beehives:     Raspberry Pi Zero W (€20)
5-20 Beehives:    Raspberry Pi Zero W (€20) - upgrade later
20-50 Beehives:   Raspberry Pi 3 (€35)
50-100+ Beehives: Raspberry Pi 4 (€75)
```

### Network Traffic
```
Per Hive, Per Reading:
├─ MQTT Publish: ~150 bytes
├─ Frequency: Every 2 hours (12 readings/day)
├─ Daily: ~1.8 KB/day per hive
├─ Monthly: ~54 KB/month per hive
├─ Yearly: ~650 KB/year per hive

10 Beehives:
├─ Daily: ~18 KB/day
├─ Monthly: ~0.54 MB/month
└─ Yearly: ~6.5 MB/year
→ Negligible network impact! ✅
```

---

## 🛠️ Maintenance & Operations

### Automatic Management
```
✅ Systemd services auto-restart on failure
✅ Mosquitto auto-starts on boot
✅ Python subscriber auto-starts on boot
✅ Flask web server auto-starts on boot
✅ No manual intervention needed (usually)
```

### Monitoring & Logging
```
View Mosquitto logs:
  sudo journalctl -u mosquitto -f

View Python subscriber logs:
  sudo journalctl -u beehive-subscriber -f

View Flask web server logs:
  sudo journalctl -u beehive-web -f

View system status:
  ps aux | grep python
  ps aux | grep mosquitto
```

### Database Maintenance
```
Backup database:
  cp ~/beehive-monitor/beehive_data.db ~/backup_$(date +%Y%m%d).db

Clean old data (>90 days):
  sqlite3 ~/beehive-monitor/beehive_data.db
  DELETE FROM beehive_readings WHERE timestamp < datetime('now','-90 days');

Export to CSV:
  sqlite3 -header -csv ~/beehive-monitor/beehive_data.db \
    "SELECT * FROM beehive_readings" > export.csv
```

### Regular Tasks
```
Daily:
  ☐ Check dashboard for new readings
  ☐ Monitor battery voltages

Weekly:
  ☐ Verify system is logging data
  ☐ Check Raspberry Pi disk space

Monthly:
  ☐ Backup database
  ☐ Review system logs
  ☐ Update Raspberry Pi OS (optional)

Quarterly:
  ☐ Clean database (remove old data if needed)
  ☐ Verify backups are working
  ☐ Update software packages (optional)
```

---

## 🎓 Learning Outcomes

By implementing this system, you'll learn:

### Hardware
- ✅ Arduino programming
- ✅ WiFi module configuration (ESP-01)
- ✅ Sensor integration and calibration
- ✅ Power management and battery optimization

### Networking
- ✅ MQTT protocol and pub/sub messaging
- ✅ WiFi networking and configuration
- ✅ Network security basics
- ✅ IP networking and DNS

### Software Development
- ✅ Python programming
- ✅ Flask web framework
- ✅ WebSocket real-time communication
- ✅ JavaScript and modern web development

### DevOps
- ✅ Linux/Raspberry Pi OS administration
- ✅ Systemd service management
- ✅ Process monitoring and logging
- ✅ Database management (SQLite)

### Data Science
- ✅ Data collection and logging
- ✅ Time-series data analysis
- ✅ Data visualization with Chart.js
- ✅ Statistical analysis (min/max/average)

---

## 🚀 Next Steps

### Phase 1: Plan (1-2 days)
- [ ] Read this entire document
- [ ] Review hardware requirements
- [ ] Plan your network setup
- [ ] Decide: Raspberry Pi Zero W or Pi 3/4?
- [ ] Set timeline for implementation

### Phase 2: Procure (3-7 days, depending on shipping)
- [ ] Order Raspberry Pi + microSD
- [ ] Order Arduino/ESP-01 WiFi components
- [ ] Order sensors (SHTC3 or keep DHT22)
- [ ] Gather tools and cables

### Phase 3: Build (1-2 days)
- [ ] Set up Raspberry Pi
- [ ] Install Mosquitto
- [ ] Install Python dependencies
- [ ] Wire Arduino + ESP-01
- [ ] Update Arduino firmware

### Phase 4: Deploy (1 day)
- [ ] Test MQTT communication
- [ ] Verify dashboard loading
- [ ] Configure WiFi credentials
- [ ] Deploy to beehive location
- [ ] Monitor first 24 hours

### Phase 5: Optimize (Ongoing)
- [ ] Fine-tune measurement intervals
- [ ] Add alerts/notifications
- [ ] Expand to more beehives
- [ ] Add data export features

---

## ❓ Common Questions

**Q: Do I need internet access?**
A: No! Everything runs locally on your Raspberry Pi. Internet is optional.

**Q: Can I access the dashboard from outside my home?**
A: Yes, if you configure VPN or port forwarding (advanced). But it's not necessary.

**Q: How much storage do I need?**
A: ~1 MB per hive per year. A 32 GB microSD holds 32,000 years of data for one hive!

**Q: What if the Raspberry Pi crashes?**
A: Services auto-restart. Data is safely stored in SQLite. Very resilient.

**Q: Can I add more hives later?**
A: Yes! Just configure the new Arduino+ESP-01 with a new hive_id and it publishes to the same Mosquitto broker.

**Q: Is it hard to set up?**
A: Moderate difficulty. Following the guides, most people can do it in 6-8 hours.

---

## 📚 Documentation Files

This architecture includes comprehensive guides:

1. **LOCAL_MONITORING_ARCHITECTURE.md** (this file)
   - System overview and design
   - Technology stack explanation
   - Performance and scalability

2. **RASPBERRY_PI_SETUP_COMPLETE.md**
   - Step-by-step Raspberry Pi installation
   - Mosquitto configuration
   - Python service setup
   - Systemd service files

3. **ARDUINO_WIFI_MQTT_GUIDE.md**
   - Arduino firmware for WiFi + MQTT
   - ESP-01 wiring and configuration
   - Code with detailed comments
   - Testing procedures

4. **FLASK_DASHBOARD_SETUP.md**
   - Flask web server installation
   - WebSocket configuration
   - REST API endpoints
   - Dashboard HTML/CSS/JavaScript

5. **TESTING_AND_DEPLOYMENT.md**
   - System testing checklist
   - Deployment procedures
   - Troubleshooting guide
   - Performance monitoring

---

## 🎉 Summary

You're building a **complete, private, local monitoring system** that:

- ✅ Costs €50-80 one-time (vs €60-240/year for cloud)
- ✅ Stores data locally (100% privacy)
- ✅ Works offline (no internet dependency)
- ✅ Scales easily (add more beehives anytime)
- ✅ Provides real-time updates (WebSocket)
- ✅ Is fully customizable (you control everything)
- ✅ Requires 6-8 hours initial setup
- ✅ Needs minimal maintenance afterward

**Status: Ready to Begin Implementation** 🚀

Choose your path:
- **Path A**: Jump to Raspberry Pi setup (Phase 3)
- **Path B**: Deep dive into architecture details (Phase 2)
- **Path C**: Review hardware requirements first (Phase 1)

---

**Created**: 2025-11-16
**System**: ArduiBeeScale with Local MQTT Monitoring
**Architecture**: WiFi (ESP-01) + MQTT (Mosquitto) + Python + Flask
**Status**: Complete Architecture Guide - Ready for Implementation
