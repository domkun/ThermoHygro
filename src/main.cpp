#include <Arduino.h>
#include <DHT.h>
#include <Homie.h>

#define DHTPIN D2     // what digital pin we're connected to

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

const int DATA_INTERVAL_MILLIS = 60 * 1000UL;
const int ERROR_DELAY_MILLIS = 5 * 1000UL;

unsigned long lastDataSent = 0;

unsigned long lastDataRead = 0;

bool errorOccured = false;

HomieNode temperatureNode("temperature", "temperature");

HomieNode humidityNode("humidity", "humidity");

HomieNode heatIndexNode("heat-index", "heat-index");

bool shouldProcess(unsigned long currentMillis) {
  unsigned long lastSent = currentMillis - lastDataSent;
  unsigned long lastRead = currentMillis - lastDataRead;
  if (errorOccured) {
    if (lastRead >= ERROR_DELAY_MILLIS) {
      return true;
    }
  } else {
    if (lastSent >= DATA_INTERVAL_MILLIS || lastDataSent == 0) {
      return true;
    }
  }

  return false;
}

void setupHandler() {
  // setup the unit for each node
  Homie.setNodeProperty(temperatureNode, "unit", "°C", true);
  Homie.setNodeProperty(humidityNode, "unit", "%", true);
  Homie.setNodeProperty(heatIndexNode, "unit", "°C", true);

  // start sensor
  dht.begin();
}

void loopHandler() {
  if (shouldProcess(millis())) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

    // save the time when data was read as the DHT22 needs at least a 2 seconds
    // delay between reads
    lastDataRead = millis();

    // TODO: add debugging library or implement one
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Heat Index: ");
    Serial.print(heatIndex);
    Serial.println(" °C");

    if (Homie.setNodeProperty(temperatureNode, "temperature", String(temperature), true) &&
        Homie.setNodeProperty(humidityNode, "humidity", String(humidity), true) &&
        Homie.setNodeProperty(heatIndexNode, "heat-index", String(heatIndex), true)) {
      errorOccured = false;
      lastDataSent = millis();
    } else {
      errorOccured = true;
      Serial.println("Sending data failed");
    }
  }
}

void setup() {
  Homie.setFirmware("thermo-hygro", "0.1.0");
  Homie.registerNode(temperatureNode);
  Homie.registerNode(humidityNode);
  Homie.registerNode(heatIndexNode);
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}
