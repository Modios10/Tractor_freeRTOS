#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "CHUCHEL";
const char* password = "146}cR01";

const char* mqtt_server = "192.168.137.159";
const uint16_t mqtt_port = 1883;
const char* publish_topic = "motor/data";
const char* sub_topic_accel = "motor/control/accel";
const char* sub_topic_brake = "motor/control/brake";


#define RX_PIN D7
#define TX_PIN D6

SoftwareSerial stmSerial(RX_PIN, TX_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

String serialLine;

struct TelemetryData {
  long accel;
  long rpm;
  long vehicle_speed;
  long gear;
  long brake;
  bool valid;
};

static bool readField(const String& line, const char* key, long& value) {
  int searchFrom = 0;

  while (true) {
    int start = line.indexOf(key, searchFrom);
    if (start < 0) {
      return false;
    }

    bool tokenStartOk = (start == 0) || (line[start - 1] == ' ');
    int valueStart = start + (int)strlen(key);
    if (!tokenStartOk) {
      searchFrom = valueStart;
      continue;
    }

    int end = line.indexOf(' ', valueStart);
    if (end < 0) {
      end = line.length();
    }

    String token = line.substring(valueStart, end);
    token.trim();
    if (token.length() == 0) {
      return false;
    }

    value = token.toInt();
    return true;
  }
}

static bool parseTelemetryLine(const String& line, TelemetryData& data) {
  long accel = 0;
  long rpm = 0;
  long vehicle_speed = 0;
  long gear = 0;
  long brake = 0;

  if (!readField(line, "TH=", accel)) return false;
  if (!readField(line, "ENG=", rpm)) return false;
  if (!readField(line, "VEH=", vehicle_speed)) return false;
  if (!readField(line, "G=", gear)) return false;
  if (!readField(line, "BR=", brake)) return false;

  data.accel = accel;
  data.rpm = rpm;
  data.vehicle_speed = vehicle_speed;
  data.gear = gear;
  data.brake = brake;
  data.valid = true;
  return true;
}

//Function to connect to wifi network
static void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

//Function to reconnect to MQTT server if connection is lost
static void reconnectMqtt() {
  while (!client.connected()) {
    Serial.print("Conectando a MQTT...");
    String clientId = "STM_ESP8266-";
    clientId += String(ESP.getChipId(), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
    } else {
      Serial.print("fallo rc=");
      Serial.print(client.state());
      client.subscribe(sub_topic_accel);
      client.subscribe(sub_topic_brake);
      Serial.println(" reintentando en 2 s");
      delay(2000);
    }
  }
}


//Fnction to publish data to MQTT
static void publishTelemetry(const TelemetryData& data) {
  char payload[192];
  snprintf(payload, sizeof(payload),
           "{\"accel\":%ld,\"rpm\":%ld,\"v\":%ld,\"gear\":%ld,\"brake\":%ld}",
           data.accel, data.rpm, data.vehicle_speed, data.gear, data.brake);

  if (client.publish(publish_topic, payload)) {
    Serial.print("MQTT -> ");
    Serial.println(payload);
  } else {
    Serial.println("Error publicando MQTT");
  }
}


//Function to recieve data from MQTT and send it to STM32 via serial
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String mensaje;
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }

  // Si llega orden de acelerar, mandamos un prefijo "A:" por serial al STM32
  if (String(topic) == sub_topic_accel) {
    stmSerial.print("A:");
    stmSerial.println(mensaje);
    Serial.print("Comando Acelerador: "); Serial.println(mensaje);
  } 
  // Si llega orden de freno, mandamos un prefijo "B:" por serial
  else if (String(topic) == sub_topic_brake) {
    stmSerial.print("B:");
    stmSerial.println(mensaje);
    Serial.print("Comando Freno: "); Serial.println(mensaje);
  }
}




void setup() {
  Serial.begin(115200);
  stmSerial.begin(19200);
  stmSerial.listen();

  setupWifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  Serial.println();
  Serial.println("=========================================");
  Serial.println("STM32 -> ESP8266 -> Raspberry Pi");
  Serial.println("RX STM32: D7");
  Serial.println("Baud STM32: 19200");
  Serial.println("MQTT topic: motor/data");
  Serial.println("=========================================");
}


void loop() {
  if (!client.connected()) {
    reconnectMqtt();
  }
  client.loop();

  while (stmSerial.available() > 0) {
    char dato = (char)stmSerial.read();
    Serial.write(dato);

    if (dato == '\r') {
      continue;
    }

    if (dato == '\n') {
      TelemetryData data = {0, 0, 0, 0, 0, false};

      if (serialLine.length() > 0 && parseTelemetryLine(serialLine, data)) {
        // Imprimir valores parseados en la terminal serial
        Serial.print("Valores recibidos -> ");
        Serial.print("Accel: "); Serial.print(data.accel);
        Serial.print("  RPM: "); Serial.print(data.rpm);
        Serial.print("  Vel: "); Serial.print(data.vehicle_speed);
        Serial.print("  Gear: "); Serial.print(data.gear);
        Serial.print("  Brake: "); Serial.println(data.brake);

        publishTelemetry(data);
      } else if (serialLine.length() > 0) {
        Serial.print("\nLinea no reconocida: ");
        Serial.println(serialLine);
      }

      serialLine = "";
    } else {
      if (serialLine.length() < 160) {
        serialLine += dato;
      } else {
        serialLine = "";
      }
    }
  }

  delay(0);
}