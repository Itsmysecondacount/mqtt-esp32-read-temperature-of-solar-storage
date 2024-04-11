#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Define your WiFi network credentials
const char* ssid = "xxxxxx"; // Hidden SSID
const char* password = "xxxxxx"; // Hidden Password

// Define the MQTT server address
const char* mqttServer = "192.168.1.40";
const int mqttPort = 1883;

// Configure the static IP
IPAddress ip(192, 168, 1, 37);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Define the MQTT channel details
const char* mqttTopic1 = "jamk/tank/temperature1";
const char* mqttTopic2 = "jamk/tank/temperature2";

// Define the MQTT client name
const char* mqttClientName = "ESP32Client-SolarTank-V2";

// Pin connected to the OneWire bus
const int oneWireBus = 22;  // Change this to the pin you are using

// Initialize the OneWire bus
OneWire oneWire(oneWireBus);

// Initialize the DS18B20 sensor
DallasTemperature sensors(&oneWire);

// Initialize the WiFi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Initialize the serial port
  Serial.begin(115200);

  // Set the static IP
  WiFi.config(ip, gateway, subnet);

  // Connect to the WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi network");
  Serial.println(WiFi.localIP());

  // Set up the MQTT server
  client.setServer(mqttServer, mqttPort);

  // Initialize the DS18B20 sensors
  sensors.begin();
}

void loop() {
  // Connect to the MQTT server
  if (!client.connected()) {
    reconnect();
  }

  // Publish the temperatures on the MQTT channels
  publishTemperatures();
}

void reconnect() {
  // Reconnect to the MQTT server
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(mqttClientName)) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" Trying again in 5 seconds...");
      delay(5000);
    }
  }
}

void publishTemperatures() {
  // Request the sensors to send the temperature
  sensors.requestTemperatures();
  // Read and print the temperature from each sensor and publish it on the corresponding MQTT channel
  for (int i = 0; i < sensors.getDeviceCount(); i++) {
    float temperature = sensors.getTempCByIndex(i);

    // Build the MQTT channel name
    char topic[50];
    snprintf(topic, sizeof(topic), "jamk/tank/temperature%d", i + 1);

    // Publish the temperature on the MQTT channel
    client.publish(topic, String(temperature).c_str());

    // Print to the serial port
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(" - MQTT Channel: ");
    Serial.print(topic);
    Serial.print(", Temperature: ");
    Serial.println(temperature, 1);
    Serial.println(" °C");
  }

  // Wait 1 minute before reading the temperatures again
  delay(60000);
}