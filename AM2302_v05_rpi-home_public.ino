/*
 * Author: Andreas Steen
 * 
 * ToDo:
 *  Add deep sleep
 *  Send status to MQTT
 */



// === Modify these values to suit your needs ===

// MQTT setup
char server[] = "rpi-home";                                             // This is the name of you RPi, can also be the IP-adress
char clientId[] = "mancave";                                            // Mandatory to send clientId each pub to mosquitto, not sure how it's actually used and if it need to be unique for each device
const char publishTopicTemp[] = "homebridge/mancave/temp";              // Use whatever topic structure you want. I went for gateway/room/sensor.
const char publishTopicHumidity[] = "homebridge/mancave/humidity";      // Use whatever topic structure you want. I went for gateway/room/sensor.

// WiFi setup
const char* ssid = "Your-WiFi-SSID";
const char* password = "Your-WiFi-Password";

// ==============================================


// WiFi & MQTT references
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7


// AM2302 references
#include "DHT.h"
#define DHTPIN 4            // what digital pin on the board the DHT22 is connected to
#define DHTTYPE AM2301      // read DHT.h for list of other types of boards
DHT dht(DHTPIN, DHTTYPE);


// Variables needed for float to string conversion
char bufTemp[10];
char bufHumidity[10];
String strTemp;
String strHumidity;


WiFiClient wifiClient;
PubSubClient client(server, 1883, wifiClient);  // No security or encryption used in this code


void setup() {

  // --> ToDo, send error messages to MQTT (for sensor errors, battery status etc)

  
  Serial.begin(9600);
  Serial.setTimeout(2000);
  while(!Serial) { }        // Wait for serial to initialize.


  // Start WiFi & MQTT
  wifiConnect();
  mqttConnect();


  // Give the NodeMcu 3 seconds to start and connect before doing anything
  delay(3000);

  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();               // Read humidity in %
  float t = dht.readTemperature();            // Read temperature as Celsius (the default)
  // float f = dht.readTemperature(true);     // Read temperature as Fahrenheit (isFahrenheit = true)


  // Check if any reads failed and exit early (to try again).
  // if (isnan(h) || isnan(t) || isnan(f)) {
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  // Compute heat index in Fahrenheit (the default)
  // float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);


  // Convert floating values to strings
  dtostrf(t,2,1,bufTemp);               //Current temperature must be in the range 0 to 100 degrees Celsius to a maximum of 1dp
  strTemp = String(bufTemp);
  dtostrf(h,2,0,bufHumidity);           //Current relative humidity must be in the range 0 to 100 percent with no decimal places
  strHumidity = String(bufHumidity);


  // Send temperature to MQTT server
  if (client.publish(publishTopicTemp, (char*) strTemp.c_str() )) {
  Serial.println(" ... Publish OK");
  } else {
  Serial.println(" ... Publish FAILED");
  }


  // Send humidity to MQTT server
  if (client.publish(publishTopicHumidity, (char*) strHumidity.c_str() )) {
  Serial.println(" ... Publish OK");
  } else {
  Serial.println(" ... Publish FAILED");
  }

  // Put the board to sleep
  ESP.deepSleep(60e6); // sleep for 60 seconds

}



void loop() {
  // Since we use deep sleep, all code is put in setup() as it's only executed once each awake cycle
  // Make sure to connect pin GPIO16 with RST on the board so it can wake up after sleep
} // End loop


// ======================================================
void wifiConnect() {
 WiFi.begin(ssid, password);
 Serial.print("Connecting to "); Serial.print(ssid);
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 } 
 Serial.print("WiFi connected, IP address: "); Serial.println(WiFi.localIP());
}
// ======================================================
void mqttConnect() {
 if (!!!client.connected()) {
 Serial.print("Reconnecting MQTT client to "); Serial.println(server);
 while (!!!client.connect(clientId)) {
 Serial.print(".");
 delay(500);
 }
 Serial.println();
 }
}
