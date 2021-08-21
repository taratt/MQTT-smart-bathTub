#include <SPI.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MFRC522.h>

#define WATERLEVEL A0 
#define TLED D1
#define DLED D8
#define BLED D0

MFRC522 mfrc522(D4, D3);
const char* ssid = "YOUR SSID";
const char* password =  "YOUR PASSWORD";
const char* mqtt_server = "192.168.1.106";
const int mqtt_port = 1883;
int temperature=0;
int depth =0;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode( TLED, OUTPUT);
  pinMode( DLED, OUTPUT);
  pinMode( BLED, OUTPUT);
  digitalWrite(BLED, HIGH);
  digitalWrite(DLED, LOW);
  pinMode( WATERLEVEL, INPUT);
  Serial.begin(115200);
  while(!Serial);
  Serial.setDebugOutput(false);
  Serial.println();
  Serial.println();
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
    Serial.print("Connected to WiFi :");
  Serial.println(WiFi.SSID());

    client.setServer(mqtt_server, mqtt_port);
  
  client.setCallback(MQTTcallback);
  while (!client.connected()) 
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266","node","1"))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  client.subscribe("tempDepth");


  analogWriteRange(100);

  SPI.begin();           
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();

}


void MQTTcallback(char* topic, byte* payload, unsigned int leng) 
{
  Serial.print("Message:");
  bool temp = false;
  String tempera = "";
  String depptt ="";
  char sep = 'm';
  for (int i = 0; i < leng; i++) 
  {
    if ((char)payload[i] != sep && temp == false){
      tempera = tempera + (char)payload[i];
    }
      
    else if ((char)payload[i] == sep && temp == false){
      temp = true;
    }
    else if ((char)payload[i] != sep && temp == true){
  
      depptt = depptt + (char)payload[i];
      
    }
     
  }
  temperature =tempera.toInt();
  depth = depptt.toInt();
  Serial.print(tempera);
  Serial.print(" ");
  Serial.print(depptt);
  analogWrite(TLED, temperature);
  Serial.println();
  Serial.println("-----------------------");
}

void array_to_string(byte array[], unsigned int len, char buffer[])
{
   for (unsigned int i = 0; i < len; i++)
   {
      byte nib1 = (array[i] >> 4) & 0x0F;
      byte nib2 = (array[i] >> 0) & 0x0F;
      buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
      buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
   }
   buffer[len*2] = '\0';
}

//byte readCard[5];
int waterValue = 0;
unsigned long previousMillis = 0;       
const long interval = 1000; 
int ledState = LOW;  
void loop() {
   unsigned long currentMillis = millis();
  client.loop();
  if (mfrc522.PICC_IsNewCardPresent()){
    if(mfrc522.PICC_ReadCardSerial()){
      Serial.println(">>RFID TAG UID:");
      for (byte i =0; i < mfrc522.uid.size; ++i){
//        readCard[i] = mfrc522.uid.uidByte[i];
        Serial.print(mfrc522.uid.uidByte[i] , HEX);
        
        Serial.print(" ");
      }
  
      char str[32] = "";
       array_to_string(mfrc522.uid.uidByte, 4, str);
       mfrc522.PICC_HaltA();
      client.publish("rfid",str);
      Serial.println("published");
      delay(1000);
  }
  else;
  client.loop();
   
  }
  else{
  waterValue = analogRead(WATERLEVEL);
  waterValue = map(waterValue, 0, 1023, 0, 100);
//  Serial.println(waterValue);
//  int depthValue= map(waterValue, 0, 100, 0, 50);
  analogWrite(DLED, waterValue);
  
  delay(100);
//  Serial.println(depth);
//  Serial.println(temperature);
//  Serial.println(waterValue);
  int maxi;
  int mini;
  if (waterValue < depth){
    maxi = depth;
    mini = waterValue;
  }
   else{
    maxi = waterValue;
    mini = depth;
   }
  if((maxi-mini) > 10){
        if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
          if (ledState == LOW) {
            ledState = HIGH;
          } else {
            ledState = LOW;
          }
          digitalWrite(BLED, ledState);
//          Serial.println(ledState);
        }

  }
  }
}
