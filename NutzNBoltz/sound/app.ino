// NutzNBoltz GHOSTS IOT Sound Sensor
// Rovisys through Chris Conry 
// Rabah Gattfi, John Huff, Orin Higgins, Radu Almassan, Benjamin Weaver

#include <avr/dtostrf.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include "secrets.h"

#define PIN_ANALOG_IN A1

char* ssid = SECRET_SSID;
char* pass = SECRET_PASS;
char* mqtt_server = MQTT_SERVER;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];

int status = WL_IDLE_STATUS;
const int time = 250; //Sample in mS
unsigned int amp = 0;
 
void setup() 
{
   Serial.begin(115200);
   while(!Serial){;}
   setup_wifi();
   client.setServer(mqtt_server, 1883);
   client.setCallback(callback);
   pinMode(A1, INPUT);
}

void setup_wifi() 
{
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

   while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
   }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void callback(char* topic, byte* message, unsigned int length) 
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String soundMessage;
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)message[i]);
    soundMessage += (char)message[i];
  }
  Serial.println();
}

void reconnect() 
{
  while(!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) 
    {
      Serial.println("connected");
      client.subscribe("esp32/output");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() 
{
   if(!client.connected()) 
   {
      reconnect();
   }
   client.loop();
   unsigned long start= millis(); 
   unsigned int peak = 0;   
 
   unsigned int max = 0;
   unsigned int min = 1024;
 
   while (millis() - start < amp)
   {
      amp = analogRead(0);
      if (amp < 1024)  
      {
         if (amp > max)
         {
            max = amp; 
         }
         else if (amp < min)
         {
            min = amp;  
         }
      }
   }
   amp = max - min; 
   double decibels = 100 * ((amp * 5.0) / 1024);  
   char tempString[8];
   dtostrf(decibels, 1, 2, tempString);
   Serial.print("Sound: ");
   Serial.println(tempString);
   client.publish("esp32/sound",tempString);   
}