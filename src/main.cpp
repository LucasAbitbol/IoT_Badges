#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "PageIndex.h"

#define SS_PIN D2  //--> SDA / SS is connected to pinout D2
#define RST_PIN D1  //--> RST is connected to pinout D1
MFRC522 mfrc522(SS_PIN, RST_PIN);  //--> Create MFRC522 instance.

#define ON_Board_LED 2
 
const char* ssid = "Pioneer"; // Enter your WiFi name
const char* password =  "eb96bbb9"; // Enter WiFi password
const char* mqttServer = "maqiatto.com";
const int mqttPort = 1883;
const char* mqttUser = "adrien.peyrouty@e-rekcah.com";
const char* mqttPassword = "ISIS_IoT_2020";
 
WiFiClient espClient;
PubSubClient client(espClient);
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
  return 1;
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



  Serial.println("Please tag a card or keychain to see the UID !");
  Serial.println("");
 
  client.setServer(mqttServer, mqttPort);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
}
 
void loop() {
  client.loop();

  if(getid()==1) {
      client.publish("adrien.peyrouty@e-rekcah.com/test1", StrUID.c_str()); //Topic name
  }
}