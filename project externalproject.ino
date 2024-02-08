#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
// Naming the variables
WiFiClient client;
HTTPClient http;
String url;
String API = "22YJBLY6TX86MN68";
int soilMoisturePin = A0; // Analog pin connected to the soil moisture sensor
int dhtPin = 4; // Digital pin connected to the DHT sensor
int ledPin = 5; // Digital pin connected to the LED
String fieldSoilMoisture = "1";
String fieldHumidity = "2";
String fieldTemperature = "3";
String fieldPredictedSoilMoisture = "4";
DHT dht(dhtPin, DHT11);

// Creating the functions for the tasks
void connectToWiFi();
void sendHttpRequest(int data, String fieldNo);
void soilMoisture();
void humiditySensor();
int readFieldFromThingSpeak(String fieldNo);

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT); // Set the LED pin as output
  connectToWiFi();
}

void loop() {
  soilMoisture();
  delay(5000); // Adjust the delay as needed
  humiditySensor();
  delay(5000); // Adjust the delay as needed

  // Read predicted soil moisture from Field 4
  int predictedSoilMoisture = readFieldFromThingSpeak(fieldPredictedSoilMoisture);
  Serial.print("Predicted Soil Moisture: ");
  Serial.print(predictedSoilMoisture);
  Serial.println("%");

  // Check and control the LED based on moisture conditions
  int currentMoisture = analogRead(soilMoisturePin);
  int moisturePercentage = map(currentMoisture, 0, 1023, 100, 0);
  Serial.print("Current Soil Moisture Level: ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  if (moisturePercentage < predictedSoilMoisture) {
    digitalWrite(ledPin, HIGH); // Turn on the LED
    Serial.println("LED turned ON");
  } else {
    digitalWrite(ledPin, LOW); // Turn off the LED
    Serial.println("LED turned OFF");
  }
}

void connectToWiFi() {
  const char* ssid = "1234567890";
  const char* password = "1234567890";

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void sendHttpRequest(int data, String fieldNo) {
  url = "http://api.thingspeak.com/update?api_key=";
  url += API;
  url += "&field";
  url += fieldNo;
  url += "=";
  url += data;

  http.begin(client, url);
  Serial.print("Waiting for the response");
  int httpcode = http.GET();
  if (httpcode > 0) {
    Serial.print("Data sent successfully");
  } else {
    Serial.print("Error in Sending Data");
  }
  http.end();
}

int readFieldFromThingSpeak(String fieldNo) {
  url = "http://api.thingspeak.com/channels/2383020/fields/" + fieldNo + ".json?results=2";
  http.begin(client, url);

  Serial.print("Reading data from ThingSpeak...");
  int httpcode = http.GET();

  if (httpcode == 200) {
    String response = http.getString();
    Serial.println("Response from ThingSpeak:");
    Serial.println(response);

    // Parse the JSON response
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);

    // Check for parsing errors
    if (error) {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.c_str());
      return 0;
    }

    // Extract the value from the JSON document
    int value = doc["feeds"][0]["field" + fieldNo].as<int>();
    return value;
  } else {
    Serial.print("Error reading data from ThingSpeak. Status code: ");
    Serial.println(httpcode);
    return 0;
  }

  http.end();
}


void soilMoisture() {
  int soilMoistureValue = analogRead(soilMoisturePin);
  int moisturePercentage = map(soilMoistureValue, 0, 1023, 100, 0);
  
  // Print the moisture level
  Serial.print("Soil Moisture Level: ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  // Send data to ThingSpeak
  sendHttpRequest(moisturePercentage, fieldSoilMoisture);
}

void humiditySensor() {
  DHT dht(dhtPin, DHT11);
  //float temperature = dht.readTemperature();
  //float humidity = dht.readHumidity();
float temperature=35;
float humidity=45;
  // Check if any reads failed
  if (!isnan(temperature) && !isnan(humidity)) {
    // Print temperature and humidity
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    // Send data to ThingSpeak
    sendHttpRequest(temperature, fieldTemperature);
    sendHttpRequest(humidity, fieldHumidity);

  }
  else {
    Serial.println("Failed to read from DHT sensor!");
  };
}
