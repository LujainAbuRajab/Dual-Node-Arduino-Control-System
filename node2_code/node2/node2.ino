#include <DHT.h>
#include <ArduinoJson.h>
#include <Servo.h>

#define DHTPIN 8        // Pin where the DHT11 sensor is connected
#define DHTTYPE DHT11   // DHT 11 or DHT22
#define POT_PIN A1
#define LDR_PIN A10 
#define SERVO_PIN 9
#define MOTOR_ENA 4       
#define MOTOR_IN1 27       
#define MOTOR_IN2 26  
#define LED_PIN 13

DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor
Servo servo;

void setup() {
  Serial1.begin(9600);     // Start Serial1 for communication
  Serial.begin(9600);       // Start Serial Monitor for debugging
  dht.begin();              // Initialize the DHT sensor
  servo.attach(SERVO_PIN);
  pinMode(MOTOR_ENA, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Read values from DHT sensor
  float temperature = dht.readTemperature(); // Read temperature as Celsius
  float humidity = dht.readHumidity();       // Read humidity
  int potValue = analogRead(POT_PIN);
  int ldrValue = analogRead(LDR_PIN);


 

  // Check if the readings are valid
  if (isnan(temperature) || isnan(humidity) || isnan(potValue) ) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  // Create a JSON object
  StaticJsonDocument<200> doc; // Adjust size based on your needs
  doc["node"] = 2; // Identifier for this node
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["pot"] = potValue;
  doc["ldr"] = ldrValue;

  // Serialize the JSON object to a string
  char jsonOutput[2048];
  serializeJson(doc, jsonOutput);

  // Send the JSON string
  Serial1.println(jsonOutput);

  // Debugging Output
  Serial.print("Node 2 Sent JSON: ");
  Serial.println(jsonOutput);

  // Check for incoming data from Node 1
  if (Serial1.available()) {
    String receivedData = Serial1.readStringUntil('\n');

    // Create a JSON document to hold the parsed data
    StaticJsonDocument<200> docReceived;
    DeserializationError error = deserializeJson(docReceived, receivedData);

    // Check for errors in parsing
    if (!error) {
      int nodeID = docReceived["node"];
      float temperatureReceived = docReceived["temp"];
      float humidityReceived = docReceived["hum"];
      int potValueReceived = docReceived["pot"];
      int switchStart = docReceived["Start"];
      int switchDir = docReceived["Dir"];
    

      // Print the received values
      Serial.print("Node 1 JSON Received: ");
      Serial.print(receivedData);
      Serial.print(" | Node ID: ");
      Serial.print(nodeID);
      Serial.print(" | Temperature: ");
      Serial.print(temperatureReceived);
      Serial.print(" | Humidity: ");
      Serial.println(humidityReceived);

      servo.write(map(potValue, 0, 1023, 0, 180));


      if (switchStart == 1) {  // If Start Switch is pressed
        int motorSpeed = map(potValueReceived, 0, 1023, 0, 255);
        analogWrite(MOTOR_ENA, motorSpeed);
        analogWrite(LED_PIN, motorSpeed);

        if (switchDir == 1) { // Forward
          digitalWrite(MOTOR_IN1, HIGH);
          digitalWrite(MOTOR_IN2, LOW);
        } else { // Reverse
          digitalWrite(MOTOR_IN1, LOW);
          digitalWrite(MOTOR_IN2, HIGH);
        }
      } else { // Stop motor
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        analogWrite(MOTOR_ENA, 0);
        analogWrite(LED_PIN, 0);
      }



      
    } else {
      Serial.print("Failed to parse JSON from Node 1: ");
      Serial.println(error.c_str());
    }
  }

  delay(500); // Delay to prevent flooding
}
