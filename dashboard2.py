from flask import Flask, request, jsonify
import paho.mqtt.client as mqtt

app = Flask(__name__)

# Configuración MQTT (Misma IP de tu broker)
BROKER_HOST = "192.168.137.159"
BROKER_PORT = 1883
TOPIC_ACCEL = "motor/control/accel"
TOPIC_BRAKE = "motor/control/brake"

client = mqtt.Client()

def conectar_mqtt():
    try:
        client.connect(BROKER_HOST, BROKER_PORT, 60)
        client.loop_start()
        print(f"[API] Conectado a MQTT en {BROKER_HOST}")
    except Exception as e:
        print(f"[API ERROR] No se pudo conectar a MQTT: {e}")

# Este es el "Endpoint" que Grafana va a contactar
@app.route('/api/control', methods=['POST'])
def control_tractor():
    data = request.get_json(force=True)  # Asegura que se intente parsear JSON incluso si el header no es correcto
    print(f"[API] Orden recibida desde Grafana: {data}")

    if not data:
        return jsonify({"error": "No JSON payload"}), 400

    # Si Grafana manda el valor del slider de aceleración
    if 'aceleracion' in data:
        client.publish(TOPIC_ACCEL, str(data['aceleracion']))
        print("Valores publicados via MQTT")

    # Si Grafana manda el clic del botón de freno
    if 'freno' in data:
        client.publish(TOPIC_BRAKE, str(data['freno']))

    return jsonify({"status": "comando en ruta al tractor"}), 200 # Respuesta exitosa

if __name__ == '__main__':
    conectar_mqtt()
    print("--- Iniciando API de Control para Grafana ---")
    # Escucha en todas las interfaces de red en el puerto 5000
    app.run(host='0.0.0.0', port=5000)
