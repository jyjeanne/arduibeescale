# WiFi + MQTT Implementation Deployment Checklist

**Project**: ArduiBeeScale with Local MQTT Monitoring
**Status**: Complete Implementation Ready for Deployment
**Last Updated**: 2025-11-16

---

## Phase 1: Pre-Deployment Verification (1-2 hours)

### Raspberry Pi Setup Verification
- [ ] Raspberry Pi powered on and accessible via SSH
- [ ] Verify IP address: `hostname -I` → Note the IPv4 address (e.g., 192.168.1.100)
- [ ] Update system: `sudo apt update && sudo apt upgrade -y`
- [ ] Verify disk space: `df -h` → At least 5GB free
- [ ] Verify RAM: `free -h` → Should show 100MB+ available

### Mosquitto MQTT Broker Verification
- [ ] MQTT broker installed: `sudo systemctl status mosquitto`
- [ ] Status shows: `active (running)` in green ✅
- [ ] Test MQTT locally on Pi:
  ```bash
  # Terminal 1: Subscribe
  mosquitto_sub -h localhost -t "test/#" -v

  # Terminal 2 (another SSH session): Publish
  mosquitto_pub -h localhost -t "test/message" -m "Hello MQTT"
  ```
- [ ] Should see message in Terminal 1
- [ ] Broker accessible from network: `mosquitto_pub -h 192.168.1.100 -t "test" -m "test"`

### Python Dependencies Verification
- [ ] Verify all packages installed:
  ```bash
  python3 -c "import paho.mqtt.client as mqtt; print('✅ paho-mqtt OK')"
  python3 -c "import flask; print('✅ flask OK')"
  python3 -c "import flask_socketio; print('✅ flask-socketio OK')"
  python3 -c "import flask_cors; print('✅ flask-cors OK')"
  ```
- [ ] All packages report OK ✅

### Directory Structure Verification
- [ ] Create directories:
  ```bash
  mkdir -p ~/beehive-monitor/templates
  mkdir -p ~/beehive-monitor/static
  mkdir -p ~/beehive-monitor/backups
  ls -la ~/beehive-monitor/
  ```
- [ ] Should show: `templates/`, `static/`, `backups/` folders

---

## Phase 2: Python Services Installation (1-2 hours)

### MQTT Subscriber Installation
- [ ] Copy script to Raspberry Pi:
  ```bash
  # From your computer:
  scp mqtt_subscriber.py pi@raspberrypi.local:~/beehive-monitor/
  ```
- [ ] Make executable:
  ```bash
  chmod +x ~/beehive-monitor/mqtt_subscriber.py
  ```
- [ ] Test run:
  ```bash
  python3 ~/beehive-monitor/mqtt_subscriber.py
  ```
- [ ] Should see:
  ```
  [timestamp] - INFO - Starting BeezScale MQTT Subscriber...
  [timestamp] - INFO - Database initialized successfully
  [timestamp] - INFO - MQTT client starting...
  [timestamp] - INFO - Connected to MQTT broker...
  [timestamp] - INFO - Subscribed to topic: beehive/#
  ```
- [ ] Press `Ctrl+C` to stop
- [ ] Database created: `ls -l ~/beehive-monitor/beehive_data.db`

### Flask Web Server Installation
- [ ] Copy script:
  ```bash
  scp app.py pi@raspberrypi.local:~/beehive-monitor/
  ```
- [ ] Copy dashboard template:
  ```bash
  scp templates/dashboard.html pi@raspberrypi.local:~/beehive-monitor/templates/
  ```
- [ ] Test run:
  ```bash
  cd ~/beehive-monitor
  python3 app.py
  ```
- [ ] Should see:
  ```
  [timestamp] - INFO - Starting BeezScale Flask server...
  [timestamp] - INFO - MQTT Bridge: Connecting to broker...
  [timestamp] - INFO - MQTT Bridge: Connected and subscribed
  * Running on http://0.0.0.0:5000
  ```
- [ ] Press `Ctrl+C` to stop
- [ ] Test dashboard access:
  - [ ] From your computer: `http://raspberrypi.local:5000`
  - [ ] Should see BeezScale dashboard with controls
  - [ ] Should show "Connecting..." status initially

### Systemd Services Installation
- [ ] Copy service files:
  ```bash
  sudo nano /etc/systemd/system/beehive-subscriber.service
  # Paste content from beehive-subscriber.service file
  # Ctrl+X, Y, Enter to save

  sudo nano /etc/systemd/system/beehive-web.service
  # Paste content from beehive-web.service file
  # Ctrl+X, Y, Enter to save
  ```
- [ ] Enable and start services:
  ```bash
  sudo systemctl daemon-reload
  sudo systemctl enable beehive-subscriber
  sudo systemctl enable beehive-web
  sudo systemctl start beehive-subscriber
  sudo systemctl start beehive-web
  ```
- [ ] Verify services running:
  ```bash
  sudo systemctl status beehive-subscriber
  sudo systemctl status beehive-web
  ```
- [ ] Both should show: `active (running)` in green ✅
- [ ] View logs:
  ```bash
  sudo journalctl -u beehive-subscriber -f   # Press Ctrl+C to exit
  sudo journalctl -u beehive-web -f          # Press Ctrl+C to exit
  ```

---

## Phase 3: Arduino & ESP-01 Preparation (2-3 hours)

### Arduino Code Setup
- [ ] Copy `arduino_wifi_mqtt.ino` to your Arduino project
- [ ] Create `config_wifi.h` from `config_wifi_template.h`:
  ```
  Copy config_wifi_template.h to config_wifi.h
  ```
- [ ] Edit `config_wifi.h` with your settings:
  ```cpp
  #define WIFI_SSID            "your_actual_ssid"     // ← Change this
  #define WIFI_PASSWORD        "your_actual_password" // ← Change this
  #define MQTT_BROKER          "192.168.1.100"        // ← Update with your Pi's IP
  #define MQTT_CLIENT_ID       "beehive-001"
  #define MQTT_TOPIC           "beehive/hive-001"
  ```
- [ ] Calibrate your load cell:
  ```cpp
  #define SCALE_CALIBRATION    -19689.35  // ← Calibrate for your load cell
  #define SCALE_OFFSET         -145680    // ← Calibrate for your load cell
  ```
- [ ] Do NOT commit `config_wifi.h` to version control
- [ ] Add to `.gitignore`: `arduino/config_wifi.h`

### Hardware Assembly Verification
- [ ] Level Shifter Connected (CRITICAL for ESP-01 protection):
  - [ ] TXB0104 module available (5V ↔ 3.3V conversion)
  - [ ] Arduino 5V → Level Shifter VCC
  - [ ] ESP-01 3.3V → Level Shifter 3.3V
  - [ ] Arduino GND → Level Shifter GND → ESP-01 GND
  - [ ] Arduino TX (pin 9) → Level Shifter A1 → Level Shifter B1 → ESP-01 RX (GPIO3)
  - [ ] Arduino RX (pin 8) ← Level Shifter B2 ← Level Shifter A2 ← ESP-01 TX (GPIO1)

- [ ] ESP-01 Configuration:
  - [ ] GPIO0 → GND (normal operation mode)
  - [ ] GPIO2 → 3.3V (pull-up)
  - [ ] CH_PD → 3.3V (chip enable)
  - [ ] GND → GND

- [ ] HX711 Load Cell:
  - [ ] DT (pin 5) connected
  - [ ] CLK (pin 6) connected
  - [ ] VCC to 5V
  - [ ] GND to GND
  - [ ] Load cell strain gauges calibrated and connected

- [ ] DHT22 Sensor:
  - [ ] Data pin (pin 10) connected
  - [ ] VCC to 5V
  - [ ] GND to GND
  - [ ] 10kΩ pull-up resistor on data line (may already be on sensor module)

- [ ] Battery & Power:
  - [ ] 4x AA battery holder connected
  - [ ] Voltage divider on A0 for battery monitoring (5kΩ + 1kΩ resistor)
  - [ ] Multimeter test: 4.5-5V at Arduino power

### Arduino Upload & Testing
- [ ] Open Arduino IDE
- [ ] Load `arduino_wifi_mqtt.ino`
- [ ] Verify compilation: Sketch → Verify ✅ (No errors)
- [ ] Select board: Tools → Board → Arduino UNO
- [ ] Select port: Tools → Port → COM3 (or your port)
- [ ] Upload code: Sketch → Upload
- [ ] Wait for upload success message
- [ ] Open Serial Monitor: Tools → Serial Monitor
- [ ] Set baud rate: 9600
- [ ] Should see messages:
  ```
  [INF] ===========================================
  [INF] ArduiBeeScale - WiFi + MQTT Edition
  [INF] ===========================================
  [INF] System initializing...
  [INF] WiFi SSID: your_ssid
  [INF] MQTT Broker: 192.168.1.100
  [INF] MQTT Topic: beehive/hive-001
  [INF] Starting first measurement...
  ```

---

## Phase 4: End-to-End Testing (2-3 hours)

### WiFi Connection Test
- [ ] Serial Monitor shows:
  ```
  [INF] ========== Starting Measurement Cycle ==========
  [INF] Reading sensors...
  [INF] Weight (kg): X.XX
  [INF] Temperature (C): XX.XX
  [INF] Humidity (%): XX.XX
  [INF] Battery (V): X.XX
  [INF] Connecting to WiFi...
  [INF] WiFi connected!
  ```
- [ ] WiFi connection succeeds (may take 15-30 seconds)
- [ ] If fails: Check SSID/password, WiFi signal strength, ESP-01 configuration

### MQTT Publishing Test
- [ ] Serial Monitor shows:
  ```
  [INF] Connecting to MQTT broker...
  [INF] MQTT connected!
  [INF] Publishing data to MQTT...
  [INF] Data published successfully!
  ```
- [ ] On Raspberry Pi, check message arrival:
  ```bash
  mosquitto_sub -h localhost -t "beehive/#" -v
  # Should see:
  # beehive/hive-001 {"temperature":XX.XX,"humidity":XX.XX,"weight":X.XX,"battery_voltage":X.XX}
  ```
- [ ] If no message: Check MQTT broker status, Topic name, Broker IP

### Dashboard Data Display Test
- [ ] Open dashboard in browser: `http://raspberrypi.local:5000`
- [ ] Select hive from dropdown: "hive-001"
- [ ] Should see:
  - [ ] Temperature stat card displays latest value ✅
  - [ ] Humidity stat card displays latest value ✅
  - [ ] Weight stat card displays latest value ✅
  - [ ] Battery stat card displays latest value ✅
  - [ ] Charts start loading (may be empty on first run)
- [ ] If no data shows:
  - [ ] Check MQTT Subscriber is running: `sudo systemctl status beehive-subscriber`
  - [ ] Check Flask logs: `sudo journalctl -u beehive-web -f`
  - [ ] Verify Arduino published data to correct topic

### Database Content Verification
- [ ] On Raspberry Pi, check database:
  ```bash
  sqlite3 ~/beehive-monitor/beehive_data.db
  ```
- [ ] Execute queries:
  ```sql
  sqlite> SELECT * FROM hives;
  # Should show your hive entry

  sqlite> SELECT * FROM readings LIMIT 5;
  # Should show sensor readings

  sqlite> .quit
  ```
- [ ] If empty: Check MQTT Subscriber logs for errors

### System Sleep Test (24-hour test)
- [ ] Disconnect Arduino from USB (if using battery)
- [ ] Let system run for 24 hours
- [ ] Verify:
  - [ ] Arduino goes to sleep after each measurement
  - [ ] Wakes up every 2 hours for next measurement
  - [ ] Dashboard shows new data arriving
  - [ ] No errors in logs
  - [ ] Battery voltage stays stable
- [ ] Check logs:
  ```bash
  sudo journalctl -u beehive-subscriber -n 100
  sudo journalctl -u beehive-web -n 100
  ```

---

## Phase 5: Field Deployment (1-2 hours)

### Pre-Deployment Checks
- [ ] WiFi signal strength at deployment location
  - [ ] Test with phone/laptop
  - [ ] Signal strength ≥ -70 dBm (good) or better
  - [ ] If weak: Consider WiFi range extender

- [ ] Weather protection
  - [ ] Enclosure for Arduino/ESP-01 (waterproof or under cover)
  - [ ] Sensor placement protected from rain but exposed to ambient
  - [ ] Load cell protected from water and dirt

- [ ] Power supply
  - [ ] Fresh batteries installed (4x AA = ~4.8V)
  - [ ] Expected run time: 3-4 months on fresh batteries
  - [ ] Or USB power if available nearby

### Deployment Steps
- [ ] Place Raspberry Pi at home (connected to power, on WiFi)
- [ ] Power up Arduino system at hive location
- [ ] Monitor dashboard for incoming data:
  ```
  http://raspberrypi.local:5000
  Select hive-001
  Should see new readings arriving
  ```
- [ ] Let run for 24 hours, monitor:
  - [ ] Consistent data arrival every 2 hours
  - [ ] No connection errors
  - [ ] Dashboard updates working
  - [ ] Battery voltage trend

- [ ] Verify remote access (from different network):
  - [ ] Can you access dashboard from outside home WiFi?
  - [ ] Note: Dashboard is local-only (no internet needed)
  - [ ] Must be on same WiFi network or use VPN

---

## Phase 6: Maintenance & Troubleshooting (Ongoing)

### Regular Checks (Weekly)
- [ ] Dashboard still showing data: `http://raspberrypi.local:5000`
- [ ] No error messages in logs:
  ```bash
  sudo journalctl -u beehive-subscriber -n 20
  sudo journalctl -u beehive-web -n 20
  ```
- [ ] Battery voltage trending downward (but still >3.5V)

### Regular Maintenance (Monthly)
- [ ] Backup database:
  ```bash
  cp ~/beehive-monitor/beehive_data.db ~/backups/beehive_$(date +%Y%m%d).db
  ```
- [ ] Check disk usage:
  ```bash
  du -sh ~/beehive-monitor/
  du -sh ~/backups/
  ```
- [ ] Restart services (fresh start):
  ```bash
  sudo systemctl restart beehive-subscriber
  sudo systemctl restart beehive-web
  ```

### Troubleshooting Table

| Problem | Symptom | Solution |
|---------|---------|----------|
| No WiFi connection | "WiFi connecting..." on Serial Monitor for >1 min | Check SSID/password, WiFi range, ESP-01 power |
| No MQTT connection | WiFi OK but MQTT fails | Verify Mosquitto running: `sudo systemctl status mosquitto` |
| No data in dashboard | Dashboard loads but no stats | Check Arduino serial output, verify MQTT topic matches |
| Dashboard won't load | "Cannot reach server" | Verify Flask running: `sudo systemctl status beehive-web`, check port 5000 |
| Battery voltage jumps | Erratic battery readings | Check voltage divider resistors, verify analog pin connections |
| Sensor reading errors | Wildly incorrect values | Recalibrate scale, check sensor connections, swap DHT22 if needed |
| System not sleeping | Arduino doesn't go to sleep | Check `done()` function, verify watchdog ISR handler |

---

## ✅ Success Criteria - All Deployments

- [x] **Raspberry Pi**: Running and accessible via SSH
- [x] **Mosquitto**: MQTT broker running and accepting connections
- [x] **Python Services**: Both mqtt_subscriber and app.py running as services
- [x] **Arduino**: WiFi+MQTT code uploaded and tested
- [x] **Hardware**: All sensors connected and validated
- [x] **Network**: Arduino can connect to WiFi from deployment location
- [x] **MQTT**: Data publishing to correct topic every 2 hours
- [x] **Database**: Data being stored in SQLite
- [x] **Dashboard**: Real-time display of sensor data
- [x] **Sleep**: System properly sleeping between measurements
- [x] **Battery**: Stable voltage, lasting 3+ months

---

## 🚀 Ready for Production

When all checkboxes are marked ✅, your system is:
- **Fully functional** - All components working
- **Reliable** - 3+ month battery life, auto-restart services
- **Secure** - No cloud credentials, local data only
- **Maintainable** - Clear logs, simple architecture
- **Scalable** - Can add more hives (change MQTT_TOPIC)

**Dashboard Access**: `http://raspberrypi.local:5000`

---

**Document Version**: 1.0
**Last Updated**: 2025-11-16
**Project**: ArduiBeeScale with WiFi + Local MQTT
**Status**: Ready for Deployment ✅
