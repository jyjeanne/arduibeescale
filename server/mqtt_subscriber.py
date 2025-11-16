#!/usr/bin/env python3
"""
BeezScale MQTT Subscriber
=========================
Subscribes to MQTT topics from Arduino beehive monitors and stores data in SQLite.

Author: Jeremy JEANNE
Project: ArduiBeeScale
License: GNU GPLv3

This service:
- Connects to Mosquitto MQTT broker
- Subscribes to beehive/# topics
- Parses incoming JSON sensor data
- Stores readings in SQLite database
- Handles connection failures gracefully
- Logs all activity for debugging
"""

import paho.mqtt.client as mqtt
import sqlite3
import json
import logging
import time
import os
import sys
from datetime import datetime
from pathlib import Path

# ==========================================
# CONFIGURATION
# ==========================================

MQTT_BROKER = "localhost"  # Mosquitto broker address
MQTT_PORT = 1883
MQTT_TOPIC = "beehive/#"
MQTT_CLIENT_ID = "beehive-subscriber"
MQTT_KEEPALIVE = 60

DATABASE_PATH = "/home/pi/beehive-monitor/beehive_data.db"
LOG_FILE = "/home/pi/beehive-monitor/mqtt_subscriber.log"

# Create necessary directories
Path(DATABASE_PATH).parent.mkdir(parents=True, exist_ok=True)

# ==========================================
# LOGGING CONFIGURATION
# ==========================================

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler(LOG_FILE),
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger(__name__)

# ==========================================
# DATABASE INITIALIZATION
# ==========================================

def init_database():
    """Initialize SQLite database with required schema."""
    try:
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()

        # Create hives table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS hives (
                hive_id TEXT PRIMARY KEY,
                name TEXT,
                location TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                last_reading TIMESTAMP
            )
        """)

        # Create readings table
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS readings (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                hive_id TEXT NOT NULL,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                temperature REAL,
                humidity REAL,
                weight REAL,
                battery_voltage REAL,
                raw_json TEXT,
                FOREIGN KEY (hive_id) REFERENCES hives(hive_id)
            )
        """)

        # Create index for faster queries
        cursor.execute("""
            CREATE INDEX IF NOT EXISTS idx_hive_timestamp
            ON readings(hive_id, timestamp DESC)
        """)

        cursor.execute("""
            CREATE INDEX IF NOT EXISTS idx_timestamp
            ON readings(timestamp DESC)
        """)

        conn.commit()
        conn.close()
        logger.info("Database initialized successfully")

    except sqlite3.Error as e:
        logger.error(f"Database initialization failed: {e}")
        raise

# ==========================================
# MQTT CALLBACKS
# ==========================================

def on_connect(client, userdata, flags, rc):
    """Called when connected to MQTT broker."""
    if rc == 0:
        logger.info("Connected to MQTT broker successfully")
        client.subscribe(MQTT_TOPIC, qos=1)
        logger.info(f"Subscribed to topic: {MQTT_TOPIC}")
    else:
        logger.error(f"Failed to connect to MQTT broker. Error code: {rc}")

def on_disconnect(client, userdata, rc):
    """Called when disconnected from MQTT broker."""
    if rc != 0:
        logger.warning(f"Unexpected disconnection from MQTT broker. Error code: {rc}")
    else:
        logger.info("Disconnected from MQTT broker")

def on_message(client, userdata, msg):
    """Called when a message is received from MQTT broker."""
    try:
        # Extract hive_id from topic (beehive/hive-001 -> hive-001)
        topic_parts = msg.topic.split('/')
        if len(topic_parts) < 2:
            logger.warning(f"Invalid topic format: {msg.topic}")
            return

        hive_id = topic_parts[1]

        # Parse JSON payload
        try:
            payload = json.loads(msg.payload.decode('utf-8'))
        except json.JSONDecodeError as e:
            logger.error(f"Failed to parse JSON from topic {msg.topic}: {e}")
            logger.debug(f"Raw payload: {msg.payload}")
            return

        # Extract sensor data
        temperature = payload.get('temperature')
        humidity = payload.get('humidity')
        weight = payload.get('weight')
        battery_voltage = payload.get('battery_voltage')

        # Validate data
        if temperature is None or humidity is None or weight is None:
            logger.warning(f"Missing required sensor data from {hive_id}")
            return

        # Store in database
        store_reading(hive_id, temperature, humidity, weight, battery_voltage, msg.payload.decode('utf-8'))

        logger.info(f"Stored reading from {hive_id}: T={temperature}°C, H={humidity}%, W={weight}g, V={battery_voltage}V")

    except Exception as e:
        logger.error(f"Error processing MQTT message: {e}")
        logger.debug(f"Message topic: {msg.topic}, Payload: {msg.payload}")

def on_subscribe(client, userdata, mid, granted_qos):
    """Called when subscription is confirmed."""
    logger.info(f"Subscription confirmed with QoS: {granted_qos[0]}")

# ==========================================
# DATABASE OPERATIONS
# ==========================================

def store_reading(hive_id, temperature, humidity, weight, battery_voltage, raw_json):
    """Store sensor reading in database."""
    try:
        conn = sqlite3.connect(DATABASE_PATH)
        cursor = conn.cursor()

        # Ensure hive exists
        cursor.execute("""
            INSERT OR IGNORE INTO hives (hive_id, name, location)
            VALUES (?, ?, ?)
        """, (hive_id, hive_id, "Unknown"))

        # Insert reading
        cursor.execute("""
            INSERT INTO readings (hive_id, temperature, humidity, weight, battery_voltage, raw_json)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (hive_id, temperature, humidity, weight, battery_voltage, raw_json))

        # Update last_reading timestamp
        cursor.execute("""
            UPDATE hives SET last_reading = CURRENT_TIMESTAMP WHERE hive_id = ?
        """, (hive_id,))

        conn.commit()
        conn.close()

    except sqlite3.Error as e:
        logger.error(f"Failed to store reading: {e}")

def get_latest_readings():
    """Get latest reading from each hive."""
    try:
        conn = sqlite3.connect(DATABASE_PATH)
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()

        cursor.execute("""
            SELECT DISTINCT ON (hive_id)
                hive_id, temperature, humidity, weight, battery_voltage, timestamp
            FROM readings
            ORDER BY hive_id, timestamp DESC
        """)

        results = cursor.fetchall()
        conn.close()
        return results

    except sqlite3.Error as e:
        logger.error(f"Failed to retrieve readings: {e}")
        return []

# ==========================================
# MQTT CLIENT SETUP
# ==========================================

def setup_mqtt_client():
    """Create and configure MQTT client."""
    client = mqtt.Client(client_id=MQTT_CLIENT_ID, clean_session=True)

    # Set callbacks
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message
    client.on_subscribe = on_subscribe

    # Enable logging
    client.enable_logger(logger)

    return client

# ==========================================
# MAIN SERVICE LOOP
# ==========================================

def main():
    """Main service loop."""
    logger.info("=" * 50)
    logger.info("Starting BeezScale MQTT Subscriber")
    logger.info("=" * 50)
    logger.info(f"MQTT Broker: {MQTT_BROKER}:{MQTT_PORT}")
    logger.info(f"Database: {DATABASE_PATH}")
    logger.info(f"Subscribing to: {MQTT_TOPIC}")
    logger.info("=" * 50)

    # Initialize database
    init_database()

    # Create MQTT client
    client = setup_mqtt_client()

    # Connection loop with auto-reconnect
    while True:
        try:
            logger.info("Attempting to connect to MQTT broker...")
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=MQTT_KEEPALIVE)

            # Start network loop
            client.loop_forever()

        except ConnectionRefusedError:
            logger.error("Connection refused - is Mosquitto running?")
            logger.info("Retrying in 10 seconds...")
            time.sleep(10)

        except Exception as e:
            logger.error(f"Connection error: {e}")
            logger.info("Retrying in 10 seconds...")
            time.sleep(10)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.info("Subscriber stopped by user")
        sys.exit(0)
    except Exception as e:
        logger.critical(f"Fatal error: {e}")
        sys.exit(1)
