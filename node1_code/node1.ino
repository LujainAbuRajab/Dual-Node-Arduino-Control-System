#include <DHT.h>
#include <ArduinoJson.h>
#include <Servo.h>

#define DHTPIN 21        // Pin where the DHT11 sensor is connected
#define DHTTYPE DHT11   // DHT 11 or DHT22
#define POT_PIN A0
#define SERVO_PIN 4
#define SWITCH_DIR 23     
#define SWITCH_START 28 

DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor
Servo servo;

void setup() {
  Serial1.begin(9600);     // Start Serial1 for communication
  Serial.begin(9600);       // Start Serial Monitor for debugging
  dht.begin();              // Initialize the DHT sensor
  servo.attach(SERVO_PIN);
  pinMode(SWITCH_DIR, INPUT);
  pinMode(SWITCH_START, INPUT);
}

void loop() {
  // Read values from DHT sensor
  float temperature = dht.readTemperature(); // Read temperature as Celsius
  float humidity = dht.readHumidity();       // Read humidity
  int potValue = analogRead(POT_PIN);
  int switchDir = digitalRead(SWITCH_DIR);
  int switchStart = digitalRead(SWITCH_START);


  // Check if the readings are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Create a JSON object
  StaticJsonDocument<200> doc; // Adjust size based on your needs
  doc["node"] = 1; // Identifier for this node
  doc["temp"] = temperature;
  doc["hum"] = humidity;
  doc["pot"] = potValue;
  doc["Dir"] = switchDir;
  doc["Start"] = switchStart;

  // Serialize the JSON object to a string
  char jsonOutput[2048];
  serializeJson(doc, jsonOutput);

  // Send the JSON string
  Serial1.println(jsonOutput);

  // Debugging Output
  Serial.print("Node 1 Sent JSON: ");
  Serial.println(jsonOutput);

  // Check for incoming data from Node 2
  if (Serial1.available()) {
    String receivedData = Serial1.readStringUntil('\n');

    // Create a JSON document to hold the parsed data
    StaticJsonDocument<200> docReceived;
    DeserializationError error = deserializeJson(docReceived, receivedData);

    // Check for errors in parsing
    if (!error) {
      int nodeID = docReceived["node"];
      float temperatureReceived = docReceived["temperature"];
      float humidityReceived = docReceived["humidity"];
      int potValueReceived = docReceived["pot"];
      int ldrValue = docReceived["ldr"];

      // Print the received values
      Serial.print("Node 2 JSON Received: ");
      Serial.print(receivedData);
      Serial.print(" | Node ID: ");
      Serial.print(nodeID);
      Serial.print(" | Temperature: ");
      Serial.print(temperatureReceived);
      Serial.print(" | Humidity: ");
      Serial.println(humidityReceived);
      Serial.print(" | LDR Value: ");
      Serial.println(ldrValue);
      servo.write(map(potValueReceived, 0, 1023, 0, 180));
    } else {
      Serial.print("Failed to parse JSON from Node 2: ");
      Serial.println(error.c_str());
    }
  }

  delay(500); // Delay to prevent flooding
}
