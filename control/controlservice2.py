#!/usr/bin/env python3
import time
import json
from influxdb import InfluxDBClient
import paho.mqtt.client as mqtt

# ----- CONFIG -----
INFLUX_DB      = 'sensors'
ALERT_MEAS     = ['alerts_accel0', 'alerts_accel1']
MQTT_BROKER    = 'localhost'
MQTT_PORT      = 1883
FETCH_INTERVAL = 5   # seconds
# ------------------

# Connections
influx = InfluxDBClient(host='localhost', port=8086, database=INFLUX_DB)
mqttc  = mqtt.Client()
mqttc.connect(MQTT_BROKER, MQTT_PORT)

# Store the last processed timestamp
last_time = None

def fetch_alerts():
    """
    Return all new alert points in chronological order
    from both measurements. Each point gets a 'sensor'
    field set to 'accel0' or 'accel1'.
    """
    global last_time
    all_points = []

    for meas in ALERT_MEAS:
        # Choose the field name based on measurement
        field = 'accel' if meas.endswith('accel0') else 'accel1'

        # Build the InfluxQL query
        query = (
            f"SELECT time, host, \"{field}\" AS accel "
            f"FROM \"{meas}\""
        )
        if last_time:
            query += f" WHERE time > '{last_time}'"
        query += " ORDER BY time ASC"

        result = influx.query(query)
        pts = list(result.get_points(measurement=meas))

        # Tag each point with its sensor name
        for pt in pts:
            pt['sensor'] = 'accel0' if meas.endswith('accel0') else 'accel1'
        all_points.extend(pts)

    # Sort globally by time to merge streams
    all_points.sort(key=lambda p: p['time'])
    return all_points

if __name__ == '__main__':
    while True:
        alerts = fetch_alerts()
        for pt in alerts:
            ts     = pt['time']
            host   = pt.get('host', 'unknown')
            sensor = pt['sensor']
            accel  = pt['accel']

            # Publish one MQTT message per alert
            topic = f"control/{host}/{sensor}"
            cmd = {
                "command": "schedule_service",
                "sensor":  sensor,
                "accel":   accel,
                "time":    ts
            }
            payload = json.dumps(cmd)
            mqttc.publish(topic, payload)
            print(f"Published to {topic}: {payload}")

            # Update last_time after each point
            last_time = ts

        time.sleep(FETCH_INTERVAL)
