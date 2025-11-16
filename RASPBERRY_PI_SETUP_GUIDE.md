# Raspberry Pi Setup Guide - Complete Step-by-Step

**Project**: ArduiBeeScale Local MQTT Monitoring
**Target Device**: Raspberry Pi Zero W or Pi 3/4
**OS**: Raspberry Pi OS (Lite or Desktop)
**Status**: Implementation Guide

---

## Prerequisites

### What You Need
- Raspberry Pi Zero W (~€15-20) or Pi 3/4 (~€35-75)
- microSD card (32 GB or larger) (~€8-12)
- USB Power supply (5V, 2A minimum) (~€5-8)
- Network access (WiFi or Ethernet)
- Computer with SD card reader
- 30-60 minutes of setup time

### Optional
- Raspberry Pi case/cooling
- HDMI cable for direct monitoring
- Ethernet cable (for Pi 3/4)

---

## Phase 1: Raspberry Pi Operating System Installation (15 minutes)

### Step 1a: Download Raspberry Pi Imager

Visit: https://www.raspberrypi.com/software/

Download and install on your computer (Windows, Mac, or Linux)

### Step 1b: Flash Raspberry Pi OS to microSD

1. **Open Raspberry Pi Imager**
   - Insert microSD card into computer
   - Launch Raspberry Pi Imager

2. **Select Operating System**
   - Click "CHOOSE OS"
   - Select "Raspberry Pi OS"
   - Choose "Raspberry Pi OS Lite" (minimal, recommended)
   - OR "Raspberry Pi OS" (with desktop GUI)
   - → Use Lite for headless operation (no HDMI needed)

3. **Select Storage**
   - Click "CHOOSE STORAGE"
   - Select your microSD card (careful! choose correct device)

4. **Advanced Settings**
   - Click gear icon (⚙️) for advanced options
   - Set hostname: `beehive-monitor` or `raspberrypi`
   - Enable SSH: ✅ (important!)
   - Set username/password:
     - Username: `pi`
     - Password: `beehive` (change this!)
   - Configure WiFi:
     - SSID: Your WiFi network name
     - Password: Your WiFi password
   - Set locale/timezone

5. **Write Image**
   - Click "WRITE"
   - Wait 5-10 minutes
   - Eject microSD when complete

### Step 1c: First Boot

1. **Insert microSD into Raspberry Pi**
2. **Connect Power**
   - Pi boots automatically
   - Wait 2-3 minutes for first boot
3. **Wait for Setup**
   - OS initializes
   - Creates filesystem
   - Configures settings

---

## Phase 2: Connect to Raspberry Pi (10 minutes)

### Step 2a: Find Your Raspberry Pi's IP Address

**Option 1: Using Hostname (Easiest)**
```bash
# From your computer (Windows: PowerShell, Mac/Linux: Terminal)
ssh pi@raspberrypi.local

# Or with custom hostname:
ssh pi@beehive-monitor.local

# Password: beehive (or what you set above)
```

**Option 2: Using IP Address**
```bash
# From your computer, scan for Raspberry Pi
# Windows:
arp -a

# Mac/Linux:
arp -a | grep -i "b8\|dc"  # Look for Raspberry Pi OUI

# Or check your router's connected devices list
# Usually something like: 192.168.1.50 or 192.168.1.100
```

**Option 3: Check Router Dashboard**
- Log into your WiFi router (usually http://192.168.1.1)
- Look for connected devices
- Find device named "raspberrypi" or "beehive-monitor"
- Note its IP address

### Step 2b: SSH Connection

```bash
# Connect to Raspberry Pi
ssh pi@raspberrypi.local

# Or with IP address:
ssh pi@192.168.1.100

# Enter password when prompted (default: beehive)
```

**Success!** You should see:
```
pi@raspberrypi:~ $
```

---

## Phase 3: System Update (15 minutes)

### Step 3a: Update Package Lists

```bash
sudo apt update

# This downloads the latest package information
# Takes 1-2 minutes
```

### Step 3b: Upgrade Installed Packages

```bash
sudo apt upgrade -y

# This updates all existing packages
# Takes 5-10 minutes depending on what needs updating
# -y flag means "yes" to all prompts
```

### Step 3c: Install Development Tools

```bash
# Install Python development headers
sudo apt install -y python3-dev python3-pip

# Install Git (for code management)
sudo apt install -y git

# Takes 2-5 minutes
```

---

## Phase 4: Mosquitto MQTT Broker Installation (10 minutes)

### Step 4a: Install Mosquitto

```bash
# Install MQTT broker and client tools
sudo apt install -y mosquitto mosquitto-clients

# This installs:
# - mosquitto: The MQTT broker server
# - mosquitto-clients: Command-line tools for testing
```

### Step 4b: Enable Auto-Start

```bash
# Make sure Mosquitto starts automatically on boot
sudo systemctl enable mosquitto

# Start Mosquitto now
sudo systemctl start mosquitto

# Verify it's running
sudo systemctl status mosquitto

# You should see: active (running) in green ✅
```

### Step 4c: Test Mosquitto

```bash
# In a terminal on Raspberry Pi:
# Terminal 1 - Subscribe to test topic
mosquitto_sub -h localhost -t "test/topic" -v

# In another terminal on your computer:
# Terminal 2 - Publish test message
mosquitto_pub -h raspberrypi.local -t "test/topic" -m "Hello MQTT!"

# You should see in Terminal 1:
# test/topic Hello MQTT!
# Success! ✅
```

### Step 4d: Configure Mosquitto (Optional - for Authentication)

**Default Setup** (no authentication):
```bash
# Mosquitto allows anonymous connections
# Works fine for local network
# No changes needed
```

**With Authentication** (more secure):
```bash
# Create password file
sudo mosquitto_passwd -c /etc/mosquitto/passwd beehive

# Enter password when prompted (use something secure)
# Repeat password

# Edit configuration
sudo nano /etc/mosquitto/mosquitto.conf

# Add at the end:
listener 1883
password_file /etc/mosquitto/passwd
allow_anonymous false

# Save with Ctrl+X, then Y, then Enter

# Restart Mosquitto
sudo systemctl restart mosquitto

# Test with authentication
mosquitto_pub -h localhost -u beehive -P <password> -t "test" -m "Test"
```

---

## Phase 5: Python Dependencies (10 minutes)

### Step 5a: Install Required Python Libraries

```bash
# Install Python packages for MQTT and Flask
pip3 install paho-mqtt flask flask-socketio flask-cors

# This installs:
# - paho-mqtt: MQTT client library
# - flask: Web framework
# - flask-socketio: WebSocket support
# - flask-cors: Cross-origin resource sharing

# Takes 3-5 minutes
```

### Step 5b: Verify Installation

```bash
# Test Python imports
python3 -c "import paho.mqtt.client as mqtt; print('✅ paho-mqtt OK')"
python3 -c "import flask; print('✅ flask OK')"
python3 -c "import flask_socketio; print('✅ flask-socketio OK')"

# You should see three ✅ messages
```

---

## Phase 6: Create Project Structure (5 minutes)

### Step 6a: Create Directory

```bash
# Create working directory
mkdir -p ~/beehive-monitor
cd ~/beehive-monitor

# Verify you're in the right place
pwd
# Should show: /home/pi/beehive-monitor
```

### Step 6b: Create Subdirectories

```bash
# Create folders for different components
mkdir -p ~/beehive-monitor/templates
mkdir -p ~/beehive-monitor/static
mkdir -p ~/beehive-monitor/backups

# Verify structure
ls -la ~/beehive-monitor/
# Should show: templates/, static/, backups/ folders
```

---

## Phase 7: Create MQTT Subscriber Script (15 minutes)

### Step 7a: Create Script

```bash
# Create the Python script
nano ~/beehive-monitor/mqtt_subscriber.py
```

### Step 7b: Copy Script Content

Paste the complete `mqtt_subscriber.py` code from the previous message:
- Handles MQTT messages
- Stores data in SQLite database
- Includes error handling and logging

**Key Functions**:
- `init_database()`: Creates SQLite table
- `on_connect()`: Handles connection to Mosquitto
- `on_message()`: Handles incoming MQTT messages
- `main()`: Starts the subscriber

### Step 7c: Make Executable

```bash
# Make the script executable
chmod +x ~/beehive-monitor/mqtt_subscriber.py

# Test run (Ctrl+C to stop)
python3 ~/beehive-monitor/mqtt_subscriber.py

# You should see:
# [timestamp] - INFO - Starting BeezScale MQTT Subscriber...
# [timestamp] - INFO - Database initialized successfully
# [timestamp] - INFO - MQTT client starting...
# [timestamp] - INFO - Connected to MQTT broker...
# [timestamp] - INFO - Subscribed to topic: beehive/#

# Press Ctrl+C to stop
```

---

## Phase 8: Create Flask Web Server (15 minutes)

### Step 8a: Create Flask App

```bash
# Create the Flask application
nano ~/beehive-monitor/app.py
```

### Step 8b: Copy Flask Code

Paste the complete `app.py` code from the previous message:
- Flask web server
- REST API endpoints
- WebSocket support
- MQTT-to-WebSocket bridge

**Key Endpoints**:
- `/` - Serves dashboard HTML
- `/api/hives` - List all hives
- `/api/hive/<id>/latest` - Latest reading
- `/api/hive/<id>/history` - Historical data
- `/api/hive/<id>/stats` - Statistics
- WebSocket: `new_reading` - Real-time updates

### Step 8c: Create Dashboard HTML

```bash
# Create dashboard file
nano ~/beehive-monitor/templates/dashboard.html
```

### Step 8d: Copy Dashboard Code

Paste the complete `templates/dashboard.html` code from the previous message:
- Modern responsive UI
- Chart.js graphs
- Real-time Socket.io updates
- Stat cards and gauges

### Step 8e: Test Flask Server

```bash
# Run Flask server
cd ~/beehive-monitor
python3 app.py

# You should see:
# [timestamp] - INFO - Starting BeezScale Flask server...
# [timestamp] - INFO - MQTT to WebSocket bridge initialized
# * Running on http://0.0.0.0:5000

# Open browser on your computer:
# http://raspberrypi.local:5000
# OR
# http://192.168.1.100:5000

# You should see the BeezScale dashboard! ✅

# Press Ctrl+C to stop
```

---

## Phase 9: Create Systemd Services (Auto-Start) (15 minutes)

### Step 9a: Create MQTT Subscriber Service

```bash
# Create service file
sudo nano /etc/systemd/system/beehive-subscriber.service
```

Paste this content:
```
[Unit]
Description=BeezScale MQTT Subscriber
After=network.target mosquitto.service

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/beehive-monitor
ExecStart=/usr/bin/python3 /home/pi/beehive-monitor/mqtt_subscriber.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Save: Ctrl+X, Y, Enter

### Step 9b: Create Flask Web Service

```bash
# Create service file
sudo nano /etc/systemd/system/beehive-web.service
```

Paste this content:
```
[Unit]
Description=BeezScale Flask Web Server
After=network.target beehive-subscriber.service

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/beehive-monitor
ExecStart=/usr/bin/python3 /home/pi/beehive-monitor/app.py
Restart=always
RestartSec=10
Environment="FLASK_ENV=production"

[Install]
WantedBy=multi-user.target
```

Save: Ctrl+X, Y, Enter

### Step 9c: Enable Services

```bash
# Reload systemd configuration
sudo systemctl daemon-reload

# Enable services to start on boot
sudo systemctl enable beehive-subscriber
sudo systemctl enable beehive-web

# Start services immediately
sudo systemctl start beehive-subscriber
sudo systemctl start beehive-web

# Check status
sudo systemctl status beehive-subscriber
sudo systemctl status beehive-web

# Both should show: active (running) ✅
```

### Step 9d: View Service Logs

```bash
# View MQTT Subscriber logs
sudo journalctl -u beehive-subscriber -f

# Press Ctrl+C to exit

# View Flask Web logs
sudo journalctl -u beehive-web -f

# Press Ctrl+C to exit
```

---

## Phase 10: Find Your Raspberry Pi's IP Address (Permanent)

### Step 10a: Check Current IP

```bash
# Find IP address
hostname -I

# You might see: 192.168.1.100
# Or: 192.168.1.100 fe80::xxxx:xxxx:xxxx:xxxx
# Take the first IPv4 address

# Note this IP! You'll use it for Arduino configuration
```

### Step 10b: Make IP Address Static (Optional but Recommended)

```bash
# Edit DHCP configuration
sudo nano /etc/dhcpcd.conf

# Add at the end:
interface wlan0
static ip_address=192.168.1.100/24
static routers=192.168.1.1
static domain_name_servers=8.8.8.8

# Save: Ctrl+X, Y, Enter

# Reboot for changes to take effect
sudo reboot

# Wait 2 minutes then SSH back:
ssh pi@192.168.1.100
```

---

## Phase 11: Verify Everything is Running

### Step 11a: Check Services Status

```bash
# Check all services
sudo systemctl status mosquitto
sudo systemctl status beehive-subscriber
sudo systemctl status beehive-web

# All should show: active (running) in green ✅
```

### Step 11b: Check Disk Usage

```bash
# See disk space
df -h

# You should have plenty of space on microSD card
# At least 5 GB free
```

### Step 11c: Check Memory Usage

```bash
# See RAM usage
free -h

# Should show something like:
#                 total        used        free
# Mem:            477Mi        120Mi       300Mi
# Good! Plenty of headroom
```

### Step 11d: Access Dashboard from Browser

```
On your computer, open:
http://raspberrypi.local:5000
OR
http://192.168.1.100:5000

You should see:
✅ "BeezScale - Real-time Beehive Monitor" heading
✅ "Select a hive..." dropdown (empty for now)
✅ Connection status indicator (should be green)

Everything working! ✅
```

---

## Phase 12: Next Steps

### For Arduino Configuration
1. Note your Raspberry Pi IP address (e.g., 192.168.1.100)
2. Use this in Arduino configuration:
   ```cpp
   #define MQTT_BROKER       "192.168.1.100"
   #define MQTT_PORT         1883
   #define MQTT_TOPIC        "beehive/hive-001"
   ```
3. Make sure Arduino and Raspberry Pi are on same WiFi network

### For System Maintenance
```bash
# View all logs
sudo journalctl -u beehive-subscriber
sudo journalctl -u beehive-web

# Restart a service
sudo systemctl restart beehive-subscriber
sudo systemctl restart beehive-web

# Stop a service
sudo systemctl stop beehive-subscriber
sudo systemctl stop beehive-web
```

### For Backup
```bash
# Backup database periodically
cp ~/beehive-monitor/beehive_data.db ~/backups/beehive_$(date +%Y%m%d).db

# Add to crontab for automatic backups
crontab -e
# Add: 0 2 * * * cp ~/beehive-monitor/beehive_data.db ~/backups/beehive_$(date +\%Y\%m\%d).db
```

---

## 🎯 Summary

You've successfully set up:

✅ **Raspberry Pi Operating System**
✅ **Mosquitto MQTT Broker**
✅ **Python MQTT Subscriber**
✅ **Flask Web Server**
✅ **Real-time Dashboard**
✅ **Auto-Start Services**

**Status**: Raspberry Pi is ready to receive data from your Arduino! 🚀

**Next**: Configure your Arduino with WiFi + MQTT code (see ARDUINO_WIFI_MQTT_GUIDE.md)

---

## ⚠️ Troubleshooting

### Services Won't Start
```bash
# Check logs
journalctl -u beehive-subscriber -n 50

# Verify Python dependencies
pip3 list | grep -E "paho|flask|socketio"

# Reinstall if needed
pip3 install --upgrade paho-mqtt flask flask-socketio
```

### Can't Access Dashboard
```bash
# Verify Flask is running
ps aux | grep app.py

# Check port 5000 is listening
sudo netstat -tlnp | grep 5000

# Restart Flask
sudo systemctl restart beehive-web

# Check logs
journalctl -u beehive-web -f
```

### MQTT Connection Issues
```bash
# Check Mosquitto
sudo systemctl status mosquitto

# Test MQTT manually
mosquitto_pub -h localhost -t "test" -m "test"
mosquitto_sub -h localhost -t "test"

# Check logs
journalctl -u mosquitto
```

---

**Created**: 2025-11-16
**Status**: Complete Setup Guide
**Next Phase**: Arduino WiFi + MQTT Configuration
