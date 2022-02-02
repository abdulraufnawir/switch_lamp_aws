#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"
#include "DHT.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_LG.h>


#define DHTPIN 4        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 11
#define Desired_temp 24 //The desired temperature is 24*C at any time


const uint16_t kIrLed = 4;  // IR pin I/O on MCU
IRLgAc ac(kIrLed);  // Set the GPIO to be used for sending messages.
 
DHT dht(DHTPIN, DHTTYPE);
 
int h ;
int t;
unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 30000;

// Relay Pins
#define AC1 LED_BUILTIN
#define AC2 2 //test
#define AC3 4 //tested
#define AC4 12 // tested


//Topic of MQTT 
#define AWS_IOT_PUBLISH_TOPIC   "wemos/temp_humid" // esp8266_1/sub
#define AWS_IOT_SUBSCRIBE_TOPIC1 "wemos/AC1"
#define AWS_IOT_SUBSCRIBE_TOPIC2 "wemos/AC2"
#define AWS_IOT_SUBSCRIBE_TOPIC3 "wemos/AC3"
#define AWS_IOT_SUBSCRIBE_TOPIC4 "wemos/AC4"

 
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
 
 
//void messageReceived(char *topic, byte *payload, unsigned int length)
//{
//  Serial.print("Received [");
//  Serial.print(topic);
//  Serial.print("]: ");
//  for (int i = 0; i < length; i++)
//  {
//    Serial.print((char)payload[i]);
//  }
//  Serial.println();
//}
 
 
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
  doc["humidity"] = h;
  doc["temperature"] = t;
  //doc["AC_Status"] = ; //ON or OFF
  // doc["AC_setpoint"]=;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
 
void setup()
{
  Serial.begin(115200);
  connectAWS();
  dht.begin();
//  ac.begin();
  pinMode(AC1, OUTPUT);
  pinMode(AC2, OUTPUT);
  pinMode(AC3, OUTPUT);
  pinMode(AC4, OUTPUT);
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);

  if ( strstr(topic, "wemos/AC1") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay_data = doc["status"];
    int r = Relay_data.toInt();
    digitalWrite(AC1, !r);
    Serial.print("AC1 - "); Serial.println(Relay_data);
  }

  if ( strstr(topic, "wemos/AC2") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay_data = doc["status"];
    int r = Relay_data.toInt();
    digitalWrite(AC2, !r);
    Serial.print("AC2 - "); Serial.println(Relay_data);
  }

  if ( strstr(topic, "wemos/AC1") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay_data = doc["status"];
    int r = Relay_data.toInt();
    digitalWrite(AC3, !r);
    Serial.print("AC3 - "); Serial.println(Relay_data);
  }

  if ( strstr(topic, "wemos/AC1") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay_data = doc["status"];
    int r = Relay_data.toInt();
    digitalWrite(AC4, !r);
    Serial.print("AC4 - "); Serial.println(Relay_data);
  }


}
 
void loop()
{
  h = dht.readHumidity();
  t = dht.readTemperature();
 
  if (isnan(h) || isnan(t) )  // Check if any reads failed and exit early (to try again).
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
 
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  delay(2000);
 
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
