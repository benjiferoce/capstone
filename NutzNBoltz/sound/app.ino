// NutzNBoltz GHOSTS IOT Sound Sensor
// Rovisys through Chris Conry 
// Rabah Gattfi, John Huff, Orin Higgins, Radu Almassan, Benjamin Weaver

#include <avr/dtostrf.h>
#include <Adafruit_Sensor.h>
#include "connect.h"

#define PIN_ANALOG_IN A1
char* mqtt_server = SERVER;
auto lastMsg = 0;
char msg[50];
 
void setup() 
{
  Serial.begin(115200);
  while(!Serial)
  {
    ;
  }
  Serial.print("Sketch version ");
  Serial.println(VERSION);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(A1, INPUT);
}


void loop() 
{
   if(!client.connected()) 
   {
      reconnect();
   }
   client.loop();

   unsigned int start = millis(); 
   unsigned int peak  = 0;   
   unsigned int max  = 0;
   unsigned int min  = 1024;
 
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
   auto decibels = 100 * ((amp * 5.0) / 1024);  
   char tempString[2];
   dtostrf(decibels, 1, 2, tempString);
   Serial.print("Sound: ");
   Serial.println(tempString);
   client.publish("esp32/sound",tempString);
    handleSketchDownload();   
}