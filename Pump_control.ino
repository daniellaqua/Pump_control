

int sensorPin = A0;    // select the input pin for the potentiometer
const int ledPin = 13;      // select the pin for the LED
const int buttonPin = 12;     // the number of the pushbutton pin
const int pressure_lvl_ready = 600;
const int pressure_lvl_still_ready = 540;
const float LSB = 0.003223; 

// variables will change:
int sensorValue = 0;  // variable to store the value coming from the sensor
float sensorVoltage = 0;
int sensorPressure = 0;
int buttonState = 0;         // variable for reading the pushbutton status
int pumpState = 0;

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Pressure Control");
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  sensorVoltage = sensorValue * LSB;
  sensorPressure = sensorValue * 250;
  //Serial.println();
  //Serial.print("Pressure: ");
  //Serial.print(sensorValue);

  if (sensorValue >= pressure_lvl_ready && pumpState == 1) {
    // turn LED off:
    digitalWrite(ledPin, LOW);
    // change pump state to OFF
    pumpState = 0;
    // print "Pump OFF"

    Serial.println();
    Serial.println("pressure level reached");
    Serial.println("Pump: OFF");
    Serial.print("Sensor value: ");
    Serial.println(sensorValue);
    Serial.print("Sensor voltage: ");
    Serial.println(sensorVoltage);
    Serial.print("Pressure: ");
    Serial.println(sensorPressure);
  }
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    if (pumpState == 1) {
      // turn LED OFF:
      digitalWrite(ledPin, LOW);
      // change pump state to OFF
      pumpState = 0;
      // print "Pump OFF"
      Serial.println();
      Serial.println("Pump: OFF");
      Serial.print("Sensor value: ");
      Serial.println(sensorValue);
      Serial.print("Sensor voltage: ");
      Serial.println(sensorVoltage);
      Serial.print("Pressure: ");
      Serial.println(sensorPressure);
    } else {
      if (sensorValue < pressure_lvl_still_ready) {
        // turn LED on:
        digitalWrite(ledPin, HIGH);
        // change pump state to ON
        pumpState = 1;
        // print "Pump ON"
        Serial.println();
        Serial.println("Pump: ON");
        Serial.print("Sensor value: ");
        Serial.println(sensorValue);
        Serial.print("Sensor voltage: ");
        Serial.println(sensorVoltage);
        Serial.print("Pressure: ");
        Serial.println(sensorPressure);
      } else {
        // print "pressure already high"
        Serial.println();
        Serial.println("Pressure already high!");
        Serial.print("Sensor value: ");
        Serial.println(sensorValue);
        Serial.print("Sensor voltage: ");
        Serial.println(sensorVoltage);
        Serial.print("Pressure: ");
        Serial.println(sensorPressure);
      }
    }
  delay(200);  
  }

}
