import json
import time
from datetime import datetime
import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# CONFIGURACIÓN DE CONEXIONES (MQTT & INFLUXDB)
BROKER_HOST = "192.168.137.159"
BROKER_PORT = 1883
MQTT_TOPIC = "motor/data"

# Configuración InfluxDB
TOKEN = "D9VbhaYZfbrB6V6Lm41QmMs6YyWSRalkW4wHWYsAnZt93P28BHwwWKmo6gpndhef0HpqT1QrlU774-JeiN-MbQ=="
ORG = "FacultadIng"
BUCKET = "telemetria_tractor"
URL = "http://localhost:8086"


# INICIALIZACIÓN DE CLIENTE INFLUXDB
influx_client = InfluxDBClient(url=URL, token=TOKEN, org=ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)


# FUNCIONES DE PARSEO Y FORMATEO
def first_present(data, *keys):
    for key in keys:
        if key in data and data[key] is not None:
            return data[key]
    return None


def to_number(value):
    try:
        if isinstance(value, bool):
            return None
        if isinstance(value, (int, float)):
            return value
        text = str(value).strip()
        if text == "":
            return None
        if "." in text:
            return float(text)
        return int(text)
    except (TypeError, ValueError):
        return None


# CALLBACKS MQTT
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[MQTT] Conectado exitosamente al broker")
        # Nos suscribimos a ambos canales
        client.subscribe("motor/data")
        client.subscribe("tractor/conductor")
        print("[MQTT] Suscrito a telemetría y cámara")
    else:
        print(f"[MQTT ERROR] Fallo de conexión, código: {rc}")


# Este callback se ejecuta cada vez que llega un mensaje al tópico suscrito
def on_message(client, userdata, msg):
    try:
        # Decodificar el mensaje JSON entrante
        payload = msg.payload.decode("utf-8", errors="replace")
        data = json.loads(payload)

        # extract mesages comming from esp8266 (motor data)
        if msg.topic == MQTT_TOPIC:    
            accel = to_number(first_present(data, "accel", "th", "throttle", "throttle_pct"))
            rpm = to_number(first_present(data, "rpm", "eng", "engine_rpm", "engine_speed"))
            vehicle_speed = to_number(first_present(data, "v", "veh", "vehicle_speed", "speed"))
            gear = to_number(first_present(data, "gear", "g"))
            brake = to_number(first_present(data, "brake", "br"))

            # Validación de campos obligatorios
            if accel is None or rpm is None or vehicle_speed is None or gear is None:
                print(f"[WARN] JSON incompleto o inesperado omitido: {data}")
                return
            
            if brake is None:
                brake = 0

            # Crear el punto con el formato exacto para InfluxDB
            punto = Point("monitoreo_tractor") \
                .tag("id_maquina", "tractor_01") \
                .tag("sensor_set", "motor_v1") \
                .field("aceleracion", float(accel)) \
                .field("rpm", int(rpm)) \
                .field("velocidad", float(vehicle_speed)) \
                .field("marcha", int(gear)) \
                .field("freno", float(brake))

            # Enviar de forma síncrona a InfluxDB
            write_api.write(bucket=BUCKET, org=ORG, record=punto)

            # Log rápido en la consola de la Raspberry para monitoreo continuo
            timestamp = datetime.now().strftime("%H:%M:%S")
            print(f"[{timestamp}][OK] InfluxDB <- RPM:{rpm} | Vel:{vehicle_speed}km/h | Marcha:{gear}")


        # Messages recieved by raspy 2 (camara)
        elif msg.topic == "tractor/conductor":
            estado_texto = data.get("estado", "Desconocido")
            nivel_alerta = data.get("alerta", 0)

            # Creamos un punto nuevo para el conductor
            punto_camara = Point("estado_conductor") \
                .tag("id_maquina", "tractor_01") \
                .field("estado_facial", estado_texto) \
                .field("alerta_peligro", int(nivel_alerta))

            write_api.write(bucket=BUCKET, org=ORG, record=punto_camara)
            
            timestamp = datetime.now().strftime("%H:%M:%S")
            print(f"[{timestamp}][OK] InfluxDB <- Conductor: {estado_texto} | Alerta: {nivel_alerta}")


    except json.JSONDecodeError:
        print(f"[ERROR] El mensaje MQTT no contiene un JSON válido: {msg.payload!r}")
    except Exception as e:
        print(f"[ERROR INESPERADO]: {e}")


# FLUJO PRINCIPAL
def main():
    print("--- Iniciando Bridge de Telemetría (MQTT -> InfluxDB) ---")

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    try:
        client.connect(BROKER_HOST, BROKER_PORT, 60)
    except Exception as e:
        print(f"[ERROR] No se pudo conectar al broker MQTT: {e}")
        return

    # loop_forever() bloquea el script de manera eficiente manteniendo la escucha activa
    # y procesando los eventos en el hilo principal sin consumir CPU de más.
    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print("\n--- Script finalizado por el usuario (Ctrl+C) ---")
    finally:
        print("Cerrando conexiones...")
        client.disconnect()
        influx_client.close()
        print("Conexiones cerradas de forma segura.")


if __name__ == "__main__":
    main()
