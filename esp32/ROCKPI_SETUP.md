# Rock Pi 6 Setup Guide

Complete setup guide for Rock Pi 6 with Home Assistant, Mosquitto MQTT, and dedicated WiFi network for beehives.

---

## Overview

This setup creates a **dedicated beehive WiFi network** using the USB WiFi dongle as an Access Point:

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           ROCK PI 6                                      │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐      │
│  │   Ethernet      │    │  USB WiFi       │    │  Home Assistant │      │
│  │   (eth0)        │    │  Dongle (wlan0) │    │  + Mosquitto    │      │
│  │   Internet      │    │  Access Point   │    │  + Dashboard    │      │
│  └────────┬────────┘    └────────┬────────┘    └─────────────────┘      │
│           │                      │                                       │
└───────────│──────────────────────│───────────────────────────────────────┘
            │                      │
            ▼                      ▼
    ┌───────────────┐      ┌───────────────────────────────┐
    │ Home Router   │      │    Beehive WiFi Network       │
    │ (Internet)    │      │    SSID: "BeehiveNet"         │
    │               │      │    192.168.4.x                │
    └───────────────┘      │                               │
                           │  ┌─────────┐  ┌─────────┐    │
                           │  │ Hive 1  │  │ Hive 2  │    │
                           │  │ ESP32   │  │ ESP32   │    │
                           │  └─────────┘  └─────────┘    │
                           └───────────────────────────────┘
```

**Benefits of separate beehive network:**
- Better security (beehives isolated from main network)
- Optimized for low-power ESP32 devices
- No interference with home network
- Can work even without internet connection
- Extended range with external antenna on USB dongle

---

## Hardware Requirements

| Component | Description | Notes |
|-----------|-------------|-------|
| Rock Pi 6 | Main server | 4GB+ RAM recommended |
| MicroSD Card | 32GB+ Class 10 | For initial boot |
| **SSD Drive** | 128GB+ NVMe/SATA | **Recommended for reliability** |
| **SSD Extension Card** | NVMe HAT or SATA adapter | For Rock Pi SSD connection |
| Power Supply | USB-C 5V 3A+ | Official or quality PSU |
| Ethernet Cable | Cat5e/Cat6 | For internet connection |
| USB WiFi Dongle | RTL8188/RTL8812 with SMA | For beehive network |
| External Antenna | 3-5 dBi | For extended range |

### SSD Options for Rock Pi

| Type | Interface | Speed | Notes |
|------|-----------|-------|-------|
| **NVMe SSD** | M.2 PCIe | ~1000 MB/s | Fastest, requires NVMe HAT |
| **SATA SSD** | SATA III | ~500 MB/s | Good speed, SATA adapter needed |
| **USB 3.0 SSD** | USB | ~400 MB/s | Easiest, external enclosure |

**Recommended SSD Extension Cards:**
- Rock Pi Penta SATA HAT (up to 5 SATA drives)
- Rock Pi Quad SATA HAT
- M.2 NVMe to USB 3.0 enclosure (simplest)
- Generic M.2 NVMe HAT for Rock Pi

### Recommended USB WiFi Dongles

| Chipset | Range | Notes |
|---------|-------|-------|
| RTL8188EUS | ~50m | Budget, good Linux support |
| RTL8812AU | ~100m | Dual-band, excellent range |
| MT7612U | ~100m | Good AP mode support |

---

## Step 1: Install Operating System

### Option A: Armbian (Recommended for AP mode)

1. **Download Armbian for Rock Pi 6**
   - Go to: https://www.armbian.com/rockpi-4/
   - Download: Armbian Bookworm (CLI or Desktop)

2. **Flash to SD card**
   - Download Balena Etcher: https://www.balena.io/etcher/
   - Flash the image to SD card

3. **First boot**
   - Insert SD card into Rock Pi
   - Connect Ethernet cable
   - Connect power
   - Wait 2-3 minutes for first boot

4. **Find Rock Pi IP address**
   - Check your router's DHCP client list
   - Or use: `nmap -sn 192.168.1.0/24`

5. **SSH into Rock Pi**
   ```bash
   ssh root@192.168.1.xxx
   # Default password: 1234
   ```

6. **Complete initial setup**
   - Change root password
   - Create new user (e.g., "beehive")
   - Set timezone

### Option B: Home Assistant OS (Simpler but limited AP support)

> **Note**: HA OS has limited network configuration. Use Armbian if you need full AP control.

1. Download HA OS for Rock Pi: https://www.home-assistant.io/installation/
2. Flash and boot
3. Access at: `http://homeassistant.local:8123`

---

## Step 1b: Configure SSD Storage

Using an SSD instead of SD card provides **better reliability** and **faster performance** for Home Assistant database operations.

### 1b.1 Connect SSD Extension Card

1. **Power off Rock Pi**
2. **Connect SSD HAT/Extension card** to Rock Pi GPIO/PCIe
3. **Insert SSD** into the extension card slot
4. **Power on** and boot from SD card

### 1b.2 Detect SSD

```bash
# List all storage devices
lsblk

# You should see something like:
# NAME         SIZE  TYPE MOUNTPOINT
# mmcblk0      32G   disk           <- SD Card
# ├─mmcblk0p1  256M  part /boot
# └─mmcblk0p2  31.7G part /
# nvme0n1      256G  disk           <- NVMe SSD
# OR
# sda          256G  disk           <- SATA SSD

# Check SSD details
sudo fdisk -l /dev/nvme0n1   # For NVMe
# OR
sudo fdisk -l /dev/sda       # For SATA/USB SSD
```

### 1b.3 Partition and Format SSD

```bash
# Install partition tools
sudo apt install -y parted

# Identify your SSD device (nvme0n1 or sda)
SSD_DEVICE="/dev/nvme0n1"   # Change if different

# Create GPT partition table
sudo parted $SSD_DEVICE mklabel gpt

# Create single partition using all space
sudo parted $SSD_DEVICE mkpart primary ext4 0% 100%

# Format as ext4
sudo mkfs.ext4 ${SSD_DEVICE}p1   # For NVMe (adds p1)
# OR
sudo mkfs.ext4 ${SSD_DEVICE}1    # For SATA (adds 1)
```

### 1b.4 Mount SSD for Home Assistant Data

```bash
# Create mount point
sudo mkdir -p /mnt/ssd

# Get partition UUID (more reliable than device name)
sudo blkid

# Example output:
# /dev/nvme0n1p1: UUID="xxxx-yyyy-zzzz" TYPE="ext4"

# Add to fstab for auto-mount at boot
sudo nano /etc/fstab
```

Add this line (replace UUID with yours):
```
UUID=xxxx-yyyy-zzzz  /mnt/ssd  ext4  defaults,noatime  0  2
```

Mount and verify:
```bash
# Mount now
sudo mount -a

# Verify
df -h /mnt/ssd

# Should show your SSD mounted
```

### 1b.5 Create Home Assistant Directory on SSD

```bash
# Create directory structure on SSD
sudo mkdir -p /mnt/ssd/homeassistant
sudo mkdir -p /mnt/ssd/mosquitto

# Set permissions
sudo chown -R $USER:$USER /mnt/ssd/homeassistant
sudo chown -R $USER:$USER /mnt/ssd/mosquitto

# Create symlink for convenience
ln -s /mnt/ssd/homeassistant ~/homeassistant
```

### 1b.6 (Optional) Boot OS from SSD

For maximum performance, you can boot the entire OS from SSD.

#### Method A: Use armbian-config (Easiest)

```bash
sudo armbian-config
```

Navigate to: **System → Install → Install/Update the bootloader on MTD/SPI → Boot from SPI - systemass on SATA/USB**

#### Method B: Manual Transfer

```bash
# Install rsync
sudo apt install -y rsync

# Copy root filesystem to SSD
sudo rsync -axv / /mnt/ssd/

# Update fstab on SSD
sudo nano /mnt/ssd/etc/fstab
# Change root mount from mmcblk0p2 to SSD partition

# Update boot configuration (depends on Rock Pi model)
# Edit /boot/armbianEnv.txt or extlinux.conf
sudo nano /boot/armbianEnv.txt
# Add or modify: rootdev=/dev/nvme0n1p1
```

### 1b.7 Verify SSD Performance

```bash
# Install benchmark tool
sudo apt install -y hdparm

# Test read speed
sudo hdparm -t /dev/nvme0n1

# Expected results:
# SD Card: ~40-90 MB/s
# SATA SSD: ~400-500 MB/s
# NVMe SSD: ~800-2000 MB/s
```

### SSD vs SD Card Comparison

| Aspect | SD Card | SSD |
|--------|---------|-----|
| Read Speed | 40-90 MB/s | 400-2000 MB/s |
| Write Speed | 20-40 MB/s | 300-1500 MB/s |
| Lifespan | ~10,000 writes | ~1,000,000 writes |
| Reliability | Lower (wear) | Higher |
| Cost (128GB) | ~€15 | ~€25-40 |
| Best For | Initial boot | HA database, Docker |

> **Recommendation**: Keep SD card for boot, use SSD for Home Assistant data and Docker volumes.

---

## Step 2: Configure USB WiFi Dongle

### 2.1 Verify WiFi Dongle Detection

```bash
# Check USB devices
lsusb

# Check wireless interfaces
iwconfig

# Should see wlan0 or similar
ip link show
```

### 2.2 Install Required Packages

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install hostapd (Access Point daemon)
sudo apt install -y hostapd

# Install dnsmasq (DHCP server)
sudo apt install -y dnsmasq

# Install network tools
sudo apt install -y net-tools wireless-tools
```

### 2.3 Stop Services During Configuration

```bash
sudo systemctl stop hostapd
sudo systemctl stop dnsmasq
```

---

## Step 3: Configure WiFi Access Point

### 3.1 Configure Static IP for WiFi Interface

Edit `/etc/dhcpcd.conf`:
```bash
sudo nano /etc/dhcpcd.conf
```

Add at the end:
```
# Static IP for WiFi Access Point
interface wlan0
    static ip_address=192.168.4.1/24
    nohook wpa_supplicant
```

### 3.2 Configure DHCP Server (dnsmasq)

Backup and edit dnsmasq config:
```bash
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
sudo nano /etc/dnsmasq.conf
```

Add this content:
```conf
# Beehive WiFi Network DHCP Configuration
interface=wlan0

# DHCP range for beehives
dhcp-range=192.168.4.10,192.168.4.50,255.255.255.0,24h

# DNS server (use router or public DNS)
server=8.8.8.8
server=8.8.4.4

# Domain
domain=beehive.local

# Static IPs for beehives (optional but recommended)
# Add your beehives here after they connect
#dhcp-host=AA:BB:CC:DD:EE:F1,hive01,192.168.4.11
#dhcp-host=AA:BB:CC:DD:EE:F2,hive02,192.168.4.12
#dhcp-host=AA:BB:CC:DD:EE:F3,hive03,192.168.4.13

# Logging
log-queries
log-dhcp
```

### 3.3 Configure Access Point (hostapd)

Create hostapd config:
```bash
sudo nano /etc/hostapd/hostapd.conf
```

Add this content:
```conf
# Beehive WiFi Access Point Configuration
interface=wlan0
driver=nl80211

# Network name (SSID) - CHANGE THIS
ssid=BeehiveNet

# WiFi mode (g = 2.4GHz, a = 5GHz)
hw_mode=g
channel=6

# Country code (important for TX power)
country_code=FR

# WPA2 security - CHANGE PASSWORD
wpa=2
wpa_passphrase=YourSecurePassword123
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP

# Performance settings
ieee80211n=1
wmm_enabled=1

# Hidden network (optional, set to 1 to hide SSID)
ignore_broadcast_ssid=0

# Max clients
max_num_sta=10

# Logging
logger_syslog=-1
logger_syslog_level=2
```

> **Important**: Change `ssid` and `wpa_passphrase` to your own values!

### 3.4 Point hostapd to Config File

```bash
sudo nano /etc/default/hostapd
```

Find and change:
```
DAEMON_CONF="/etc/hostapd/hostapd.conf"
```

### 3.5 Enable IP Forwarding (Optional - for internet access)

If you want beehives to have internet access through the Rock Pi:

```bash
sudo nano /etc/sysctl.conf
```

Uncomment:
```
net.ipv4.ip_forward=1
```

Apply:
```bash
sudo sysctl -p
```

Add NAT rule:
```bash
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"
```

Load on boot - edit `/etc/rc.local`:
```bash
iptables-restore < /etc/iptables.ipv4.nat
exit 0
```

### 3.6 Start Services

```bash
# Restart dhcpcd
sudo systemctl restart dhcpcd

# Unmask and enable hostapd
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd

# Enable and start dnsmasq
sudo systemctl enable dnsmasq
sudo systemctl start dnsmasq
```

### 3.7 Verify Access Point

```bash
# Check hostapd status
sudo systemctl status hostapd

# Check if AP is broadcasting
iwconfig wlan0

# Check connected clients
cat /var/lib/misc/dnsmasq.leases
```

---

## Step 4: Install Docker

Home Assistant runs best in Docker on Armbian.

```bash
# Install Docker
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh

# Add user to docker group
sudo usermod -aG docker $USER

# Install Docker Compose
sudo apt install -y docker-compose

# Verify installation
docker --version
docker-compose --version
```

---

## Step 5: Install Home Assistant Container

### 5.1 Create Directory Structure on SSD

```bash
# If you set up SSD in Step 1b, directories already exist
# Otherwise create them:
mkdir -p /mnt/ssd/homeassistant/config
mkdir -p /mnt/ssd/mosquitto/{config,data,log}

# Navigate to SSD directory
cd /mnt/ssd/homeassistant

# Or use symlink if created earlier
cd ~/homeassistant
```

### 5.2 Create Docker Compose File

```bash
nano docker-compose.yml
```

Add this content (**using SSD paths**):
```yaml
version: '3'
services:
  homeassistant:
    container_name: homeassistant
    image: ghcr.io/home-assistant/home-assistant:stable
    volumes:
      - /mnt/ssd/homeassistant/config:/config      # SSD storage
      - /etc/localtime:/etc/localtime:ro
    restart: unless-stopped
    privileged: true
    network_mode: host

  mosquitto:
    container_name: mosquitto
    image: eclipse-mosquitto:latest
    volumes:
      - /mnt/ssd/mosquitto/config:/mosquitto/config  # SSD storage
      - /mnt/ssd/mosquitto/data:/mosquitto/data      # SSD storage
      - /mnt/ssd/mosquitto/log:/mosquitto/log        # SSD storage
    ports:
      - "1883:1883"
      - "9001:9001"
    restart: unless-stopped
```

> **SSD Benefit**: Home Assistant database writes are heavy. Using SSD prevents SD card wear and improves performance.

### 5.3 Configure Mosquitto

```bash
# Create directories on SSD (if not already done)
mkdir -p /mnt/ssd/mosquitto/config
mkdir -p /mnt/ssd/mosquitto/data
mkdir -p /mnt/ssd/mosquitto/log

# Create config file
nano /mnt/ssd/mosquitto/config/mosquitto.conf
```

Add:
```conf
# Mosquitto MQTT Broker Configuration
listener 1883
allow_anonymous false
password_file /mosquitto/config/passwords

# Persistence on SSD
persistence true
persistence_location /mosquitto/data/

# Logging
log_dest file /mosquitto/log/mosquitto.log
log_type all
```

### 5.4 Create MQTT User

```bash
# Create password file using SSD path
docker run -it --rm -v /mnt/ssd/mosquitto/config:/mosquitto/config eclipse-mosquitto mosquitto_passwd -c /mosquitto/config/passwords beehive
# Enter password when prompted (e.g., "beehive_mqtt_password")
```

### 5.5 Start Services

```bash
# Navigate to directory with docker-compose.yml
cd /mnt/ssd/homeassistant

docker-compose up -d

# Check status
docker-compose ps

# View logs
docker-compose logs -f homeassistant
```

### 5.6 Access Home Assistant

1. Open browser: `http://192.168.1.xxx:8123` (use Rock Pi's ethernet IP)
2. Create your account
3. Complete setup wizard

---

## Step 6: Configure Home Assistant MQTT

### 6.1 Add MQTT Integration

1. Go to: **Settings → Devices & Services → + Add Integration**
2. Search for **"MQTT"**
3. Configure:
   - Broker: `localhost` or `192.168.4.1`
   - Port: `1883`
   - Username: `beehive`
   - Password: (your password from step 5.4)
4. Click **Submit**

### 6.2 Test MQTT Connection

In Home Assistant:
1. Go to: **Developer Tools → MQTT**
2. Subscribe to: `#`
3. You should see messages when ESP32 connects

---

## Step 7: Configure ESP32 for Beehive Network

Update `config.h` on each ESP32:

```cpp
// WiFi - Beehive dedicated network
#define WIFI_SSID         "BeehiveNet"           // Your AP SSID
#define WIFI_PASSWORD     "YourSecurePassword123" // Your AP password

// MQTT - Rock Pi Access Point IP
#define MQTT_BROKER       "192.168.4.1"          // Rock Pi AP IP
#define MQTT_PORT         1883
#define MQTT_USER         "beehive"
#define MQTT_PASSWORD     "beehive_mqtt_password"

// Hive Identity
#define HIVE_ID           "hive01"
#define HIVE_NAME         "Garden Hive"
```

---

## Step 8: Verify Complete Setup

### 8.1 Check WiFi Network

On your phone/laptop:
1. Look for WiFi network "BeehiveNet"
2. Connect with your password
3. You should get IP 192.168.4.x

### 8.2 Check ESP32 Connection

1. Power on ESP32
2. Open Serial Monitor (115200 baud)
3. Should see:
   ```
   [INFO] Connecting to WiFi...
   [INFO] WiFi connected! IP: 192.168.4.11
   [INFO] Connecting to MQTT...
   [INFO] MQTT connected!
   ```

### 8.3 Check Home Assistant

1. Go to: **Settings → Devices & Services → MQTT**
2. You should see your beehive device
3. Sensors should show values

---

## Network Summary

| Network | IP Range | Purpose |
|---------|----------|---------|
| Home Network (eth0) | 192.168.1.x | Internet, HA web access |
| Beehive Network (wlan0) | 192.168.4.x | ESP32 beehives only |

| Device | IP Address | Notes |
|--------|------------|-------|
| Rock Pi (ethernet) | 192.168.1.xxx | Access HA from home network |
| Rock Pi (WiFi AP) | 192.168.4.1 | MQTT broker for beehives |
| Hive 1 | 192.168.4.11 | First beehive ESP32 |
| Hive 2 | 192.168.4.12 | Second beehive ESP32 |
| Hive 3 | 192.168.4.13 | Third beehive ESP32 |

---

## Troubleshooting

### WiFi AP Not Starting

```bash
# Check hostapd status
sudo systemctl status hostapd

# Check for errors
sudo journalctl -u hostapd -n 50

# Common fix: check driver compatibility
sudo hostapd -dd /etc/hostapd/hostapd.conf
```

### ESP32 Can't Connect to WiFi

- Verify SSID and password match exactly
- Check WiFi channel (try channel 1, 6, or 11)
- Ensure ESP32 is within range
- Check `hostapd` logs: `sudo tail -f /var/log/syslog | grep hostapd`

### MQTT Connection Failed

```bash
# Test MQTT from Rock Pi
mosquitto_pub -h localhost -u beehive -P your_password -t test -m "hello"

# Check mosquitto logs
docker logs mosquitto
```

### Home Assistant Can't Find Sensors

1. Check MQTT integration is connected
2. Verify ESP32 is publishing (check MQTT tab in HA)
3. Restart ESP32 to resend discovery messages

---

## Optional: Increase WiFi Range

### Use High-Gain Antenna

Replace the stock antenna with 5-8 dBi antenna for extended range.

### Adjust TX Power

```bash
# Check current TX power
iwconfig wlan0 | grep Tx-Power

# Increase power (if supported)
sudo iwconfig wlan0 txpower 20
```

Add to `/etc/rc.local` for persistence.

### Position Antenna

- Mount antenna vertically
- Position Rock Pi at elevation
- Avoid metal obstructions

---

## Security Recommendations

1. **Change default passwords** for WiFi and MQTT
2. **Use WPA2** encryption (already configured)
3. **Disable SSID broadcast** if desired (set `ignore_broadcast_ssid=1`)
4. **MAC filtering** - add only known ESP32 MAC addresses to dnsmasq
5. **Firewall** - restrict access to MQTT port

---

## Maintenance Commands

```bash
# Restart all services
sudo systemctl restart hostapd dnsmasq
cd /mnt/ssd/homeassistant && docker-compose restart

# View connected beehives
cat /var/lib/misc/dnsmasq.leases

# Check WiFi signal of connected devices
iw dev wlan0 station dump

# Update Home Assistant
cd /mnt/ssd/homeassistant
docker-compose pull
docker-compose up -d

# Backup Home Assistant config (to home directory)
tar -czvf ~/ha-backup-$(date +%Y%m%d).tar.gz /mnt/ssd/homeassistant/config/

# Backup entire SSD data
tar -czvf ~/beehive-full-backup-$(date +%Y%m%d).tar.gz /mnt/ssd/
```

### SSD Maintenance Commands

```bash
# Check SSD health (if SMART supported)
sudo apt install -y smartmontools
sudo smartctl -a /dev/nvme0n1   # For NVMe
# OR
sudo smartctl -a /dev/sda       # For SATA

# Check SSD usage
df -h /mnt/ssd

# Check SSD I/O statistics
iostat -x /dev/nvme0n1 1 5

# Trim SSD (important for performance)
sudo fstrim -v /mnt/ssd

# Add weekly TRIM cron job
sudo nano /etc/cron.weekly/fstrim-ssd
# Add: #!/bin/bash
#      fstrim /mnt/ssd
sudo chmod +x /etc/cron.weekly/fstrim-ssd

# Monitor SSD temperature (NVMe only)
sudo nvme smart-log /dev/nvme0n1 | grep temperature
```

### Docker on SSD Commands

```bash
# Check Docker disk usage
docker system df

# Clean unused Docker data (free space)
docker system prune -a

# Move Docker data directory to SSD (optional)
# Stop Docker
sudo systemctl stop docker
# Edit /etc/docker/daemon.json
sudo nano /etc/docker/daemon.json
# Add: {"data-root": "/mnt/ssd/docker"}
# Move existing data
sudo rsync -aP /var/lib/docker/ /mnt/ssd/docker/
# Start Docker
sudo systemctl start docker
```
