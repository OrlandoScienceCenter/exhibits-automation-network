#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "Secrets.h"

#define ONPIN  14
#define OFFPIN 12
#define CONPIN 13

WiFiClient espClient;
PubSubClient client(espClient);

bool onLastState = 0;
bool onCurState = 0;
bool offLastState = 0;
bool offCurState = 0;

char msg[50];

void setup() {
  pinMode(ONPIN,INPUT_PULLUP);
  pinMode(OFFPIN,INPUT_PULLUP);
  pinMode(CONPIN,OUTPUT);
  digitalWrite(CONPIN,HIGH);
  Serial.begin(115200);
  wifiSetup();
  client.setServer(mqtt_server, 1883);

  pinMode(0,OUTPUT);
  pinMode(15,OUTPUT);
  digitalWrite(0,LOW);
  digitalWrite(15,LOW);
}

void loop() {
  if(!client.connected()) {
    digitalWrite(CONPIN,HIGH);
    reconnect();
    digitalWrite(CONPIN,LOW);
  }
  client.loop();
  onCurState = digitalRead(ONPIN);
  offCurState = digitalRead(OFFPIN);
  if(!onCurState && onLastState){
    Serial.println("Sending on commands");
    snprintf (msg, 49, "powerOn");
    client.publish(topic1, msg);
    client.publish(topic2, msg);
  }
  else if(!offCurState && offLastState){
    Serial.println("Sending off commands");
    snprintf (msg, 49, "powerOff");
    client.publish(topic1, msg);
    client.publish(topic2, msg);
  }
  onLastState = onCurState;
  offLastState = offCurState;
  delay(10);
}

void wifiSetup(){
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    String clientId = F("ESPCli-");
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println(F("connected"));
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));
      delay(5000);
    }
  }
}
