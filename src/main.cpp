// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain
#include <Arduino.h>
#include <DHT.h>
#include <Homie.h>

#define DHTPIN D2     // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);


const int TEMPERATURE_INTERVAL = 30;

unsigned long lastTemperatureSent = 0;

HomieNode temperatureNode("temperature", "temperature");

HomieNode humidityNode("humidity", "humidity");

HomieNode heatIndexNode("heat-index", "heat-index");

void setupHandler() {
  Homie.setNodeProperty(temperatureNode, "unit", "°C", true);
  Homie.setNodeProperty(humidityNode, "unit", "%", true);
  Homie.setNodeProperty(heatIndexNode, "unit", "°C");
  dht.begin();
}

void loopHandler() {
  if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0) {
    float temperature = dht.readTemperature();
    //float humidity = dht.readHumidity();
    //float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    //Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Heat Index: ");
    //Serial.print(heatIndex);
    Serial.println(" °C");

    /*if (Homie.setNodeProperty(temperatureNode, "temperature", String(temperature), true) &&
        Homie.setNodeProperty(humidityNode, "humidity", String(humidity), true) &&
        Homie.setNodeProperty(heatIndexNode, "heat-index", String(heatIndex), true)) { */
    if (Homie.setNodeProperty(temperatureNode, "temperature", String(temperature), true)) {
      lastTemperatureSent = millis();
    } else {
      Serial.println("Sending data failed");
    }
  }
}

void setup() {


  Homie.setFirmware("oskar-temperature", "1.0.0");
  Homie.registerNode(temperatureNode);
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();


}

void loop() {
  Homie.loop();
}
