#include <WiFi.h>
#include <PubSubClient.h>

// ---------- Wi-Fi ----------
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ---------- MQTT ----------
const char* MQTT_SERVER = "YOUR_MQTT_BROKER";
const int MQTT_PORT = 1883;

const char* MQTT_TOPIC = "smartwaste/bin";

// ---------- ESP32 ----------
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ---------- Ultrasonic Sensor ----------
#define TRIG_PIN 5
#define ECHO_PIN 18

// Bin dimensions
const float BIN_HEIGHT_CM = 40.0;

// ---------- Functions ----------

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");

    String clientId = "ESP32_WasteBin_";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, state=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  float distance = duration * 0.0343 / 2;

  return distance;
}

int calculateFillLevel(float distance) {

  if (distance < 0) {
    return -1;
  }

  float fillPercentage =
      ((BIN_HEIGHT_CM - distance) / BIN_HEIGHT_CM) * 100;

  fillPercentage = constrain(fillPercentage, 0, 100);

  return (int)fillPercentage;
}

void setup() {

  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  connectWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  Serial.println("Smart Waste Management ESP32 Started!");
}

void loop() {

  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();

  float distance = getDistance();

  int fillLevel = calculateFillLevel(distance);

  if (fillLevel >= 0) {

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    Serial.print("Bin Fill Level: ");
    Serial.print(fillLevel);
    Serial.println("%");

    // Create MQTT JSON message
    String payload = "{";
    payload += "\"bin_id\":\"BIN001\",";
    payload += "\"fill_level\":";
    payload += String(fillLevel);
    payload += ",";
    payload += "\"distance\":";
    payload += String(distance);
    payload += "}";

    Serial.print("Sending MQTT: ");
    Serial.println(payload);

    mqttClient.publish(MQTT_TOPIC, payload.c_str());
  }

  delay(10000);
}
