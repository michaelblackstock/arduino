// Control ESP8266 anywhere

// Import required libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <aREST.h>
#include <stdio.h>

// Clients
WiFiClient espClient;
PubSubClient client(espClient);

// Create aREST instance
aREST rest = aREST(client);

// Unique ID to identify the device for cloud.arest.io
char* device_id = "c49n2di";

// WiFi parameters
const char* ssid = "mikenet";
const char* password = "thabo2elise";

// Functions
void callback(char* topic, byte* payload, unsigned int length);
int gpio0_pin = 0;
int gpio2_pin = 2;
int toggle = 0;
void setup(void)
{

  // preparing GPIOs
  pinMode(gpio0_pin, OUTPUT);
  digitalWrite(gpio0_pin, LOW);
  pinMode(gpio2_pin, OUTPUT);
  digitalWrite(gpio2_pin, LOW);

  
  // Start Serial
  Serial.begin(115200);

  // Set callback
  client.setCallback(callback);
  
  // Give name and ID to device
  rest.set_id(device_id);
  rest.set_name("esp8266");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Set output topic
  char* out_topic = rest.get_topic();

}

void loop() {

  // Connect to the cloud
 rest.loop(client);

}

// Handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

 //digitalWrite(gpio2_pin, HIGH);
 Serial.println(topic);
 Serial.println(topic);
 Serial.println(length);
 if (toggle)
 {
   // digitalWrite(gpio2_pin, HIGH);
   
 }
 if (!toggle)
 {
   // digitalWrite(gpio2_pin, LOW);
   
 }
 toggle = !toggle;
 //Serial.write(payload, DEC) ;
 rest.handle_callback(client, topic, payload, length);

}
