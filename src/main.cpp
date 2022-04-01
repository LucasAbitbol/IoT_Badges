#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "PageIndex.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define SS_PIN D8  //--> SDA / SS is connected to pinout D
#define RST_PIN D3  //--> RST is connected to pinout D3
MFRC522 mfrc522(SS_PIN, RST_PIN);  //--> Create MFRC522 instance.

void MQTTcallback(char* topic, byte* payload, unsigned int length);

#define ON_Board_LED 2
 
const char* ssid = "Honor 10"; // Enter your WiFi name
const char* password =  "12345678"; // Enter WiFi password
const char* mqttServer = "maqiatto.com";
const int mqttPort = 1883;
const char* mqttUser = "sarah.gros@etud.univ-jfc.com";
const char* mqttUser2 = "beckerslucas1@gmail.com";
const char* mqttPassword = "iEnter";
const char* mqttPassword2 = "azerty";

LiquidCrystal_I2C lcd(0x27, 20, 4);

 
WiFiClient espClient;
WiFiClient espClient2;
PubSubClient client(espClient);
PubSubClient client2(espClient2);
ESP8266WebServer server(80);
int readsuccess;
byte readcard[4];
char str[32] = "";
String StrUID;


void array_to_string(byte array[], unsigned int len, char buffer[]) {
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

int getid() {  
  if(!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if(!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  
  for(int i=0;i<4;i++){
    readcard[i]=mfrc522.uid.uidByte[i]; //storing the UID of the tag in readcard
    array_to_string(readcard, 4, str);
    StrUID = str;
  }
  mfrc522.PICC_HaltA();
  Serial.println(str);
  return 1;
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
      payload[length] = '\0';
      String authorisation = String((char*)payload);
      if (authorisation == "authorized") {
        Serial.println("Accès autorisé");
        lcd.init();
        lcd.backlight();
        lcd.setCursor(3,1);
	      lcd.print("Acces autorise");
        delay(3000);
        lcd.clear();
      }
      if (authorisation == "denied") {
        Serial.println("Accès refusé");
        lcd.init();
        lcd.backlight();
        lcd.setCursor(4,1);
	      lcd.print("Acces refuse");
        delay(3000);
        lcd.clear();
      }
}
 
void setup() {
  
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  SPI.begin();
  mfrc522.PCD_Init();
  delay(500);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    Serial.println("");
  }
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client2.setServer(mqttServer, mqttPort);
 
  while (!client.connected()) {
    Serial.println("Connecting to Sarah Gros MQTT...");
 
    if (client.connect("marcelle1", mqttUser, mqttPassword )) {
 
      Serial.println("Connected to Sarah Gros");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }

    while (!client2.connected()) {
    Serial.println("Connecting to Lucas Beckers MQTT...");
 
    if (client2.connect("marcelle1", mqttUser2, mqttPassword2 )) {
 
      Serial.println("Connected to Lucas Beckers");  
      client2.subscribe("beckerslucas1@gmail.com/secureDoor");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client2.state());
      delay(2000);
    }
      }

  client2.setCallback(MQTTcallback);

  Serial.println("Please tag a card or keychain to see the UID !");
  Serial.println("");
}
 
void loop() {
  client.loop();
  client2.loop();
  if(getid()==1) {
    if (!client.connected()) {
      if (client.connect("marcelle1", mqttUser, mqttPassword )) {
        client.subscribe("sarah.gros@etud.univ-jfc.com/iEnter");
        client.publish("sarah.gros@etud.univ-jfc.com/iEnter", StrUID.c_str()); //Topic name
      } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
      }
    }
  }
  if (!client2.connected()) {
    if (client2.connect("marcelle1", mqttUser2, mqttPassword2 )) {
      client2.subscribe("beckerslucas1@gmail.com/secureDoor");
      client2.loop();
    }
    else {
      Serial.print("Echec");
    }
  }
}