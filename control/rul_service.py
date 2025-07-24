import time
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
client = InfluxDBClient(
    url="http://localhost:8086",
    token="YOUR_TOKEN",
    org="YOUR_ORG"
)
query_api = client.query_api()
write_api = client.write_api(write_options=SYNCHRONOUS)

BUCKET = "sensors"
HOST = "raspberrypi" 
def get_last_rul():
    flux = f'''
    from(bucket:"{BUCKET}")
      |> range(start: -30d)
      |> filter(fn: (r) => r._measurement == "rul" and r.host == "{HOST}")
      |> last()
    '''
    result = query_api.query(flux)
    for table in result:
        for record in table.records:
            return float(record.get_value())
    return 100.0

def main():
    rul = get_last_rul()
    last_time = None
    while True:
        # Costruisci filtro tempo
        if last_time:
            time_filter = f'|> range(start: {last_time})'
        else:
            time_filter = '|> range(start: -1m)'

        # Query per punti nuovi con accel>11
        flux = (
            f'from(bucket:"{BUCKET}") '
            f'{time_filter} '
            f'|> filter(fn: (r) => r._measurement == "mqtt_consumer" '
            f'and r.host == "{HOST}" '
            f'and r._field == "accel" '
            f'and r._value > 11)'
        )
        result = query_api.query(flux)
        # Conta quanti record
        count = sum(1 for table in result for _ in table.records)
        if count > 0:
            rul -= count
            p = Point("rul").tag("host", HOST).field("rul", rul)
            write_api.write(BUCKET, record=p)
            print(f"Decremented RUL by {count}, new RUL = {rul}")
        last_time = 'now()'
        time.sleep(10)

if __name__ == "__main__":
    main()
