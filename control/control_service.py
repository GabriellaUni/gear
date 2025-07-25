import time
import json
from influxdb import InfluxDBClient
import paho.mqtt.client as mqtt

# Config
INFLUX_DB = 'sensors'
ALERT_MEAS = 'alerts'
MQTT_BROKER = 'localhost'
MQTT_PORT = 1883
FETCH_INTERVAL = 10  # secondi

# Connessioni
influx = InfluxDBClient(host='localhost', port=8086, database=INFLUX_DB)
mqttc = mqtt.Client()
mqttc.connect(MQTT_BROKER, MQTT_PORT)

# Tiene traccia dell'ultimo timestamp elaborato
last_time = None

def fetch_alerts():
    global last_time
    query = f'SELECT time, accel, host FROM "{ALERT_MEAS}"'
    if last_time:
        query += f" WHERE time > '{last_time}'"
    query += ' ORDER BY time ASC'
    result = influx.query(query)
    points = list(result.get_points(measurement=ALERT_MEAS))
    return points

if __name__ == '__main__':
    while True:
        alerts = fetch_alerts()
        for pt in alerts:
            ts = pt['time']
            host = pt.get('host', 'unknown')
            accel = pt.get('accel')
            # Business rule: tutti gli alert accel>11 generano un comando
            topic = f'control/{host}'
            cmd = {'command': 'schedule_service', 'accel': accel, 'time': ts}
            payload = json.dumps(cmd)
            mqttc.publish(topic, payload)
            print(f'Published to {topic}: {payload}')
            last_time = ts
        time.sleep(FETCH_INTERVAL)
