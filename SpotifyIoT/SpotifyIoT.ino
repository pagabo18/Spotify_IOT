
#include <SPI.h>      // incluye libreria bus SPI
#include <MFRC522.h>      // incluye libreria especifica para MFRC522
#include <WiFi.h>
#include <PubSubClient.h> //Install library "PubSubClient" by Nick O'Leary
#include "DHT.h"

/****************************************
 * Define Constants
 ****************************************/
#define WIFISSID "FamGarciaBado2.4" // Put your WifiSSID here
#define PASSWORD "A2404P2410I1408" // Put your wifi password here
#define TOKEN "BBFF-Ifk9wAiEvuOm8pHsyfNYEaoiWXqjoQ" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "PracticaIot9" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                           //it should be a random and unique ascii string and different from all other devices

#define RST_PIN  9      // constante para referenciar pin de reset
#define SS_PIN  10      // constante para referenciar pin de slave select
char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[200];
char topic[150];


#define VARIABLE_LABEL_sens "sensor" // Assing the variable label


MFRC522 mfrc522(SS_PIN, RST_PIN); // crea objeto mfrc522 enviando pines de slave select y reset

byte LecturaUID[4];         // crea array para almacenar el UID leido
byte Usuario1[4]= {0x90, 0x0E, 0xE4, 0xA4} ;  
byte Usuario2[4]= {0x06, 0x76, 0x25, 0xD9} ;    




/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}







void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);     // inicializa comunicacion por monitor serie a 9600 bps
  SPI.begin();        // inicializa bus SPI
  mfrc522.PCD_Init();     // inicializa modulo lector
  Serial.println();
  Serial.print("Wait for WiFi..");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Not connected");
    delay(1000);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);  

  
}

void loop() {
    if (!client.connected()) {
    reconnect();
  }
  if ( ! mfrc522.PICC_IsNewCardPresent())   // si no hay una tarjeta presente
    return;           // retorna al loop esperando por una tarjeta
  
  if ( ! mfrc522.PICC_ReadCardSerial())     // si no puede obtener datos de la tarjeta
    return;           // retorna al loop esperando por otra tarjeta
    
    Serial.print("UID:");       // muestra texto UID:
    for (byte i = 0; i < mfrc522.uid.size; i++) { // bucle recorre de a un byte por vez el UID
      if (mfrc522.uid.uidByte[i] < 0x10){   // si el byte leido es menor a 0x10
        Serial.print(" 0");       // imprime espacio en blanco y numero cero
        }
        else{           // sino
          Serial.print(" ");        // imprime un espacio en blanco
          }
          Serial.print(mfrc522.uid.uidByte[i], HEX);    // imprime el byte del UID leido en hexadecimal
          LecturaUID[i]=mfrc522.uid.uidByte[i];     // almacena en array el byte del UID leido      
          }
          
          Serial.print("\t");         // imprime un espacio de tabulacion             
                    
          if(comparaUID(LecturaUID, Usuario1))    // llama a funcion comparaUID con Usuario1
            Serial.println("Bienvenido Usuario 1"); // si retorna verdadero muestra texto bienvenida
          else if(comparaUID(LecturaUID, Usuario2)) // llama a funcion comparaUID con Usuario2
            Serial.println("Bienvenido Usuario 2"); // si retorna verdadero muestra texto bienvenida
           else           // si retorna falso
            Serial.println("No te conozco");    // muestra texto equivalente a acceso denegado          
                  
                  mfrc522.PICC_HaltA();     // detiene comunicacion con tarjeta                
}

boolean comparaUID(byte lectura[],byte usuario[]) // funcion comparaUID
{
  for (byte i=0; i < mfrc522.uid.size; i++){    // bucle recorre de a un byte por vez el UID
  if(lectura[i] != usuario[i])        // si byte de UID leido es distinto a usuario
    return(false);          // retorna falso
  }
    sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_sens); // Adds the variable label
   boolean s1 = true;
  Serial.println(s1);

  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(h1, 4, 2, str_hum);
   
  sprintf(payload, "%s {\"value\": %s", payload, str_hum); // Adds the value
  sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  
  client.loop();
  delay(15000);
}
