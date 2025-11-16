# ArduiBeeScale with Home Assistant Integration

**Alternative Architecture**: Arduino → MQTT → Home Assistant (No Raspberry Pi needed)

**Project**: ArduiBeeScale
**Date**: 2025-11-16
**Version**: 1.0

---

## 📋 Overview

This document describes an **alternative monitoring architecture** where Home Assistant replaces the Raspberry Pi + Flask setup. Home Assistant provides:

- 🎯 Web dashboard (built-in)
- 📱 Mobile app support
- 🔔 Advanced notifications and alerts
- 📊 Data visualization and history
- ⚙️ Automations and integrations
- 💾 Long-term history statistics

**Advantages**:
- ✅ Single unified platform (no separate Python services)
- ✅ Built-in mobile app (iOS/Android)
- ✅ Advanced automations (alerts, conditions, actions)
- ✅ Better data visualization
- ✅ No Flask/Python knowledge required
- ✅ Easy integration with other smart home devices

**Trade-offs**:
- ⚠️ Requires more powerful device (not suitable for Raspberry Pi Zero W)
- ⚠️ Slightly higher memory footprint
- ⚠️ Steeper learning curve for advanced automations

---

## 🏗️ Architecture Comparison

### Original Architecture (Raspberry Pi + Flask)
```
Arduino + ESP-01
    ↓ WiFi/MQTT
    ↓
Mosquitto MQTT Broker
    ↓
Python MQTT Subscriber
    ↓
SQLite Database
    ↓
Flask Web Server (port 5000)
    ↓
Dashboard (http://raspberrypi.local:5000)
```

### Home Assistant Architecture
```
Arduino + ESP-01
    ↓ WiFi/MQTT
    ↓
Mosquitto MQTT Broker
    ↓
Home Assistant
├─ MQTT Integration
├─ Sensor Entities
├─ Dashboard/Lovelace
├─ Automations & Alerts
└─ History Database
    ↓
Web Dashboard + Mobile App
```

---

## 🛠️ Hardware Requirements

### Option 1: Home Assistant Yellow (Recommended for Simplicity)
- **Cost**: €200-300
- **Power**: USB-C
- **Storage**: 64GB eMMC
- **Performance**: Excellent
- **Setup Time**: 15 minutes (plug and play)
- **Best for**: Users who want easy setup

### Option 2: Raspberry Pi 4 (Recommended for Cost)
- **Cost**: €50-80
- **Power**: USB-C 5V/3A
- **Storage**: microSD 64GB+ recommended
- **Performance**: Good
- **Setup Time**: 45 minutes
- **Best for**: Budget-conscious users

### Option 3: Raspberry Pi 5 (Best Performance)
- **Cost**: €80-120
- **Power**: USB-C 5V/5A
- **Storage**: microSD 128GB+ recommended
- **Performance**: Excellent
- **Setup Time**: 45 minutes
- **Best for**: Power users, many automations

### Option 4: Mini PC/Docker Host (Advanced)
- **Cost**: €100-300
- **Power**: Varies
- **Performance**: Excellent
- **Setup Time**: 1-2 hours
- **Best for**: Advanced users, existing Docker setup

### MQTT Broker (Mosquitto)
- **On same Pi as Home Assistant**: Easiest (add-on)
- **Separate Pi/device**: More flexible
- **Cost**: €0 (software) + hardware

---

## 🚀 Installation Guide

### Step 1: Choose and Install Home Assistant

#### Option A: Home Assistant OS (Recommended)
```bash
# On Raspberry Pi 4 or 5:
1. Download Home Assistant installer from: https://www.home-assistant.io/installation/
2. Write to microSD using Balena Etcher
3. Insert microSD into Pi
4. Power up and wait 5 minutes
5. Access: http://homeassistant.local:8123
6. Follow onboarding wizard
```

#### Option B: Home Assistant Container (Docker)
```bash
# If you have Docker installed:
docker run --init -d \
  --name homeassistant \
  --privileged \
  --restart=unless-stopped \
  -e TZ=Europe/Paris \
  -v /path/to/config:/config \
  -v /run/dbus:/run/dbus:ro \
  --network=host \
  ghcr.io/home-assistant/home-assistant:latest
```

#### Option C: Home Assistant Core (Python)
```bash
# For advanced users with Python environment:
python3 -m pip install homeassistant
hass --config ./config --open-ui
```

### Step 2: Install Mosquitto MQTT Broker

#### Method 1: Home Assistant Add-on (Easiest)
```
Home Assistant → Settings → Add-ons → Add-on Store
Search: "Mosquitto"
Click Install → Configure → Start
```

Configuration for Home Assistant Add-on:
```yaml
logins:
  - username: homeassistant
    password: your_secure_password
```

#### Method 2: Docker Container
```bash
docker run -d \
  --name mosquitto \
  -p 1883:1883 \
  -p 9001:9001 \
  -v /path/to/mosquitto/config:/mosquitto/config \
  -v /path/to/mosquitto/data:/mosquitto/data \
  -v /path/to/mosquitto/log:/mosquitto/log \
  eclipse-mosquitto:latest
```

#### Method 3: Dedicated Pi/Device
```bash
# On separate Raspberry Pi or Linux device:
sudo apt update
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Test:
mosquitto_sub -h localhost -t "test/#"
# In another terminal:
mosquitto_pub -h localhost -t "test/message" -m "Hello"
```

### Step 3: Configure Home Assistant MQTT Integration

#### Step 3a: Add MQTT Integration via UI
```
Home Assistant → Settings → Devices & Services
Click "Create Integration"
Search: "MQTT"
Click "MQTT"
```

**Configuration Form**:
```
Broker: 192.168.1.100  (or your MQTT broker IP)
Port: 1883
Username: homeassistant
Password: your_secure_password
Advanced Options: (keep defaults for now)
```

#### Step 3b: Or Edit configuration.yaml

Edit `/config/configuration.yaml`:

```yaml
mqtt:
  broker: 192.168.1.100
  port: 1883
  client_id: home-assistant
  username: homeassistant
  password: your_secure_password
  discovery: true
  discovery_prefix: homeassistant
```

**Save and Restart Home Assistant**:
```
Home Assistant → Settings → System → Restart
```

---

## 📡 Configure MQTT Sensors

### Method 1: YAML Configuration (Recommended for beginners)

Edit `/config/configuration.yaml` and add:

```yaml
mqtt:
  broker: 192.168.1.100
  # ... (MQTT config from above)

sensor:
  - platform: mqtt
    name: "Température Ruche"
    state_topic: "beehive/hive-001"
    value_template: "{{ value_json.temperature }}"
    unit_of_measurement: "°C"
    icon: mdi:thermometer
    device_class: temperature

  - platform: mqtt
    name: "Humidité Ruche"
    state_topic: "beehive/hive-001"
    value_template: "{{ value_json.humidity }}"
    unit_of_measurement: "%"
    icon: mdi:water-percent
    device_class: humidity

  - platform: mqtt
    name: "Poids Ruche"
    state_topic: "beehive/hive-001"
    value_template: "{{ value_json.weight }}"
    unit_of_measurement: "kg"
    icon: mdi:weight
    device_class: weight

  - platform: mqtt
    name: "Batterie Ruche"
    state_topic: "beehive/hive-001"
    value_template: "{{ value_json.battery_percent }}"
    unit_of_measurement: "%"
    icon: mdi:battery
    device_class: battery

  - platform: mqtt
    name: "Tension Batterie Ruche"
    state_topic: "beehive/hive-001"
    value_template: "{{ value_json.battery_voltage }}"
    unit_of_measurement: "V"
    icon: mdi:flash
```

### Method 2: MQTT Discovery (Automatic)

With Home Assistant MQTT Discovery enabled, Arduino code publishes:

```json
{
  "name": "Température Ruche",
  "state_topic": "beehive/hive-001/temperature",
  "unit_of_measurement": "°C",
  "device_class": "temperature"
}
```

Home Assistant automatically creates entities.

### Method 3: UI Configuration

Home Assistant 2023.5+:
```
Settings → Devices & Services → MQTT → Create New Device
Add sensors via UI form
```

---

## 🎨 Dashboard Configuration (Lovelace)

### Method 1: Visual Editor (Easiest)

```
Home Assistant → Dashboard
Click pencil icon (Edit dashboard)
Click + Add card
Select type and configure
```

### Method 2: YAML Configuration

Edit `/config/ui-lovelace.yaml`:

```yaml
# Lovelace Configuration for ArduiBeeScale

views:
  - title: Ruche 001
    path: ruche-001
    badges: []
    cards:
      # Temperature and Humidity
      - type: entities
        title: Conditions Environnementales
        entities:
          - entity: sensor.temperature_ruche
            name: Température
          - entity: sensor.humidity_ruche
            name: Humidité

      # Weight
      - type: gauge
        entity: sensor.poids_ruche
        min: 0
        max: 100
        unit: kg
        title: Poids Ruche

      # Battery Status
      - type: gauge
        entity: sensor.batterie_ruche
        min: 0
        max: 100
        unit: "%"
        title: État Batterie
        needle: true

      # 24-hour History Graph
      - type: history-graph
        title: Historique 24 Heures
        hours_to_show: 24
        entities:
          - sensor.temperature_ruche
          - sensor.humidity_ruche
          - sensor.poids_ruche

      # Weekly Weight Trend
      - type: statistics-graph
        title: Tendance Poids Hebdomadaire
        entities:
          - sensor.poids_ruche
        period: week
        stat_types:
          - mean
          - max
          - min

      # Status Card
      - type: entities
        title: État Système
        entities:
          - entity: sensor.batterie_ruche
            name: Batterie
          - entity: sensor.tension_batterie_ruche
            name: Tension

      # Last Update
      - type: markdown
        content: |
          Dernière mise à jour : {{ states.sensor.derniere_mise_a_jour.state | default('Jamais') }}
```

---

## 🔔 Alerts and Automations

### Alert 1: High Temperature

```yaml
automation:
  - id: "alert_high_temperature"
    alias: "Alerte Température Élevée Ruche"
    description: "Alerte si température dépasse 35°C"

    trigger:
      platform: numeric_state
      entity_id: sensor.temperature_ruche
      above: 35
      for:
        minutes: 5

    action:
      - service: notify.notify
        data:
          title: "⚠️ Alerte Température Ruche"
          message: "Température élevée: {{ states('sensor.temperature_ruche') }}°C"

      - service: persistent_notification.create
        data:
          title: "Alerte Température"
          message: "Température ruche: {{ states('sensor.temperature_ruche') }}°C (seuil: 35°C)"
          notification_id: "temp_alert"
```

### Alert 2: Low Battery

```yaml
  - id: "alert_low_battery"
    alias: "Alerte Batterie Faible"
    description: "Alerte si batterie <20%"

    trigger:
      platform: numeric_state
      entity_id: sensor.batterie_ruche
      below: 20

    action:
      - service: notify.notify
        data:
          title: "🔋 Batterie Faible"
          message: "Batterie ruche: {{ states('sensor.batterie_ruche') }}%"
```

### Alert 3: No Data Received

```yaml
  - id: "alert_no_data"
    alias: "Alerte Pas de Données"
    description: "Alerte si aucune données depuis 4 heures"

    trigger:
      platform: state
      entity_id: sensor.temperature_ruche
      to: "unavailable"
      for:
        hours: 4

    action:
      - service: notify.notify
        data:
          title: "❌ Pas de Données"
          message: "Aucune données reçues depuis la ruche depuis 4 heures"
```

### Alert 4: Abnormal Weight Change

```yaml
  - id: "alert_weight_change"
    alias: "Alerte Changement Poids Anormal"
    description: "Alerte si perte >5kg en 24h"

    trigger:
      platform: template
      value_template: >
        {% set current = states('sensor.poids_ruche') | float(0) %}
        {% set last_day = state_attr('sensor.poids_ruche', 'last_changed') %}
        {{ current is not none and current < (state_attr('sensor.poids_ruche', 'yesterday_value') | float(0)) - 5 }}

    action:
      - service: notify.notify
        data:
          title: "📊 Perte Poids Importante"
          message: "Perte de poids anormale détectée: {{ states('sensor.poids_ruche') }}kg"
```

### Automation 5: Daily Summary Report

```yaml
  - id: "daily_summary"
    alias: "Rapport Quotidien Ruche"
    description: "Envoie un résumé quotidien à 20h"

    trigger:
      platform: time
      at: "20:00:00"

    action:
      - service: notify.notify
        data:
          title: "📋 Rapport Quotidien Ruche"
          message: |
            **Résumé du jour:**

            🌡️ Température
            - Moyenne: {{ state_attr('sensor.temperature_ruche', 'mean') }}°C
            - Min: {{ state_attr('sensor.temperature_ruche', 'min') }}°C
            - Max: {{ state_attr('sensor.temperature_ruche', 'max') }}°C

            💧 Humidité
            - Moyenne: {{ state_attr('sensor.humidity_ruche', 'mean') }}%

            ⚖️ Poids Ruche
            - Actuel: {{ states('sensor.poids_ruche') }}kg
            - Changement: +{{ state_attr('sensor.poids_ruche', 'change_today') }}kg

            🔋 Batterie: {{ states('sensor.batterie_ruche') }}%
```

---

## 📱 Mobile App Setup

### iOS Setup

1. Install "Home Assistant" from App Store
2. Add home:
   - URL: `http://homeassistant.local:8123`
   - Or: `http://192.168.1.x:8123` (use your HA IP)
3. Accept certificate
4. Log in with your account
5. Create shortcuts for quick access

### Android Setup

1. Install "Home Assistant" from Google Play
2. Add home with same steps as iOS
3. Create app shortcuts for quick access

### Remote Access (Advanced)

For access outside home WiFi:

**Option 1: Home Assistant Cloud (Easiest)**
- Cost: ~€5/month
- Setup: Click "Home Assistant Cloud" in settings
- Access: https://my.home-assistant.io

**Option 2: Nabu Casa (Self-hosted)**
- Cost: Free
- Setup: More complex, requires dynamic DNS

**Option 3: VPN (Most Secure)**
- Cost: Varies
- Setup: Configure VPN on home router
- Access: Via VPN only

---

## 📊 Advanced Configurations

### Template Sensors

Create custom sensors:

```yaml
template:
  - sensor:
      - name: "Température Moyenne Ruche"
        unique_id: "temp_avg_ruche"
        unit_of_measurement: "°C"
        state: >
          {{ ((states('sensor.temperature_ruche') | float(0)) | round(1)) }}

      - name: "Poids Net Ruche"
        unique_id: "poids_net"
        unit_of_measurement: "kg"
        state: >
          {% set current = states('sensor.poids_ruche') | float(0) %}
          {% set baseline = 40 %}
          {{ ((current - baseline) | round(2)) }}
```

### Data Statistics

Track min/max/mean:

```yaml
history_stats:
  temperature_above_30:
    entity_id: sensor.temperature_ruche
    state: ">=30"
    type: time
    period:
      days: 7
    start: "{{ now().replace(hour=0, minute=0, second=0) - timedelta(days=7) }}"
```

---

## 🔒 Security Configuration

### Secure MQTT Broker

Edit `/etc/mosquitto/mosquitto.conf`:

```
listener 1883
protocol mqtt
allow_anonymous false

password_file /etc/mosquitto/passwd

listener 8883
protocol mqtt
certfile /etc/letsencrypt/live/yourdomain.com/cert.pem
keyfile /etc/letsencrypt/live/yourdomain.com/privkey.pem
```

Create password file:

```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd homeassistant
# Enter password when prompted
sudo systemctl restart mosquitto
```

### Home Assistant Security

1. **Enable two-factor authentication**:
   - Settings → Users → Your Account → 2FA

2. **Create restricted user for MQTT**:
   - Settings → Users → Create User
   - Set permissions appropriately
   - Use for MQTT authentication

3. **Set up HTTPS** (for remote access):
   - Generate SSL certificate
   - Update configuration.yaml:
   ```yaml
   http:
     ssl_certificate: /config/ssl/fullchain.pem
     ssl_key: /config/ssl/privkey.pem
   ```

---

## 🔄 Backup and Recovery

### Automatic Backups

Home Assistant → Settings → System → Backups:
```
Click "Create Backup"
Or enable auto-backup:
Settings → System → Backups → Enable schedule
```

### Backup Storage

Store backups to:
- USB drive
- Google Drive (via Google Backup integration)
- Nextcloud
- Network storage (NAS)

### Restore Process

```
Settings → System → Backups
Select backup
Click "Restore"
Wait 5-10 minutes
System restarts automatically
```

---

## ⚙️ Arduino Configuration Update

### Modify Arduino MQTT Topic

The MQTT payload should match Home Assistant expectations:

**Arduino Code** (arduino_wifi_mqtt_lcd.ino):
```cpp
// MQTT Payload format - single topic with JSON
void publishData() {
    char payload[256];
    snprintf(payload, sizeof(payload),
        "{\"temperature\":%.2f,\"humidity\":%.2f,\"weight\":%.2f,\"battery_voltage\":%.2f,\"battery_percent\":%.0f}",
        currentTemp, currentHumidity, currentWeight, batteryVoltage, batteryPercent);

    mqttClient.publish(MQTT_TOPIC, payload);
}

// In configuration:
#define MQTT_TOPIC "beehive/hive-001"  // Home Assistant listens here
```

---

## 📈 Data History and Statistics

### View Historical Data

Home Assistant → Settings → Devices & Services → Statistics:
```
Select sensor
View min/max/mean/median over time periods
Export to CSV
```

### Long-term Statistics

Enable in configuration.yaml:

```yaml
history:
  purge_keep_days: 365  # Keep 1 year
  exclude:
    domains:
      - updater
    entities:
      - sun.sun
```

---

## 🚨 Troubleshooting

### MQTT Connection Issues

**Problem**: "MQTT unavailable"

**Solutions**:
1. Check broker is running:
   ```bash
   sudo systemctl status mosquitto
   ```
2. Verify IP address and port
3. Check firewall allows 1883
4. Verify credentials:
   ```bash
   mosquitto_pub -h 192.168.1.100 -u homeassistant -P password -t "test" -m "test"
   ```

### Sensor Not Receiving Data

**Problem**: Sensor shows "unknown" or "unavailable"

**Solutions**:
1. Verify Arduino publishes to correct topic:
   ```bash
   mosquitto_sub -h localhost -t "beehive/#" -v
   ```
2. Check JSON format matches template
3. Restart Home Assistant
4. Check logs: Settings → System → Logs

### High Memory Usage

**Problem**: Home Assistant consuming lots of RAM

**Solutions**:
1. Reduce history retention:
   ```yaml
   history:
     purge_keep_days: 30  # Reduce from 365
   ```
2. Exclude domains you don't track
3. Disable unused integrations
4. Restart Home Assistant

---

## 📊 Comparison: Raspberry Pi + Flask vs Home Assistant

| Feature | Raspberry Pi + Flask | Home Assistant |
|---------|---|---|
| **Setup Time** | 1-2 hours | 30 minutes |
| **Learning Curve** | Medium | Easy |
| **Web Dashboard** | Basic (custom) | Excellent (built-in) |
| **Mobile App** | No | Yes (iOS/Android) |
| **Automations** | Limited | Advanced |
| **Hardware Cost** | €30-50 (Pi Zero W) | €50-300 |
| **Memory Usage** | Low | Medium |
| **Customization** | High (code) | High (YAML) |
| **Other Integrations** | Manual | Built-in |
| **Community Support** | Moderate | Excellent |
| **Long-term History** | SQLite | Unlimited |

---

## 🎯 Multiple Hives Setup

### Monitor Multiple Hives

Configure multiple MQTT sensors:

```yaml
sensor:
  # Hive 001
  - platform: mqtt
    name: "Température Ruche 001"
    state_topic: "beehive/hive-001"
    value_template: "{{ value_json.temperature }}"

  # Hive 002
  - platform: mqtt
    name: "Température Ruche 002"
    state_topic: "beehive/hive-002"
    value_template: "{{ value_json.temperature }}"

  # Hive 003
  - platform: mqtt
    name: "Température Ruche 003"
    state_topic: "beehive/hive-003"
    value_template: "{{ value_json.temperature }}"
```

Create separate dashboard views for each hive:

```yaml
views:
  - title: Ruche 001
    path: ruche-001
    cards: [...]

  - title: Ruche 002
    path: ruche-002
    cards: [...]

  - title: Comparaison Ruches
    path: comparison
    cards:
      - type: entities
        entities:
          - sensor.temperature_ruche_001
          - sensor.temperature_ruche_002
          - sensor.temperature_ruche_003
```

---

## 🔧 Advanced Customization

### Custom Component for HX711

For better load cell integration, create custom component:

```python
# /config/custom_components/hx711_mqtt/sensor.py
# (Advanced - requires Python knowledge)
```

### Integration with Other Devices

Home Assistant integrates with:
- Smart lights (Philips Hue, LIFX, etc.)
- Weather services (OpenWeatherMap)
- Calendar services (Google Calendar)
- Voice assistants (Alexa, Google Home)
- Energy monitoring (Shelly, Tasmota)

Example - Weather integration:

```yaml
automation:
  - alias: "Alerte Intempéries Prévues"
    trigger:
      platform: template
      value_template: "{{ states('weather.maison') == 'rainy' }}"
    action:
      - service: notify.notify
        data:
          message: "Pluie prévue, vérifier les ruches"
```

---

## 📚 Resources

### Official Documentation
- Home Assistant: https://www.home-assistant.io/docs/
- MQTT Integration: https://www.home-assistant.io/integrations/mqtt/
- Lovelace UI: https://www.home-assistant.io/lovelace/

### Community Resources
- Home Assistant Forums: https://community.home-assistant.io/
- Home Assistant Discord: https://discord.gg/home-assistant
- MQTT Documentation: https://mqtt.org/

### Helpful Tools
- MQTT Explorer: http://mqtt-explorer.com/
- Home Assistant Configuration Check: Built-in validator
- YAML Validator: https://www.yamllint.com/

---

## 🎓 Next Steps

### Immediate (Today)
1. Choose hardware (Yellow, Pi 4, or Pi 5)
2. Install Home Assistant
3. Configure MQTT broker

### Short-term (This Week)
1. Add MQTT integration
2. Create sensor entities
3. Build basic dashboard
4. Test MQTT data flow

### Medium-term (This Month)
1. Set up automations
2. Configure alerts
3. Add mobile app
4. Create multiple dashboards

### Long-term
1. Integrate other smart home devices
2. Set up advanced automations
3. Create custom components
4. Optimize performance

---

## ✅ Checklist: Home Assistant Setup

```
INSTALLATION:
□ Choose hardware
□ Install Home Assistant OS / Core
□ Complete onboarding
□ Create user account

MQTT BROKER:
□ Install Mosquitto (add-on or separate)
□ Configure MQTT broker
□ Create MQTT user credentials
□ Test MQTT connection

HOME ASSISTANT MQTT:
□ Install MQTT integration
□ Configure broker connection
□ Verify connection status
□ Enable MQTT discovery (optional)

SENSORS:
□ Create temperature sensor
□ Create humidity sensor
□ Create weight sensor
□ Create battery sensors
□ Verify sensor data arrival

DASHBOARD:
□ Create dashboard
□ Add sensor cards
□ Add history graphs
□ Add status cards

AUTOMATIONS:
□ Create high temperature alert
□ Create low battery alert
□ Create no-data alert
□ Create daily summary
□ Test automations

MOBILE:
□ Install Home Assistant app
□ Configure remote access
□ Create shortcuts

BACKUP:
□ Create initial backup
□ Enable auto-backup
□ Test restore process
□ Store backup securely
```

---

## 🚀 Deployment

Once everything is configured:

1. Finalize all automations and alerts
2. Create documentation for your setup
3. Test all systems for 24 hours
4. Enable auto-backups
5. Deploy Arduino + sensors
6. Monitor first week closely
7. Adjust automations based on real data

---

**Version**: 1.0
**Date**: 2025-11-16
**Status**: Complete ✅

This alternative architecture provides a more feature-rich, user-friendly solution for monitoring your ArduiBeeScale system!

