#!/usr/bin/env python3
"""
BeezScale Flask Web Server
===========================
Serves the real-time dashboard and provides REST API for beehive data.

Author: Jeremy JEANNE
Project: ArduiBeeScale
License: GNU GPLv3

This service:
- Serves the real-time dashboard at /
- Provides REST API endpoints for data retrieval
- Uses WebSocket (Socket.io) for real-time updates
- Connects to SQLite database created by MQTT subscriber
- Handles multiple concurrent clients
"""

from flask import Flask, render_template, jsonify, request
from flask_socketio import SocketIO, emit, disconnect
from flask_cors import CORS
import sqlite3
import paho.mqtt.client as mqtt
import json
import logging
import sys
import threading
from datetime import datetime, timedelta
from pathlib import Path
import os

# ==========================================
# CONFIGURATION
# ==========================================

DATABASE_PATH = "/home/pi/beehive-monitor/beehive_data.db"
LOG_FILE = "/home/pi/beehive-monitor/flask_app.log"
TEMPLATES_DIR = "/home/pi/beehive-monitor/templates"
STATIC_DIR = "/home/pi/beehive-monitor/static"

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC = "beehive/#"
MQTT_CLIENT_ID = "beehive-flask-bridge"

# Create necessary directories
Path(DATABASE_PATH).parent.mkdir(parents=True, exist_ok=True)
Path(TEMPLATES_DIR).mkdir(parents=True, exist_ok=True)
Path(STATIC_DIR).mkdir(parents=True, exist_ok=True)

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
# FLASK APP SETUP
# ==========================================

app = Flask(__name__, template_folder=TEMPLATES_DIR, static_folder=STATIC_DIR)
app.config['SECRET_KEY'] = 'beehive-monitor-secret-key-change-in-production'
app.config['CORS_HEADERS'] = 'Content-Type'

CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading')

# Global WebSocket clients tracking
connected_clients = set()

# ==========================================
# MQTT BRIDGE (Real-time Updates)
# ==========================================

mqtt_client = None

def on_mqtt_message(client, userdata, msg):
    """Bridge MQTT messages to WebSocket clients."""
    try:
        topic_parts = msg.topic.split('/')
        if len(topic_parts) < 2:
            return

        hive_id = topic_parts[1]

        try:
            payload = json.loads(msg.payload.decode('utf-8'))
        except json.JSONDecodeError:
            return

        # Send to all connected WebSocket clients
        if connected_clients:
            socketio.emit('new_reading', {
                'hive_id': hive_id,
                'temperature': payload.get('temperature'),
                'humidity': payload.get('humidity'),
                'weight': payload.get('weight'),
                'battery_voltage': payload.get('battery_voltage'),
                'timestamp': datetime.now().isoformat()
            }, broadcast=True)

        logger.debug(f"Broadcasted MQTT message from {hive_id} to {len(connected_clients)} clients")

    except Exception as e:
        logger.error(f"Error in MQTT bridge: {e}")

def mqtt_thread():
    """Run MQTT client in separate thread."""
    global mqtt_client

    mqtt_client = mqtt.Client(client_id=MQTT_CLIENT_ID, clean_session=True)
    mqtt_client.on_message = on_mqtt_message

    try:
        logger.info("MQTT Bridge: Connecting to broker...")
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
        mqtt_client.subscribe(MQTT_TOPIC, qos=1)
        logger.info("MQTT Bridge: Connected and subscribed")

        mqtt_client.loop_forever()

    except Exception as e:
        logger.error(f"MQTT Bridge error: {e}")

# Start MQTT bridge in background
mqtt_thread_obj = threading.Thread(target=mqtt_thread, daemon=True)
mqtt_thread_obj.start()

# ==========================================
# DATABASE OPERATIONS
# ==========================================

def get_db_connection():
    """Get SQLite database connection."""
    conn = sqlite3.connect(DATABASE_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def get_all_hives():
    """Get list of all hives."""
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("""
            SELECT hive_id, name, location, created_at, last_reading
            FROM hives
            ORDER BY last_reading DESC NULLS LAST
        """)

        hives = [dict(row) for row in cursor.fetchall()]
        conn.close()
        return hives

    except sqlite3.Error as e:
        logger.error(f"Database error getting hives: {e}")
        return []

def get_hive_latest(hive_id):
    """Get latest reading for a specific hive."""
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("""
            SELECT temperature, humidity, weight, battery_voltage, timestamp
            FROM readings
            WHERE hive_id = ?
            ORDER BY timestamp DESC
            LIMIT 1
        """, (hive_id,))

        row = cursor.fetchone()
        conn.close()

        return dict(row) if row else None

    except sqlite3.Error as e:
        logger.error(f"Database error getting latest reading: {e}")
        return None

def get_hive_history(hive_id, hours=24):
    """Get historical readings for a hive."""
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        timestamp_limit = datetime.now() - timedelta(hours=hours)

        cursor.execute("""
            SELECT timestamp, temperature, humidity, weight, battery_voltage
            FROM readings
            WHERE hive_id = ? AND timestamp > ?
            ORDER BY timestamp ASC
        """, (hive_id, timestamp_limit))

        readings = [dict(row) for row in cursor.fetchall()]
        conn.close()
        return readings

    except sqlite3.Error as e:
        logger.error(f"Database error getting history: {e}")
        return []

def get_hive_stats(hive_id, hours=24):
    """Get statistics for a hive."""
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        timestamp_limit = datetime.now() - timedelta(hours=hours)

        cursor.execute("""
            SELECT
                COUNT(*) as reading_count,
                AVG(temperature) as avg_temperature,
                MIN(temperature) as min_temperature,
                MAX(temperature) as max_temperature,
                AVG(humidity) as avg_humidity,
                MIN(humidity) as min_humidity,
                MAX(humidity) as max_humidity,
                AVG(weight) as avg_weight,
                MIN(weight) as min_weight,
                MAX(weight) as max_weight,
                AVG(battery_voltage) as avg_battery,
                MIN(battery_voltage) as min_battery
            FROM readings
            WHERE hive_id = ? AND timestamp > ?
        """, (hive_id, timestamp_limit))

        row = cursor.fetchone()
        conn.close()

        if row:
            return {
                'reading_count': row['reading_count'] or 0,
                'temperature': {
                    'average': round(row['avg_temperature'], 1) if row['avg_temperature'] else None,
                    'min': round(row['min_temperature'], 1) if row['min_temperature'] else None,
                    'max': round(row['max_temperature'], 1) if row['max_temperature'] else None,
                },
                'humidity': {
                    'average': round(row['avg_humidity'], 1) if row['avg_humidity'] else None,
                    'min': round(row['min_humidity'], 1) if row['min_humidity'] else None,
                    'max': round(row['max_humidity'], 1) if row['max_humidity'] else None,
                },
                'weight': {
                    'average': round(row['avg_weight'], 2) if row['avg_weight'] else None,
                    'min': round(row['min_weight'], 2) if row['min_weight'] else None,
                    'max': round(row['max_weight'], 2) if row['max_weight'] else None,
                },
                'battery': {
                    'average': round(row['avg_battery'], 2) if row['avg_battery'] else None,
                    'min': round(row['min_battery'], 2) if row['min_battery'] else None,
                }
            }
        return None

    except sqlite3.Error as e:
        logger.error(f"Database error getting stats: {e}")
        return None

# ==========================================
# ROUTES - HTML PAGES
# ==========================================

@app.route('/')
def dashboard():
    """Serve main dashboard."""
    return render_template('dashboard.html')

# ==========================================
# ROUTES - REST API
# ==========================================

@app.route('/api/hives', methods=['GET'])
def api_hives():
    """Get list of all hives."""
    hives = get_all_hives()
    return jsonify({
        'success': True,
        'count': len(hives),
        'data': hives
    })

@app.route('/api/hive/<hive_id>/latest', methods=['GET'])
def api_hive_latest(hive_id):
    """Get latest reading for a specific hive."""
    latest = get_hive_latest(hive_id)

    if latest is None:
        return jsonify({
            'success': False,
            'error': f'No readings found for hive {hive_id}'
        }), 404

    return jsonify({
        'success': True,
        'hive_id': hive_id,
        'data': latest
    })

@app.route('/api/hive/<hive_id>/history', methods=['GET'])
def api_hive_history(hive_id):
    """Get historical readings for a hive."""
    hours = request.args.get('hours', default=24, type=int)

    if hours < 1 or hours > 720:  # Max 30 days
        hours = 24

    readings = get_hive_history(hive_id, hours=hours)

    return jsonify({
        'success': True,
        'hive_id': hive_id,
        'hours': hours,
        'count': len(readings),
        'data': readings
    })

@app.route('/api/hive/<hive_id>/stats', methods=['GET'])
def api_hive_stats(hive_id):
    """Get statistics for a hive."""
    hours = request.args.get('hours', default=24, type=int)

    if hours < 1 or hours > 720:  # Max 30 days
        hours = 24

    stats = get_hive_stats(hive_id, hours=hours)

    if stats is None:
        return jsonify({
            'success': False,
            'error': f'No readings found for hive {hive_id}'
        }), 404

    return jsonify({
        'success': True,
        'hive_id': hive_id,
        'hours': hours,
        'data': stats
    })

@app.route('/api/status', methods=['GET'])
def api_status():
    """Get server status."""
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("SELECT COUNT(*) as count FROM hives")
        hive_count = cursor.fetchone()['count']

        cursor.execute("SELECT COUNT(*) as count FROM readings")
        reading_count = cursor.fetchone()['count']

        conn.close()

        return jsonify({
            'success': True,
            'status': 'running',
            'hives': hive_count,
            'readings': reading_count,
            'mqtt_clients': len(connected_clients),
            'timestamp': datetime.now().isoformat()
        })

    except Exception as e:
        logger.error(f"Error getting status: {e}")
        return jsonify({
            'success': False,
            'status': 'error',
            'error': str(e)
        }), 500

# ==========================================
# WEBSOCKET EVENTS
# ==========================================

@socketio.on('connect')
def handle_connect():
    """Handle WebSocket client connection."""
    connected_clients.add(request.sid)
    logger.info(f"Client connected: {request.sid} (Total: {len(connected_clients)})")

    # Send initial data
    hives = get_all_hives()
    emit('initial_data', {
        'hives': hives,
        'timestamp': datetime.now().isoformat()
    })

@socketio.on('disconnect')
def handle_disconnect():
    """Handle WebSocket client disconnection."""
    connected_clients.discard(request.sid)
    logger.info(f"Client disconnected: {request.sid} (Total: {len(connected_clients)})")

@socketio.on('request_update')
def handle_request_update(data):
    """Handle client request for specific data."""
    try:
        hive_id = data.get('hive_id')

        if not hive_id:
            emit('error', {'message': 'Missing hive_id'})
            return

        latest = get_hive_latest(hive_id)
        stats = get_hive_stats(hive_id)

        emit('data_update', {
            'hive_id': hive_id,
            'latest': latest,
            'stats': stats,
            'timestamp': datetime.now().isoformat()
        })

    except Exception as e:
        logger.error(f"Error in request_update: {e}")
        emit('error', {'message': str(e)})

# ==========================================
# ERROR HANDLERS
# ==========================================

@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors."""
    return jsonify({
        'success': False,
        'error': 'Not found'
    }), 404

@app.errorhandler(500)
def server_error(error):
    """Handle 500 errors."""
    logger.error(f"Server error: {error}")
    return jsonify({
        'success': False,
        'error': 'Internal server error'
    }), 500

# ==========================================
# MAIN
# ==========================================

if __name__ == '__main__':
    logger.info("=" * 50)
    logger.info("Starting BeezScale Flask server")
    logger.info("=" * 50)
    logger.info(f"Database: {DATABASE_PATH}")
    logger.info(f"Templates: {TEMPLATES_DIR}")
    logger.info("=" * 50)

    try:
        socketio.run(
            app,
            host='0.0.0.0',
            port=5000,
            debug=False,
            allow_unsafe_werkzeug=True
        )
    except KeyboardInterrupt:
        logger.info("Server stopped by user")
    except Exception as e:
        logger.critical(f"Fatal error: {e}")
        sys.exit(1)
