#!/usr/bin/env python3
"""
Detector de rostros + MQTT + Temporizador Anti-Spam
"""
import json
from picamera2 import Picamera2
import cv2
import time

try:
    import paho.mqtt.client as mqtt
    MQTT_DISPONIBLE = True
except ImportError:
    MQTT_DISPONIBLE = False

FACE_CASCADE = cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
EYE_CASCADE = cv2.data.haarcascades + "haarcascade_eye.xml"
SMILE_CASCADE = cv2.data.haarcascades + "haarcascade_smile.xml"

def cargar_cascadas():
    face = cv2.CascadeClassifier(FACE_CASCADE)
    eye = cv2.CascadeClassifier(EYE_CASCADE)
    smile = cv2.CascadeClassifier(SMILE_CASCADE)
    if face.empty() or eye.empty() or smile.empty():
        return None, None, None
    return face, eye, smile

def detectar_expresion(roi_gris, roi_color, eye_cascade, smile_cascade):
    expresion = "Normal"
    color = (0, 255, 0)
    
    ojos = eye_cascade.detectMultiScale(roi_gris, scaleFactor=1.1, minNeighbors=4, minSize=(15, 15))
    roi_inferior = roi_gris[roi_gris.shape[0]//2:, :]
    sonrisas = smile_cascade.detectMultiScale(roi_inferior, scaleFactor=1.8, minNeighbors=20, minSize=(20, 20))
    
    if len(ojos) >= 2:
        if len(sonrisas) > 0:
            expresion = "SONRIENDO"
            color = (0, 255, 255)
    else:
        expresion = "Ojos cerrados"
        color = (0, 0, 255)
        
    for (ex, ey, ew, eh) in ojos:
        cv2.rectangle(roi_color, (ex, ey), (ex+ew, ey+eh), (255, 0, 0), 2)
        
    return expresion, color

def main():
    face_cascade, eye_cascade, smile_cascade = cargar_cascadas()
    if face_cascade is None: return
        
    usar_mqtt = False
    if MQTT_DISPONIBLE:
        try:
            try:
                client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
            except AttributeError:
                client = mqtt.Client()
            
            client.connect("localhost", 1883)
            client.loop_start()
            usar_mqtt = True
            print("[OK] Conectado al servidor Mosquitto")
        except Exception as e:
            print(f"[ERROR MQTT] No se pudo conectar: {e}")
    
    # --- CONFIGURACIÓN DEL TEMPORIZADOR ---
    estado_anterior = "Normal" 
    ultimo_envio = 0  # Guarda el segundo exacto en el que se mandó el último mensaje
    tiempo_espera = 3.0  # TIEMPO EN SEGUNDOS PARA EVITAR REPETICIONES (puedes cambiarlo si quieres)
    # ---------------------------------------

    cam = Picamera2()
    config = cam.create_preview_configuration(main={"size": (1280, 960), "format": "RGB888"})
    cam.configure(config)
    cam.start()
    time.sleep(1)
    
    while True:
        frame_rgb = cam.capture_array()
        frame_bgr = cv2.cvtColor(frame_rgb, cv2.COLOR_RGB2BGR) 
        gris = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2GRAY)
        
        rostros = face_cascade.detectMultiScale(gris, scaleFactor=1.1, minNeighbors=5, minSize=(60, 60))
        
        if len(rostros) == 0:
            estado_anterior = "Normal"

        for i, (x, y, w, h) in enumerate(rostros):
            roi_gris = gris[y:y+h, x:x+w]
            roi_color = frame_bgr[y:y+h, x:x+w]
            expresion, color = detectar_expresion(roi_gris, roi_color, eye_cascade, smile_cascade)
            
            # --- ENVÍO DE MENSAJES CON CONTROL DE TIEMPO ---
            if usar_mqtt and i == 0:
                tiempo_actual = time.time()
                
                # Solo evalúa enviar si ya pasaron más de 3 segundos desde el último mensaje
                if (tiempo_actual - ultimo_envio) > tiempo_espera:
                    if expresion != estado_anterior and expresion != "Normal":
                        if expresion == "SONRIENDO":
                            client.publish("raspberry/expresiones", "sonrisa")
                            print("[MQTT] --> mensaje publicado: sonrisa")
                        elif expresion == "Ojos cerrados":
                            client.publish("raspberry/expresiones", "ojos cerrados")
                            print("[MQTT] --> mensaje publicado: ojos cerrados")
                        
                        estado_anterior = expresion 
                        ultimo_envio = tiempo_actual  # Reinicia el reloj del temporizador
            # -----------------------------------------------

            cv2.rectangle(frame_bgr, (x, y), (x+w, y+h), color, 3)
            cv2.putText(frame_bgr, f"Rostro {i+1}", (x, y-30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2)
            cv2.putText(frame_bgr, expresion, (x, y-5), cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2)
            
        cv2.imshow("Detector + MQTT Anti-Spam", frame_bgr)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
            
    cam.stop()
    cv2.destroyAllWindows()
    if usar_mqtt:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()