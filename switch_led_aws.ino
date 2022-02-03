#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"

unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 30000;

// Relay Pins
#define Switch1 LED_BUILTIN
#define Switch2 2 //
#define Switch3 4 //
#define Switch4 12 //


//Topic of MQTT 
#define AWS_IOT_PUBLISH_TOPIC   "wemos/temp_humid" // esp8266_1/sub
#define AWS_IOT_SUBSCRIBE_TOPIC1 "wemos/Switch1"
#define AWS_IOT_SUBSCRIBE_TOPIC2 "wemos/Switch2"
#define AWS_IOT_SUBSCRIBE_TOPIC3 "wemos/Switch3"
#define AWS_IOT_SUBSCRIBE_TOPIC4 "wemos/Switch4"

 
WiFiClientSecure net;
 
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);
 
PubSubClient client(net);
 
time_t now;
time_t nowish = 1510592825;
 
 
void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}
 
 

void connectAWS()
{
  delay(3000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));
 
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
 
  NTPConnect();
 
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
 
  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageHandler);
 
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(1000);
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC1);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC2);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC3);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC4);
 
  Serial.println("AWS IoT Connected!");
}
 
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["running text"] = "Hello world";
  
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
 
void setup()
{
  Serial.begin(115200);
  connectAWS();
  pinMode(Switch1, OUTPUT);
  pinMode(Switch2, OUTPUT);
  pinMode(Switch3, OUTPUT);
  pinMode(Switch4, OUTPUT);
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);

  if ( strstr(topic, "wemos/Switch1") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Switch_data = doc["status"];
    int r = Switch_data.toInt();
    digitalWrite(Switch1, !r);
    Serial.print("Switch1 - "); Serial.println(Switch_data);
  }

  if ( strstr(topic, "wemos/Switch2") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Switch_data = doc["status"];
    int r = Switch_data.toInt();
    digitalWrite(Switch2, !r);
    Serial.print("Switch2 - "); Serial.println(Switch_data);
  }

  if ( strstr(topic, "wemos/Switch1") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Switch_data = doc["status"];
    int r = Switch_data.toInt();
    digitalWrite(Switch3, !r);
    Serial.print("Switch3 - "); Serial.println(Switch_data);
  }

  if ( strstr(topic, "wemos/Switch1") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Switch_data = doc["status"];
    int r = Switch_data.toInt();
    digitalWrite(Switch4, !r);
    Serial.print("AC4 - "); Serial.println(Switch_data);
  }


}
 
void loop()
{
   
  now = time(nullptr);
 
  if (!client.connected())
  {
    connectAWS();
  }
  else
  {
    client.loop();
    if (millis() - lastMillis > 5000)
    {
      lastMillis = millis();
      publishMessage();
    }
  }
}
