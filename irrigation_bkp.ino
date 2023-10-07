#include <ArduinoJson.h>
#include "EspMQTTClient.h"

//variáveis para Json
char msg1[100];
char msg2[100];
int temp;
float graus_c;
float umid;

//configurações da conexão MQTT
EspMQTTClient client
(
  "Poliana2.4", //nome da sua rede Wi-Fi
  "31@BF482a", //senha da sua rede Wi-Fi
  "mqtt.tago.io",  // MQTT Broker server ip padrão da tago
  "Default",   // username
  "2d493784-befd-4bbc-",   // Código do Token 2d493784-befd-4bbc-9ae0-c93afb5e1fed 
  "TestClient",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void setup (){
  Serial.begin(115200);
}

void onConnectionEstablished()
{}

void loop()
{

//atribui valores aleatórios para variável 
  graus_c = random(15,35);
  Serial.println(graus_c);
  temp = graus_c * 1.8 + 32; // Convert Celsius to Fahreinheit

  umid = random(100,200);
  Serial.println(umid);

//arquivo Json

  StaticJsonDocument<300> dados_temp, dados_umid;

  dados_temp["variable"] = "temperature"; 
  dados_temp["value"] = temp;
  dados_umid["variable"] = "umidade";
  dados_umid["value"] = umid;

  serializeJson(dados_temp, msg1);
  Serial.println(msg1);

  serializeJson(dados_umid, msg2);
  Serial.println(msg2);

//envio de dados

  client.publish("info/temperatura", msg1);
  client.publish("info/umidade", msg2);
  
  delay(10000);

  client.loop();
}
