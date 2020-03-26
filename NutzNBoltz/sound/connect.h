// NutzNBoltz GHOSTS IOT Sound Sensor
// Rovisys through Chris Conry 
// Rabah Gattfi, John Huff, Orin Higgins, Radu Almassan, Benjamin Weaver

#ifndef CONNECT_H
#define CONNECT_H

#define Serial SerialUSB

#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

#include <ArduinoHttpClient.h>

#include "secrets.h"

char* ssid = SECRET_SSID;
char* pass = SECRET_PASS;
const unsigned int CHECK_INTERVAL = 5000;

WiFiClient transport;
PubSubClient client(transport);

static int status = WL_IDLE_STATUS;
const int time = 250; //Sample in ms
unsigned int amp = 0;

void setup_wifi() 
{
    if (WiFi.status() == WL_NO_MODULE) 
    {
        Serial.println("Communication with WiFi module failed!");
        while (true);
    }

    while (status != WL_CONNECTED) 
    {
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
    Serial.print("MQTT connect attempt...");
    if (client.connect("ESP32")) 
    {
      Serial.println("connected");
      client.subscribe("esp32/sound");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" wait 5 sec and try again");
      delay(5000);
    }
  }
}

const short VERSION = 3;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

void sketchDownload(const unsigned long start) 
{
  unsigned long begin_time = start;
  unsigned long current_time = 0;
  if (begin_time - current_time < CHECK_INTERVAL) return;
  current_time = begin_time;
  HttpClient client(transport, SERVER, SERVER_PORT);
  char buff[32];
  snprintf(buff, sizeof(buff), PATH, VERSION + 1);
  Serial.print("Checking for new file ");
  Serial.println(buff);
  client.get(buff);
  int statusCode = client.responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  if (statusCode != 200) 
  {
    client.stop();
    return;
  }

    int length = client.contentLength();

    if (length == HttpClient::kNoContentLengthHeader) 
    {
        client.stop();
        Serial.println("Server didn't provide Content-length header. Can't continue with update.");
        return;
    }
    Serial.print("Server returned update file of size ");
    Serial.print(length);
    Serial.println(" bytes");

    if (!InternalStorage.open(length)) 
    {
        client.stop();
        Serial.println("There is not enough space to store the update. Can't continue with update.");
    return;
    }
    byte b;

    client.setTimeout(30000); 
  
    while (length > 0) 
    {
        if (!client.readBytes(&b, 1)) 
        break;
        InternalStorage.write(b);
        length--;
    }
    InternalStorage.close();
    client.stop();

    if (length > 0) 
    {
        Serial.print("Timeout downloading update file at ");
        Serial.print(length);
        Serial.println(" bytes. Can't continue with update.");
        return;
    }

    Serial.println("Sketch update apply and reset.");
    Serial.flush();
    InternalStorage.apply();
}

#endif // CONNECT_H