#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define DHT_PIN 2
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

char* ssid = "SSID_WIFI";
char* password = "Pass_WIFI";
char* mqtt_server = "localhost";
int mqtt_port = 1883;
char* mqtt_user = "user";
char* mqtt_password = "pass";
char* mqtt_topic = "sensor/humedad_y_temperatura";

WiFiClient espClient;
PubSubClient client(espClient);

char humString[10];
char tempString[10];

void setup() {
  Serial.begin(9600);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Conectando al servidor MQTT...");
    if (client.connect("arduino-client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado al servidor MQTT");
    } else {
      Serial.print("Error de conexión MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando nuevamente en 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error al leer el sensor DHT");
  } else {
    Serial.print("Humedad: ");
    Serial.print(humidity);
    Serial.print("%\t");
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println("°C");

    snprintf(humString, sizeof(humString), "%.2f", humidity);
    snprintf(tempString, sizeof(tempString), "%.2f", temperature);

    char mqttHumTopic[50];
    char mqttTempTopic[50];
    strcpy(mqttHumTopic, mqtt_topic);
    strcpy(mqttTempTopic, mqtt_topic);
    strcat(mqttHumTopic, "/humedad");
    strcat(mqttTempTopic, "/temperatura");

    client.publish(mqttHumTopic, humString);
    client.publish(mqttTempTopic, tempString);
  }

  client.loop();
  delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {

}