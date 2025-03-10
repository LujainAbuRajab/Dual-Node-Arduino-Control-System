#include <DHT.h>
#include <ArduinoJson.h>
#include <Servo.h>

#define DHTPIN 21        // Pin where the DHT11 sensor is connected
#define DHTTYPE DHT11   // DHT 11 or DHT22
#define SWITCH_DIR 2     // Switch for direction control
#define SWITCH_START 3   // Switch for start/stop
#define POT_PIN A2       // Potentiometer for DC motor control
#define SERVO_PIN 9      // Servo motor pin      

DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor
Servo servoMotor;

void setup() {
  Serial1.begin(9600);     // Start Serial1 for communication
  Serial.begin(9600);       // Start Serial Monitor for debugging
  dht.begin(); 
  pinMode(SWITCH_DIR, INPUT_PULLUP);
  pinMode(SWITCH_START, INPUT_PULLUP);
  servo.attach(SERVO_PIN);
}

}

void loop() {
  // Read values from DHT sensor
  float temperature = dht.readTemperature(); // Read temperature as Celsius
  float humidity = dht.readHumidity();       // Read humidity
  int switchDir = digitalRead(SWITCH_DIR);
  int switchStart = digitalRead(SWITCH_START);
  int potValue = analogRead(POT_PIN);

  // Check if the readings are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Create a JSON object
  StaticJsonDocument<200> doc; // Adjust size based on your needs
  doc["node"] = 1; // Identifier for this node
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["switchDir"] = switchDir;
  doc["switchStart"] = switchStart;
  doc["potValue"] = potValue;


  // Serialize the JSON object to a string
  char jsonOutput[256];
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
      int ldrValue = docReceived["ldr"];
      int potValueReceived = docReceived["potValue"];

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
