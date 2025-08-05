#!/usr/bin/env python3
import time
import json
from influxdb import InfluxDBClient
import paho.mqtt.client as mqtt

# ----- CONFIG -----
INFLUX_DB      = 'sensors'
ALERT_MEAS     = [
    'alerts_accel0',
    'alerts_accel1',
    'alerts_humidity',
    'alerts_temp_dht',
    'alerts_humidity1',
    'alerts_temp_dht1',
    'alerts_gyro',
    'alerts_temp_mpu',
    'alerts_gyro1',
    'alerts_temp_mpu1'
]
MQTT_BROKER    = 'localhost'
MQTT_PORT      = 1883
FETCH_INTERVAL = 5   # seconds
# ------------------

# Connections
influx = InfluxDBClient(host='localhost', port=8086, database=INFLUX_DB)
mqttc  = mqtt.Client()
mqttc.connect(MQTT_BROKER, MQTT_PORT)

# Store the last processed timestamp
t_last = None


def fetch_alerts():
    """
    Return all new alert points in chronological order
    from all configured measurements. Each point gets a 'sensor'
    field set to the measurement name without 'alerts_'.
    """
    global t_last
    all_points = []

    for meas in ALERT_MEAS:
        # Derive field name and sensor label
        field = meas.replace('alerts_', '')

        # Build the InfluxQL query
        query = (
            f"SELECT time, host, \"{field}\" AS value "
            f"FROM \"{meas}\""
        )
        if t_last:
            query += f" WHERE time > '{t_last}'"
        query += " ORDER BY time ASC"

        result = influx.query(query)
        pts = list(result.get_points(measurement=meas))

        # Tag each point with its sensor name
        for pt in pts:
            pt['sensor'] = field
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
            value  = pt['value']

            # Publish one MQTT message per alert
            topic = f"control/{host}/{sensor}"
            cmd = {
                "command": "schedule_service",
                "sensor":  sensor,
                "value":   value,
                "time":    ts
            }
            payload = json.dumps(cmd)
            mqttc.publish(topic, payload)
            print(f"Published to {topic}: {payload}")

            # Update last processed timestamp
            t_last = ts

        time.sleep(FETCH_INTERVAL)
