#include <Arduino.h>
#include <DHT.h>
#include <Homie.h>

#define DHTPIN D2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define NIGHTLIGHTPIN1 D6

DHT dht(DHTPIN, DHTTYPE);

const int DATA_INTERVAL_MILLIS = 60 * 1000UL;
const int ERROR_DELAY_MILLIS = 5 * 1000UL;

unsigned long lastDataSent = 0;

unsigned long lastDataRead = 0;

bool errorOccured = false;

HomieNode temperatureNode("temperature", "temperature");

HomieNode humidityNode("humidity", "humidity");

HomieNode heatIndexNode("heat-index", "heat-index");

HomieNode nightLightNode("light", "switch");

bool shouldProcess(unsigned long currentMillis) {
  unsigned long lastSent = currentMillis - lastDataSent;
  unsigned long lastRead = currentMillis - lastDataRead;
  if (errorOccured) {
    if (lastRead >= ERROR_DELAY_MILLIS) {
      Serial.print("Triggering processing of data, cause: ERROR, ");
      Serial.println(lastRead);
      return true;
    }
  } else {
    if (lastSent >= DATA_INTERVAL_MILLIS || lastDataSent == 0) {
      Serial.print("Triggering processing of data, cause: INTERVAL");
      Serial.print(", lastSent: ");
      Serial.print(lastSent);
      Serial.print(", lastDataSent: ");
      Serial.println(lastDataSent);
      return true;
    }
  }

  return false;
}

void sendData(float temperature, float humidity, float heatIndex) {
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

void setupHandler() {
  // setup the unit for each node
  Homie.setNodeProperty(temperatureNode, "unit", "°C", true);
  Homie.setNodeProperty(humidityNode, "unit", "%", true);
  Homie.setNodeProperty(heatIndexNode, "unit", "°C", true);

  Homie.setNodeProperty(nightLightNode, "on", "false");

  // start sensor
  dht.begin();
}

bool lightOnHandler(String value) {
  if (value == "true") {
    digitalWrite(NIGHTLIGHTPIN1, HIGH);
    Homie.setNodeProperty(nightLightNode, "on", "true"); // Update the state of the light
    Serial.println("Light is on");
  } else if (value == "false") {
    digitalWrite(NIGHTLIGHTPIN1, LOW);
    Homie.setNodeProperty(nightLightNode, "on", "false");
    Serial.println("Light is off");
  } else {
    return false;
  }

  return true;
}

void loopHandler() {
  if (shouldProcess(millis())) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // save the time when data was read as the DHT22 needs at least a 2 seconds
    // delay between reads
    lastDataRead = millis();
    bool readOk = !isnan(temperature) && !isnan(humidity);

    if (readOk) {
      Serial.println("Succesful read data from sensor");
      float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
      sendData(temperature, humidity, heatIndex);
    } else {
      errorOccured = true;
      Serial.println("Error occured while reading data from sensor");
    }
  }
}

void setup() {
  // setup night light pin
  pinMode(NIGHTLIGHTPIN1, OUTPUT);
  digitalWrite(NIGHTLIGHTPIN1, LOW);

  Homie.setFirmware("thermo-hygro", "0.1.0");
  Homie.enableBuiltInLedIndicator(false);

  nightLightNode.subscribe("on", lightOnHandler);

  Homie.registerNode(temperatureNode);
  Homie.registerNode(humidityNode);
  Homie.registerNode(heatIndexNode);

  Homie.registerNode(nightLightNode);

  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}
