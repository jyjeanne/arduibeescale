# Home Assistant Complete Setup Guide

Complete guide for setting up Home Assistant dashboards, charts, and email alerts for ArduiBeeScale ESP32.

---

## Quick Setup Checklist

Follow these steps in order:

1. [ ] Configure email notifications (Gmail/SMTP)
2. [ ] Setup weather integration (OpenWeatherMap - free)
3. [ ] Create helper entities (thresholds, toggles)
4. [ ] Add dashboard cards
5. [ ] Import alert automations
6. [ ] Test alerts work

**Estimated time**: 30-60 minutes

---

## Table of Contents

1. [Email Notification Setup](#1-email-notification-setup)
2. [Weather Integration Setup](#2-weather-integration-setup)
3. [Helper Entities (User Settings)](#3-helper-entities-user-settings)
4. [Dashboard Configuration](#4-dashboard-configuration)
5. [Alert Automations](#5-alert-automations)
6. [Advanced Configurations](#6-advanced-configurations)

---

## 1. Email Notification Setup

Before creating alerts, you need to configure email notifications in Home Assistant.

### Option A: SMTP Email (Gmail, Outlook, etc.)

1. **Edit `configuration.yaml`** (via File Editor add-on or SSH):

```yaml
# Add to configuration.yaml
notify:
  - name: email_alert
    platform: smtp
    server: smtp.gmail.com           # Gmail SMTP server
    port: 587
    timeout: 15
    sender: your_email@gmail.com     # Your email address
    encryption: starttls
    username: your_email@gmail.com   # Your email address
    password: !secret gmail_app_password  # App password (see below)
    recipient:
      - recipient1@email.com         # Alert recipient(s)
      - recipient2@email.com         # Add more if needed
    sender_name: "Beehive Monitor"
```

2. **Create Gmail App Password** (required if using Gmail):
   - Go to: https://myaccount.google.com/apppasswords
   - Sign in to your Google account
   - Select "Mail" and "Other (Custom name)"
   - Enter "Home Assistant" and click Generate
   - Copy the 16-character password

3. **Add password to `secrets.yaml`**:

```yaml
# In secrets.yaml
gmail_app_password: "xxxx xxxx xxxx xxxx"
```

4. **Restart Home Assistant**:
   - Go to: Developer Tools → Restart → Restart

5. **Test email notification**:
   - Go to: Developer Tools → Services
   - Service: `notify.email_alert`
   - Service data:
     ```yaml
     title: "Test Alert"
     message: "This is a test email from Home Assistant"
     ```
   - Click "Call Service"

### Option B: Using SendGrid (More Reliable)

```yaml
notify:
  - name: email_alert
    platform: sendgrid
    api_key: !secret sendgrid_api_key
    sender: beehive@yourdomain.com
    recipient: your_email@gmail.com
```

### Option C: Mobile Push Notifications (Alternative)

If email is not required, use the Home Assistant Companion app:

```yaml
# No configuration needed - install HA Companion app on your phone
# Service: notify.mobile_app_your_phone_name
```

---

## 2. Weather Integration Setup

To receive storm alerts, you need a weather integration that fetches data from a **free external weather API** based on your local city.

### Recommended: OpenWeatherMap (Free API with City Search)

OpenWeatherMap is the best option for city-based weather with a generous free tier.

**Free Tier Limits:**
- 1,000 API calls/day (plenty for Home Assistant)
- Current weather + 5-day/3-hour forecast
- City search by name or coordinates
- No credit card required

**Setup Steps:**

1. **Create free OpenWeatherMap account**:
   - Go to: https://openweathermap.org/api
   - Click **"Sign Up"** (top right)
   - Verify your email
   - Go to: https://home.openweathermap.org/api_keys
   - Copy your API key (it may take 10-30 minutes to activate)

2. **Add integration in Home Assistant**:
   - Go to: **Settings → Devices & Services**
   - Click **+ Add Integration**
   - Search for **"OpenWeatherMap"**
   - Enter your **API key**
   - Enter your **city name** (e.g., "Paris", "Lyon, FR", "Berlin, DE")
   - Select **"Free"** mode
   - Click **Submit**

3. **Add API key to `secrets.yaml`** (recommended):
   ```yaml
   # In secrets.yaml
   openweathermap_api_key: "your_api_key_here"
   ```

4. **Verify entity created**:
   - Go to: **Developer Tools → States**
   - Search for `weather.openweathermap`
   - You should see current condition and attributes

### Alternative: Open-Meteo (100% Free, No API Key)

Open-Meteo is completely free with no registration required.

**Features:**
- No API key needed
- No rate limits for personal use
- 7-day forecast
- Requires HACS (Home Assistant Community Store)

**Setup Steps:**

1. **Install HACS** (if not already):
   - Follow: https://hacs.xyz/docs/setup/download

2. **Install Open-Meteo integration**:
   - Go to: **HACS → Integrations → + Explore & Download**
   - Search for **"Open-Meteo"**
   - Click **Download**
   - Restart Home Assistant

3. **Add integration**:
   - Go to: **Settings → Devices & Services → + Add Integration**
   - Search for **"Open-Meteo"**
   - Enter your **city name** or coordinates
   - Click **Submit**

### Alternative: Met.no (Free, No API Key)

Met.no (Norwegian Meteorological Institute) is built into Home Assistant.

**Features:**
- No API key required
- European weather data (works worldwide)
- Automatically configured during HA onboarding

**Setup Steps:**

1. Go to: **Settings → Devices & Services**
2. Click **+ Add Integration**
3. Search for **"Meteorologisk institutt (Met.no)"**
4. Enter your **location name** and **coordinates**
5. Click **Submit**

> **Note**: Met.no uses coordinates, not city names. You can get coordinates from Google Maps.

### Comparison of Free Weather APIs

| API | API Key | City Search | Rate Limit | Forecast |
|-----|---------|-------------|------------|----------|
| **OpenWeatherMap** | Yes (free) | Yes | 1,000/day | 5 days |
| **Open-Meteo** | No | Yes | Unlimited* | 7 days |
| **Met.no** | No | Coordinates | Fair use | 10 days |

*Open-Meteo: unlimited for non-commercial use

### Verifying Weather Entity

After setup, check your weather entity:

1. Go to: **Developer Tools → States**
2. Search for `weather.`
3. Note your entity ID (e.g., `weather.home`, `weather.openweathermap`)
4. Check available attributes:
   - `condition` - current weather (sunny, rainy, lightning, etc.)
   - `wind_speed` - wind speed
   - `forecast` - upcoming conditions

### Weather Conditions Reference

Home Assistant uses these standard condition values:

| Condition | Description | Storm Risk |
|-----------|-------------|------------|
| `clear-night` | Clear night sky | Low |
| `cloudy` | Overcast | Low |
| `fog` | Foggy | Low |
| `hail` | Hail | **HIGH** |
| `lightning` | Lightning/Thunder | **HIGH** |
| `lightning-rainy` | Thunderstorm | **HIGH** |
| `partlycloudy` | Partly cloudy | Low |
| `pouring` | Heavy rain | Medium |
| `rainy` | Rain | Low |
| `snowy` | Snow | Medium |
| `snowy-rainy` | Sleet | Medium |
| `sunny` | Sunny | Low |
| `windy` | Windy | Medium |
| `windy-variant` | Wind + clouds | Medium |
| `exceptional` | Exceptional/Severe | **HIGH** |

---

## 3. Helper Entities (User Settings)

Create helper entities to allow users to set alert thresholds via the UI.

### Method 1: Via UI (Recommended)

1. Go to: **Settings → Devices & Services → Helpers**
2. Click **+ Create Helper**
3. Create these helpers:

#### Weight Minimum Threshold

- Type: **Number**
- Name: `Beehive Minimum Weight`
- Icon: `mdi:scale-balance`
- Minimum value: `0`
- Maximum value: `100`
- Step: `0.5`
- Unit: `kg`
- Initial value: `15`

#### Temperature Minimum Threshold

- Type: **Number**
- Name: `Beehive Minimum Temperature`
- Icon: `mdi:thermometer-alert`
- Minimum value: `0`
- Maximum value: `50`
- Step: `1`
- Unit: `°C`
- Initial value: `27`

#### Enable/Disable Alerts

- Type: **Toggle**
- Name: `Beehive Alerts Enabled`
- Icon: `mdi:bell`

### Method 2: Via configuration.yaml

Add to `configuration.yaml`:

```yaml
# Input Number helpers for user-configurable thresholds
input_number:
  beehive_min_weight:
    name: "Beehive Minimum Weight"
    icon: mdi:scale-balance
    min: 0
    max: 100
    step: 0.5
    unit_of_measurement: "kg"
    initial: 15

  beehive_min_temperature:
    name: "Beehive Minimum Temperature"
    icon: mdi:thermometer-alert
    min: 0
    max: 50
    step: 1
    unit_of_measurement: "°C"
    initial: 27

  beehive_alert_cooldown:
    name: "Alert Cooldown (hours)"
    icon: mdi:timer
    min: 1
    max: 24
    step: 1
    unit_of_measurement: "h"
    initial: 6

# Input Boolean for enabling/disabling alerts
input_boolean:
  beehive_alerts_enabled:
    name: "Beehive Alerts Enabled"
    icon: mdi:bell
    initial: true

# Input Datetime to track last alert time
input_datetime:
  beehive_last_alert:
    name: "Last Beehive Alert"
    has_date: true
    has_time: true

  beehive_last_storm_alert:
    name: "Last Storm Alert"
    has_date: true
    has_time: true
```

#### Storm Alert Helpers (Add these for weather alerts)

Via UI (Settings → Helpers):

- **Toggle**: `Storm Alerts Enabled`
  - Icon: `mdi:weather-lightning`

- **Number**: `Storm Alert Wind Threshold`
  - Min: 20, Max: 100, Step: 5
  - Unit: km/h
  - Initial: 50

Or add to `configuration.yaml`:

```yaml
input_boolean:
  storm_alerts_enabled:
    name: "Storm Alerts Enabled"
    icon: mdi:weather-lightning
    initial: true

input_number:
  storm_wind_threshold:
    name: "Storm Wind Threshold"
    icon: mdi:weather-windy
    min: 20
    max: 100
    step: 5
    unit_of_measurement: "km/h"
    initial: 50
```

Restart Home Assistant after adding.

---

## 4. Dashboard Configuration

### Complete Dashboard YAML

Create a new dashboard or add to existing one.

1. Go to: **Overview → (3 dots menu) → Edit Dashboard**
2. Click **+ Add Card → Manual**
3. Paste the configurations below

### 3.1 Status Overview Card

```yaml
type: entities
title: "🐝 Beehive Status"
show_header_toggle: false
entities:
  - entity: sensor.beehive_1_weight
    name: Weight
    icon: mdi:scale
  - entity: sensor.beehive_1_temperature
    name: Temperature
  - entity: sensor.beehive_1_humidity
    name: Humidity
  - entity: sensor.beehive_1_battery
    name: Battery
    icon: mdi:battery
  - entity: sensor.beehive_1_wifi_signal
    name: WiFi Signal
    icon: mdi:wifi
  - type: divider
  - entity: input_boolean.beehive_alerts_enabled
    name: Alerts Enabled
  - entity: input_number.beehive_min_weight
    name: Min Weight Threshold
  - entity: input_number.beehive_min_temperature
    name: Min Temp Threshold
```

### 3.2 Gauge Cards Row

```yaml
type: horizontal-stack
cards:
  - type: gauge
    entity: sensor.beehive_1_weight
    name: Weight
    unit: kg
    min: 0
    max: 80
    severity:
      green: 20
      yellow: 40
      red: 60
    needle: true
  - type: gauge
    entity: sensor.beehive_1_temperature
    name: Temperature
    unit: °C
    min: 0
    max: 50
    severity:
      red: 0
      yellow: 20
      green: 27
    needle: true
  - type: gauge
    entity: sensor.beehive_1_battery
    name: Battery
    unit: "%"
    min: 0
    max: 100
    severity:
      red: 20
      yellow: 40
      green: 60
    needle: true
```

### 3.3 Temperature History Chart (7 Days)

```yaml
type: history-graph
title: "🌡️ Temperature History (7 Days)"
entities:
  - entity: sensor.beehive_1_temperature
    name: Temperature
hours_to_show: 168
refresh_interval: 300
```

### 3.4 Humidity History Chart (7 Days)

```yaml
type: history-graph
title: "💧 Humidity History (7 Days)"
entities:
  - entity: sensor.beehive_1_humidity
    name: Humidity
hours_to_show: 168
refresh_interval: 300
```

### 3.5 Weight History Chart (30 Days)

```yaml
type: history-graph
title: "⚖️ Weight History (30 Days)"
entities:
  - entity: sensor.beehive_1_weight
    name: Weight
hours_to_show: 720
refresh_interval: 300
```

### 3.6 Combined Chart (ApexCharts - Advanced)

For better charts, install **ApexCharts Card** from HACS:

1. Install HACS: https://hacs.xyz/docs/setup/download
2. Go to: HACS → Frontend → Search "ApexCharts Card" → Install
3. Restart Home Assistant

```yaml
type: custom:apexcharts-card
header:
  show: true
  title: "Beehive Monitoring"
  show_states: true
  colorize_states: true
graph_span: 7d
span:
  start: day
  offset: "-7d"
yaxis:
  - id: temp
    min: 0
    max: 50
    apex_config:
      tickAmount: 5
  - id: weight
    opposite: true
    min: 0
    max: 100
    apex_config:
      tickAmount: 5
series:
  - entity: sensor.beehive_1_temperature
    name: Temperature
    yaxis_id: temp
    color: "#ff6b6b"
    stroke_width: 2
    group_by:
      func: avg
      duration: 2h
  - entity: sensor.beehive_1_humidity
    name: Humidity
    yaxis_id: temp
    color: "#4ecdc4"
    stroke_width: 2
    group_by:
      func: avg
      duration: 2h
  - entity: sensor.beehive_1_weight
    name: Weight
    yaxis_id: weight
    color: "#ffe66d"
    stroke_width: 3
    group_by:
      func: avg
      duration: 2h
```

### 3.7 Statistics Card (Daily Min/Max/Average)

```yaml
type: statistics-graph
title: "Temperature Statistics (Last 7 Days)"
entities:
  - entity: sensor.beehive_1_temperature
stat_types:
  - min
  - max
  - mean
period:
  calendar:
    period: day
days_to_show: 7
```

### 3.8 Weight Statistics Card

```yaml
type: statistics-graph
title: "Weight Statistics (Last 30 Days)"
entities:
  - entity: sensor.beehive_1_weight
stat_types:
  - min
  - max
  - mean
  - change
period:
  calendar:
    period: day
days_to_show: 30
```

### 3.9 Alert Settings Card

```yaml
type: entities
title: "⚙️ Alert Settings"
show_header_toggle: false
entities:
  - entity: input_boolean.beehive_alerts_enabled
    name: Enable Alerts
  - entity: input_number.beehive_min_temperature
    name: Min Temperature Alert
  - entity: input_number.beehive_min_weight
    name: Min Weight Alert
  - entity: input_number.beehive_alert_cooldown
    name: Alert Cooldown
  - type: divider
  - entity: input_datetime.beehive_last_alert
    name: Last Alert Sent
```

### 3.10 Complete Dashboard View

To create a dedicated Beehive dashboard view:

```yaml
title: Beehive Monitor
path: beehive
icon: mdi:beehive-outline
badges: []
cards:
  - type: vertical-stack
    cards:
      # Row 1: Status + Gauges
      - type: horizontal-stack
        cards:
          - type: entities
            title: "🐝 Beehive 1 Status"
            entities:
              - entity: sensor.beehive_1_weight
              - entity: sensor.beehive_1_temperature
              - entity: sensor.beehive_1_humidity
              - entity: sensor.beehive_1_battery
          - type: vertical-stack
            cards:
              - type: horizontal-stack
                cards:
                  - type: gauge
                    entity: sensor.beehive_1_weight
                    name: Weight
                    min: 0
                    max: 80
                    needle: true
                  - type: gauge
                    entity: sensor.beehive_1_temperature
                    name: Temp
                    min: 0
                    max: 50
                    needle: true
                    severity:
                      red: 0
                      yellow: 20
                      green: 27

      # Row 2: Temperature Chart
      - type: history-graph
        title: "🌡️ Temperature (7 Days)"
        entities:
          - entity: sensor.beehive_1_temperature
        hours_to_show: 168

      # Row 3: Humidity Chart
      - type: history-graph
        title: "💧 Humidity (7 Days)"
        entities:
          - entity: sensor.beehive_1_humidity
        hours_to_show: 168

      # Row 4: Weight Chart
      - type: history-graph
        title: "⚖️ Weight (30 Days)"
        entities:
          - entity: sensor.beehive_1_weight
        hours_to_show: 720

      # Row 5: Alert Settings
      - type: entities
        title: "🔔 Alert Configuration"
        entities:
          - entity: input_boolean.beehive_alerts_enabled
          - entity: input_number.beehive_min_temperature
          - entity: input_number.beehive_min_weight
```

### 4.10 Weather Card

```yaml
type: weather-forecast
entity: weather.home
show_forecast: true
name: "Local Weather"
```

### 4.11 Storm Alert Settings Card

```yaml
type: entities
title: "⛈️ Storm Alert Settings"
show_header_toggle: false
entities:
  - entity: input_boolean.storm_alerts_enabled
    name: Enable Storm Alerts
  - entity: input_number.storm_wind_threshold
    name: Wind Speed Threshold
  - type: divider
  - entity: weather.home
    name: Current Weather
  - entity: input_datetime.beehive_last_storm_alert
    name: Last Storm Alert
```

---

## 5. Alert Automations

### 5.1 Temperature Alert (Below Threshold)

**Sends email when temperature drops below user-defined threshold (default 27°C)**

Go to: **Settings → Automations & Scenes → + Create Automation → Create new automation**

Click the 3 dots → Edit in YAML and paste:

```yaml
alias: "🐝 Beehive Temperature Alert"
description: "Send email alert when beehive temperature drops below threshold"
mode: single
trigger:
  - platform: template
    value_template: >-
      {{ states('sensor.beehive_1_temperature') | float(30) <
         states('input_number.beehive_min_temperature') | float(27) }}
    for:
      minutes: 10
condition:
  - condition: state
    entity_id: input_boolean.beehive_alerts_enabled
    state: "on"
  - condition: template
    value_template: >-
      {{ (as_timestamp(now()) - as_timestamp(states('input_datetime.beehive_last_alert')))
         > (states('input_number.beehive_alert_cooldown') | float(6) * 3600) }}
action:
  - service: notify.email_alert
    data:
      title: "🐝 BEEHIVE TEMPERATURE ALERT"
      message: >-
        ⚠️ ATTENTION: Beehive temperature is critically low!

        📍 Hive: Beehive 1
        🌡️ Current Temperature: {{ states('sensor.beehive_1_temperature') }}°C
        ⚠️ Threshold: {{ states('input_number.beehive_min_temperature') }}°C

        📊 Current Status:
        - Weight: {{ states('sensor.beehive_1_weight') }} kg
        - Humidity: {{ states('sensor.beehive_1_humidity') }}%
        - Battery: {{ states('sensor.beehive_1_battery') }}%

        ⏰ Alert Time: {{ now().strftime('%Y-%m-%d %H:%M:%S') }}

        🔧 Action Required:
        Low temperature may indicate:
        - Colony is weak or has absconded
        - Queen issues
        - Need for insulation in cold weather

        --
        ArduiBeeScale Monitoring System
  - service: input_datetime.set_datetime
    target:
      entity_id: input_datetime.beehive_last_alert
    data:
      datetime: "{{ now().strftime('%Y-%m-%d %H:%M:%S') }}"
```

### 5.2 Weight Alert (Below Threshold)

**Sends email when weight drops below user-defined threshold**

```yaml
alias: "🐝 Beehive Weight Alert"
description: "Send email alert when beehive weight drops below threshold"
mode: single
trigger:
  - platform: template
    value_template: >-
      {{ states('sensor.beehive_1_weight') | float(50) <
         states('input_number.beehive_min_weight') | float(15) }}
    for:
      minutes: 5
condition:
  - condition: state
    entity_id: input_boolean.beehive_alerts_enabled
    state: "on"
  - condition: template
    value_template: >-
      {{ (as_timestamp(now()) - as_timestamp(states('input_datetime.beehive_last_alert')))
         > (states('input_number.beehive_alert_cooldown') | float(6) * 3600) }}
action:
  - service: notify.email_alert
    data:
      title: "🐝 BEEHIVE WEIGHT ALERT"
      message: >-
        ⚠️ ATTENTION: Beehive weight is below minimum threshold!

        📍 Hive: Beehive 1
        ⚖️ Current Weight: {{ states('sensor.beehive_1_weight') }} kg
        ⚠️ Minimum Threshold: {{ states('input_number.beehive_min_weight') }} kg

        📊 Current Status:
        - Temperature: {{ states('sensor.beehive_1_temperature') }}°C
        - Humidity: {{ states('sensor.beehive_1_humidity') }}%
        - Battery: {{ states('sensor.beehive_1_battery') }}%

        ⏰ Alert Time: {{ now().strftime('%Y-%m-%d %H:%M:%S') }}

        🔧 Action Required:
        Low weight may indicate:
        - Food stores are depleted - consider feeding
        - Swarm has left the hive
        - Colony is dying or has absconded

        --
        ArduiBeeScale Monitoring System
  - service: input_datetime.set_datetime
    target:
      entity_id: input_datetime.beehive_last_alert
    data:
      datetime: "{{ now().strftime('%Y-%m-%d %H:%M:%S') }}"
```

### 5.3 Sudden Weight Drop Alert (Possible Swarm)

**Detects rapid weight loss that might indicate swarming**

```yaml
alias: "🐝 Beehive Swarm Detection"
description: "Alert when weight drops suddenly (possible swarm)"
mode: single
trigger:
  - platform: state
    entity_id: sensor.beehive_1_weight
condition:
  - condition: state
    entity_id: input_boolean.beehive_alerts_enabled
    state: "on"
  - condition: template
    value_template: >-
      {% set old_weight = trigger.from_state.state | float(0) %}
      {% set new_weight = trigger.to_state.state | float(0) %}
      {% set weight_drop = old_weight - new_weight %}
      {{ weight_drop > 1.5 and old_weight > 10 }}
action:
  - service: notify.email_alert
    data:
      title: "🐝🚨 POSSIBLE SWARM DETECTED!"
      message: >-
        🚨 URGENT: Sudden weight drop detected!

        📍 Hive: Beehive 1
        ⚖️ Previous Weight: {{ trigger.from_state.state }} kg
        ⚖️ Current Weight: {{ trigger.to_state.state }} kg
        📉 Weight Lost: {{ (trigger.from_state.state | float - trigger.to_state.state | float) | round(2) }} kg

        ⏰ Detection Time: {{ now().strftime('%Y-%m-%d %H:%M:%S') }}

        🔧 This may indicate:
        - A swarm has left the hive (1.5-3kg typical swarm weight)
        - Frames were removed
        - Equipment fell off

        Please check your hive as soon as possible!

        --
        ArduiBeeScale Monitoring System
  - service: input_datetime.set_datetime
    target:
      entity_id: input_datetime.beehive_last_alert
    data:
      datetime: "{{ now().strftime('%Y-%m-%d %H:%M:%S') }}"
```

### 5.4 Low Battery Alert

```yaml
alias: "🐝 Beehive Low Battery Alert"
description: "Alert when battery level is low"
mode: single
trigger:
  - platform: numeric_state
    entity_id: sensor.beehive_1_battery
    below: 20
    for:
      hours: 1
condition:
  - condition: state
    entity_id: input_boolean.beehive_alerts_enabled
    state: "on"
action:
  - service: notify.email_alert
    data:
      title: "🔋 Beehive Battery Low"
      message: >-
        ⚠️ Beehive 1 battery is low!

        🔋 Current Level: {{ states('sensor.beehive_1_battery') }}%
        🔌 Voltage: {{ states('sensor.beehive_1_battery_voltage') | default('N/A') }}V

        Please charge or replace the battery within the next few days.

        --
        ArduiBeeScale Monitoring System
```

### 5.5 Hive Offline Alert

```yaml
alias: "🐝 Beehive Offline Alert"
description: "Alert when hive stops reporting data"
mode: single
trigger:
  - platform: state
    entity_id: sensor.beehive_1_weight
    to: "unavailable"
    for:
      hours: 6
  - platform: state
    entity_id: sensor.beehive_1_weight
    to: "unknown"
    for:
      hours: 6
condition:
  - condition: state
    entity_id: input_boolean.beehive_alerts_enabled
    state: "on"
action:
  - service: notify.email_alert
    data:
      title: "📡 Beehive Offline"
      message: >-
        ⚠️ Beehive 1 has stopped reporting data!

        ⏰ Last seen: {{ states.sensor.beehive_1_weight.last_changed }}

        Possible causes:
        - Battery depleted
        - WiFi connectivity issues
        - Hardware failure

        Please check the hive monitor device.

        --
        ArduiBeeScale Monitoring System
```

### 5.6 Daily Summary Report

```yaml
alias: "🐝 Beehive Daily Summary"
description: "Send daily summary email at 8 AM"
mode: single
trigger:
  - platform: time
    at: "08:00:00"
condition: []
action:
  - service: notify.email_alert
    data:
      title: "🐝 Daily Beehive Report - {{ now().strftime('%Y-%m-%d') }}"
      message: >-
        Good morning! Here's your daily beehive status:

        📍 BEEHIVE 1 STATUS
        ═══════════════════════════════════

        ⚖️ Weight: {{ states('sensor.beehive_1_weight') }} kg
        🌡️ Temperature: {{ states('sensor.beehive_1_temperature') }}°C
        💧 Humidity: {{ states('sensor.beehive_1_humidity') }}%
        🔋 Battery: {{ states('sensor.beehive_1_battery') }}%
        📶 WiFi Signal: {{ states('sensor.beehive_1_wifi_signal') }} dBm

        ⚙️ ALERT THRESHOLDS
        ═══════════════════════════════════
        Min Temperature: {{ states('input_number.beehive_min_temperature') }}°C
        Min Weight: {{ states('input_number.beehive_min_weight') }} kg
        Alerts: {{ 'Enabled ✅' if is_state('input_boolean.beehive_alerts_enabled', 'on') else 'Disabled ❌' }}

        Have a great day! 🌻

        --
        ArduiBeeScale Monitoring System
```

### 5.7 Storm Alert (Weather Warning)

**Sends email when severe weather (storm, lightning, hail, high wind) is detected**

> **Important**: Replace `weather.home` with your actual weather entity ID (check Developer Tools → States)

```yaml
alias: "⛈️ Storm Alert - Beehive Warning"
description: "Alert when severe weather is detected that may affect beehives"
mode: single
trigger:
  # Trigger on dangerous weather conditions
  - platform: state
    entity_id: weather.home
    to:
      - "lightning"
      - "lightning-rainy"
      - "hail"
      - "exceptional"
    id: condition_storm
  # Trigger on high wind speed (if available)
  - platform: template
    value_template: >-
      {{ state_attr('weather.home', 'wind_speed') | float(0) >
         states('input_number.storm_wind_threshold') | float(50) }}
    for:
      minutes: 5
    id: high_wind
condition:
  - condition: state
    entity_id: input_boolean.storm_alerts_enabled
    state: "on"
  # Cooldown: only alert once every 4 hours
  - condition: template
    value_template: >-
      {% set last = states('input_datetime.beehive_last_storm_alert') %}
      {% if last == 'unknown' or last == 'unavailable' %}
        true
      {% else %}
        {{ (as_timestamp(now()) - as_timestamp(last)) > 14400 }}
      {% endif %}
action:
  - service: notify.email_alert
    data:
      title: "⛈️ STORM ALERT - Beehive Warning!"
      message: >-
        ⚠️ SEVERE WEATHER ALERT FOR YOUR BEEHIVES!

        📍 Location: Your Apiary
        🌤️ Current Condition: {{ states('weather.home') }}
        💨 Wind Speed: {{ state_attr('weather.home', 'wind_speed') | default('N/A') }} km/h
        🌡️ Temperature: {{ state_attr('weather.home', 'temperature') | default('N/A') }}°C
        💧 Humidity: {{ state_attr('weather.home', 'humidity') | default('N/A') }}%

        ⏰ Alert Time: {{ now().strftime('%Y-%m-%d %H:%M') }}

        🐝 BEEHIVE STATUS:
        • Hive Weight: {{ states('sensor.beehive_1_weight') }} kg
        • Hive Temperature: {{ states('sensor.beehive_1_temperature') }}°C
        • Battery: {{ states('sensor.beehive_1_battery') }}%

        ⚠️ RECOMMENDED ACTIONS:
        {% if 'lightning' in states('weather.home') or 'hail' in states('weather.home') %}
        • Lightning/Hail detected - ensure hives are in protected area
        • Check for damage after storm passes
        • Avoid visiting apiary during active storm
        {% endif %}
        {% if state_attr('weather.home', 'wind_speed') | float(0) > 50 %}
        • High winds detected ({{ state_attr('weather.home', 'wind_speed') }} km/h)
        • Verify hive straps and tie-downs are secure
        • Check for fallen branches near hives
        • Consider adding weight to hive tops
        {% endif %}

        Stay safe! Monitor conditions and check your hives when weather clears.

        --
        ArduiBeeScale Weather Alert System
  - service: input_datetime.set_datetime
    target:
      entity_id: input_datetime.beehive_last_storm_alert
    data:
      datetime: "{{ now().strftime('%Y-%m-%d %H:%M:%S') }}"
```

### 5.8 Storm Forecast Alert (Pre-Warning)

**Sends alert when storm is forecasted in the next hours**

```yaml
alias: "⛈️ Storm Forecast Alert"
description: "Alert when severe weather is forecasted"
mode: single
trigger:
  - platform: time_pattern
    hours: "/3"  # Check every 3 hours
condition:
  - condition: state
    entity_id: input_boolean.storm_alerts_enabled
    state: "on"
  - condition: template
    value_template: >-
      {% set dominated = namespace(storm=false) %}
      {% set dominated.storm = false %}
      {% set dominated.conditions = ['lightning', 'lightning-rainy', 'hail', 'exceptional', 'pouring'] %}
      {% for forecast in state_attr('weather.home', 'forecast')[:8] if forecast is defined %}
        {% if forecast.condition in dominated.conditions %}
          {% set dominated.storm = true %}
        {% endif %}
      {% endfor %}
      {{ dominated.storm }}
  # Only alert once per day for forecasts
  - condition: template
    value_template: >-
      {% set last = states('input_datetime.beehive_last_storm_alert') %}
      {% if last == 'unknown' or last == 'unavailable' %}
        true
      {% else %}
        {{ (as_timestamp(now()) - as_timestamp(last)) > 86400 }}
      {% endif %}
action:
  - service: notify.email_alert
    data:
      title: "⛈️ Storm Forecast - Beehive Preparation"
      message: >-
        📢 SEVERE WEATHER FORECASTED

        A storm is expected in the coming hours. Please prepare your beehives.

        🌤️ Current Weather: {{ states('weather.home') }}

        📅 FORECAST:
        {% for forecast in state_attr('weather.home', 'forecast')[:6] if forecast is defined %}
        • {{ forecast.datetime | as_timestamp | timestamp_custom('%a %H:%M') }}: {{ forecast.condition }} ({{ forecast.temperature }}°C, Wind: {{ forecast.wind_speed | default('?') }} km/h)
        {% endfor %}

        🐝 Recommended preparations:
        • Secure hive lids and ensure straps are tight
        • Check entrance reducers are in place
        • Ensure drainage around hives is clear
        • Have emergency supplies ready

        --
        ArduiBeeScale Weather Alert System
  - service: input_datetime.set_datetime
    target:
      entity_id: input_datetime.beehive_last_storm_alert
    data:
      datetime: "{{ now().strftime('%Y-%m-%d %H:%M:%S') }}"
```

---

## 6. Advanced Configurations

### 6.1 Template Sensors for Statistics

Add to `configuration.yaml`:

```yaml
template:
  - sensor:
      # Daily weight change
      - name: "Beehive 1 Daily Weight Change"
        unique_id: beehive_1_daily_weight_change
        unit_of_measurement: "kg"
        state_class: measurement
        icon: mdi:delta
        state: >-
          {% set current = states('sensor.beehive_1_weight') | float(0) %}
          {% set history = state_attr('sensor.beehive_1_weight_statistics', 'min_value') | float(current) %}
          {{ (current - history) | round(2) }}

      # Weight trend indicator
      - name: "Beehive 1 Weight Trend"
        unique_id: beehive_1_weight_trend
        icon: >-
          {% set change = states('sensor.beehive_1_daily_weight_change') | float(0) %}
          {% if change > 0.5 %}mdi:trending-up
          {% elif change < -0.5 %}mdi:trending-down
          {% else %}mdi:trending-neutral{% endif %}
        state: >-
          {% set change = states('sensor.beehive_1_daily_weight_change') | float(0) %}
          {% if change > 0.5 %}Increasing
          {% elif change < -0.5 %}Decreasing
          {% else %}Stable{% endif %}

      # Estimated honey weight
      - name: "Beehive 1 Honey Estimate"
        unique_id: beehive_1_honey_estimate
        unit_of_measurement: "kg"
        state_class: measurement
        icon: mdi:beehive-outline
        state: >-
          {% set total = states('sensor.beehive_1_weight') | float(0) %}
          {% set empty_hive = 15 %}
          {% set honey = total - empty_hive %}
          {{ [honey, 0] | max | round(1) }}

      # Battery days remaining
      - name: "Beehive 1 Battery Days Remaining"
        unique_id: beehive_1_battery_days
        unit_of_measurement: "days"
        icon: mdi:battery-clock
        state: >-
          {% set percent = states('sensor.beehive_1_battery') | float(50) %}
          {% set drain_per_day = 0.5 %}
          {{ (percent / drain_per_day) | round(0) }}

      # Temperature status
      - name: "Beehive 1 Temperature Status"
        unique_id: beehive_1_temp_status
        icon: >-
          {% set temp = states('sensor.beehive_1_temperature') | float(30) %}
          {% if temp < 20 %}mdi:snowflake
          {% elif temp > 38 %}mdi:fire
          {% else %}mdi:check-circle{% endif %}
        state: >-
          {% set temp = states('sensor.beehive_1_temperature') | float(30) %}
          {% if temp < 20 %}Cold
          {% elif temp < 27 %}Cool
          {% elif temp <= 36 %}Optimal
          {% elif temp <= 38 %}Warm
          {% else %}Hot{% endif %}
```

### 6.2 Long-Term Statistics

Add to `configuration.yaml`:

```yaml
# Keep 1 year of statistics for beehive sensors
recorder:
  purge_keep_days: 365
  commit_interval: 60
  include:
    entity_globs:
      - sensor.beehive_*
    entities:
      - input_number.beehive_min_weight
      - input_number.beehive_min_temperature
```

### 6.3 InfluxDB for Advanced Charts (Optional)

For long-term data analysis with Grafana:

1. Install InfluxDB add-on
2. Add to `configuration.yaml`:

```yaml
influxdb:
  host: a0d7b954-influxdb
  port: 8086
  database: beehive_data
  username: homeassistant
  password: !secret influxdb_password
  max_retries: 3
  include:
    entity_globs:
      - sensor.beehive_*
```

---

## Quick Start Checklist

1. [ ] Configure email notifications (`configuration.yaml`)
2. [ ] Create Gmail App Password (if using Gmail)
3. [ ] Add password to `secrets.yaml`
4. [ ] Restart Home Assistant
5. [ ] Test email notification (Developer Tools → Services)
6. [ ] Create helper entities (Settings → Helpers)
7. [ ] Import automations (Settings → Automations)
8. [ ] Create dashboard cards
9. [ ] Set your thresholds in the dashboard
10. [ ] Enable alerts toggle

---

## Troubleshooting

### Email not sending

1. Check Home Assistant logs: Settings → System → Logs
2. Verify SMTP settings are correct
3. For Gmail: ensure "Less secure apps" or App Password is configured
4. Test with Developer Tools → Services → notify.email_alert

### Automations not triggering

1. Check entity IDs match your sensors
2. Verify `input_boolean.beehive_alerts_enabled` is ON
3. Check cooldown timer hasn't blocked the alert
4. Look at automation traces: Settings → Automations → (click automation) → Traces

### Charts not showing data

1. Verify sensor has history (Developer Tools → States)
2. Check recorder configuration includes your sensors
3. Wait for enough data points to accumulate

---

## Support

For issues or questions:
- Check Home Assistant documentation: https://www.home-assistant.io/docs/
- Home Assistant Community: https://community.home-assistant.io/
