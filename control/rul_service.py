import time
from influxdb import InfluxDBClient

# Configura connessione v1
client = InfluxDBClient(host='localhost', port=8086, database='sensors')
HOST = 'pi'  # il tag host che usi

def get_last_rul():
    # Legge l'ultimo valore di rul, se esiste
    result = client.query(
        'SELECT last("rul") FROM "rul" WHERE "host"=\'%s\'' % HOST
    )
    points = list(result.get_points(measurement='rul'))
    if points:
        return float(points[0]['last'])
    return 100.0

def main():
    rul = get_last_rul()
    print(f"Starting RUL = {rul}")
    while True:
        # Conta i punti accel>11 degli ultimi 60s
        result = client.query(
            'SELECT count("accel") FROM "mqtt_consumer" '
            'WHERE time > now() - 1m '
            'AND "host"=\'%s\' AND "accel" > 11' % HOST
        )
        points = list(result.get_points())
        count = int(points[0]['count']) if points else 0
        print(f"Found {count} accel>11 events")
        if count > 0:
            rul -= count
            # Scrivi il nuovo RUL
            json_body = [{
                "measurement": "rul",
                "tags": {"host": HOST},
                "fields": {"rul": rul}
            }]
            client.write_points(json_body)
            print(f"Written new RUL = {rul}")
        time.sleep(10)

if __name__ == "__main__":
    main()
