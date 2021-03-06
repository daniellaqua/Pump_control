#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* SSID = "pulsoxi_phantom";
const char* PSK = "NIFPO2018";
const char* MQTT_BROKER = "192.168.1.2";

WiFiClient espClient;
PubSubClient client(espClient);

int sensorPin = A0;    // select the input pin for the potentiometer
const int pumpMaternal = D0;      // select the pin for the LED
const int ledOrange = D1;
const int ledGreen = D2;      // select the pin for the LED
const int buttonMaternal = D6;     // the number of the pushbutton pin
const int pressure_lvl_ready = 610;
const int pressure_lvl_still_ready = 540;
const float LSB = 0.003223; 

// variables will change:
int sensorValue = 0;  // variable to store the value coming from the sensor
float sensorVoltage = 0;
int sensorPressure = 0;
int buttonState = 1;         // variable for reading the pushbutton status
int pumpState = 0;

// Timers auxiliar variables
long mqtt_delay = 0;


void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="Pumpensteuerung/Remote_Pumpe_Maternal"){
      Serial.print("switching pump: ");
      if(messageTemp == "on"){
        if (sensorValue < pressure_lvl_still_ready) {
          digitalWrite(pumpMaternal, HIGH);
          pumpState = 1;
          Serial.print("On");
          client.publish("Pumpensteuerung/Pumpe_Maternal", "on");
          client.publish("Pumpensteuerung/Hinweis_Maternal", "Remote start!");
        } else {
          // pressure is high --> Don't switch pump ON
          client.publish("Pumpensteuerung/Hinweis_Maternal", "Pressure already high!");
        }
      } else if(messageTemp == "off"){
        digitalWrite(pumpMaternal, LOW);
        pumpState = 0;
        Serial.print("Off");
        client.publish("Pumpensteuerung/Pumpe_Maternal", "off");
        client.publish("Pumpensteuerung/Hinweis_Maternal", "Remote stop!");
      }
  }
  Serial.println();
}




void setup() {
  // declare the pumpMaternal as an OUTPUT:
  pinMode(pumpMaternal, OUTPUT);
  pinMode(ledOrange, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonMaternal, INPUT);
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_BROKER, 1883);
  client.setCallback(callback);
  
  Serial.println();
  Serial.println("Pressure Control");
}

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
 
    WiFi.begin(SSID, PSK);
    IPAddress ip(192,168,1,200);   
    IPAddress gateway(192,168,1,1);   
    IPAddress subnet(255,255,255,0);   
    WiFi.config(ip, gateway, subnet);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
 
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}


void reconnect() {
    while (!client.connected()) {
        Serial.print("Reconnecting...");
        if (!client.connect("ESP8266pump")) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
        client.subscribe("Pumpensteuerung/Remote_Pumpe_Maternal");
    }
}

void loop() {
  // read new values from sensors:
  sensorValue = analogRead(sensorPin);
  sensorVoltage = sensorValue * LSB;
  sensorPressure = sensorValue * 250;
  
  if (sensorValue > pressure_lvl_still_ready){
    digitalWrite(ledOrange, HIGH);
  }
  else {
    digitalWrite(ledOrange, LOW);
  }

  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  mqtt_delay = mqtt_delay +1;
  
  if (mqtt_delay == 200) {
    digitalWrite(ledGreen, HIGH);
    mqtt_delay = 0;
    static char Vordruck[7];
    dtostrf(sensorValue, 6, 0, Vordruck);
    client.publish("Pumpensteuerung/Vordruck_Maternal", Vordruck);
    digitalWrite(ledGreen, LOW);
  }



  // Automatic pump Switch-OFF
  if (sensorValue >= pressure_lvl_ready && pumpState == 1) {
    // turn Pump off:
    digitalWrite(pumpMaternal, LOW);
    // change pump state to OFF
    pumpState = 0;
    client.publish("Pumpensteuerung/Pumpe_Maternal", "off");
    client.publish("Pumpensteuerung/Hinweis_Maternal", "Pump stopped!");

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
  buttonState = digitalRead(buttonMaternal);

  // check if the pushbutton is pressed. If it is, the buttonState is LOW:
  if (buttonState == LOW) { // wenn Taste gedrückt (also LOW)
    if (pumpState == 1) { //wenn pumpe an -- also rote LED an) --> Pumpe ausmachen
      // switch pump OFF:
      digitalWrite(pumpMaternal, LOW); //Pin auf Low -- rote LED aus
      // change pump state to OFF
      pumpState = 0;
      client.publish("Pumpensteuerung/Pumpe_Maternal", "off");
      client.publish("Pumpensteuerung/Hinweis_Maternal", "Manueller Stopp");
      
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

      // pressure low --> switch pump on
      if (sensorValue < pressure_lvl_still_ready) {
        // switch pump ON:
        digitalWrite(pumpMaternal, HIGH);
        // change pump state to ON
        pumpState = 1;
        client.publish("Pumpensteuerung/Pumpe_Maternal", "on");
        client.publish("Pumpensteuerung/Hinweis_Maternal", " ");
        
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
        // pressure is high --> Don't switch pump ON
        
        client.publish("Pumpensteuerung/Hinweis_Maternal", "Pressure already high!");

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
  delay(10); 
}
