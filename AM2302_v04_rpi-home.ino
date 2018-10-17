/*
 * Author:/ Roland Revsäter & Andreas Steen
 * License: Apache License v2
 */


// WiFi & MQTT references
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7


// AM2302 references
#include "DHT.h"
#define DHTPIN 4     // what digital pin the DHT22 is connected to
#define DHTTYPE AM2301
DHT dht(DHTPIN, DHTTYPE);


// MQTT setup
char server[] = "rpi-home";
char clientId[] = "AM2302_ManCave";
const char publishTopicTemp[] = "homebridge/mancave/temp";
const char publishTopicHumidity[] = "homebridge/mancave/humidity";
const char PayloadTemp[] = "25";
const char PayloadHumidity[] = "75";


// WiFi setup
const char* ssid = "Granitapan";
const char* password = "740812680729";


// Create some stuff needed for float to string conversion
char bufTemp[10];
char bufHumidity[10];
String strTemp;
String strHumidity;


WiFiClient wifiClient;
PubSubClient client(server, 1883, wifiClient);


void setup() {
  
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }

  // Start WiFi & MQTT
  wifiConnect();
  mqttConnect();



}


int timeSinceLastRead = 0;

void loop() {

  // Report every 30 seconds.
  if(timeSinceLastRead > 30000) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    // --> Potentially remove reading in f to speed up the process
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    // --> Potentially send error messages to status topic in MQTT
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

/*
 * Current temperature must be in the range 0 to 100 degrees Celsius to a maximum of 1dp.
 * Current relative humidity must be in the range 0 to 100 percent with no decimal places.
 */

    // Convert floating values to strings
    dtostrf(t,2,1,bufTemp);
    strTemp = String(bufTemp);
    dtostrf(h,2,0,bufHumidity);
    strHumidity = String(bufHumidity);

    // Send temperature
    Serial.print("Sending temp payload: ");
 
    if (client.publish(publishTopicTemp, (char*) strTemp.c_str() )) {
    Serial.println(" ... Publish OK");
 
    } else {
    Serial.println(" ... Publish FAILED");
    }
  
    // Send humidity
    Serial.print("Sending humidity payload: ");
 
    if (client.publish(publishTopicHumidity, (char*) strHumidity.c_str() )) {
    Serial.println(" ... Publish OK");
 
    } else {
    Serial.println(" ... Publish FAILED");
    }

    timeSinceLastRead = 0;

  }


  // Strange delay structure but used to delay getting values 2 seconds but do a client.loop every 100ms
  delay(100);
  timeSinceLastRead += 100;


 if (!client.loop()) {
 mqttConnect();
 }

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
