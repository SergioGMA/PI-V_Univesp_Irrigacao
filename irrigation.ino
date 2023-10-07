
//Carrega as bibliotecas
#include <ArduinoJson.h>
#include "EspMQTTClient.h"
#include "max6675.h"
#include <HCSR04.h>

//define os pinos
#define pinSensorA 13
#define pinSensorD 26
#define pinReleSolenoide 14

//variáveis para Json
char msg1[100];
char msg2[100];
char msg3[100];
char msg4[100];
char msg5[100];

//variáveis temperatura
int SO = 23;
int CS = 5;
int sck = 18;
float temperatura;
int temp;
//Inicializa o sensor nos pinos definidos acima
MAX6675 thermocouple(sck, CS, SO);

//variáveis umidade
float umid;

//Define os pinos para o trigger e echo
#define p_trigger 4
#define p_echo 15
//Inicializa o sensor nos pinos definidos acima
UltraSonicDistanceSensor distanceSensor(p_trigger, p_echo);

//variáveis ultrassom
int dist;
float volume;
float litro;
float conversao;

//configurações da conexão MQTT
EspMQTTClient client
(
  "Poliana2.4", //nome da sua rede Wi-Fi
  "31@BF482a", //senha da sua rede Wi-Fi
  "mqtt.tago.io",  // MQTT Broker server ip padrão da tago
  "Default",   // username
  "2d493784-befd-4bbc-9ae0-",   // Código do Token 2d493784-befd-4bbc-9ae0-c93afb5e1fed 
  "TestClient",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void setup (){
  pinMode(pinSensorD, INPUT);
  pinMode(pinReleSolenoide, OUTPUT);
  Serial.begin(115200);
}

void onConnectionEstablished()
{}

void loop()
{

//leitura de temperatura
  temperatura = thermocouple.readCelsius();
  Serial.println(temperatura);
  temp = temperatura * 1.8 + 32; // Convert Celsius to Fahreinheit

//leitura umidade
  
  if (digitalRead(pinSensorD)) {
     umid = 1500;
     Serial.println("SEM UMIDADE ");
     digitalWrite(pinReleSolenoide, HIGH);
     
  } else {
     umid = 500;
     Serial.println("COM UMIDADE ");
     digitalWrite(pinReleSolenoide, LOW);
  }

//leitura ultassom
  //Le as informacoes do sensor, em cm
  dist = distanceSensor.measureDistanceCm();
  
  //Exibe informacoes no serial monitor
  Serial.print("Distancia em cm: ");
  Serial.println(dist);
  volume = dist * 300;
  litro = volume / 1000;
  conversao = (map(litro, 0, 5.7, 5.7, 0));
  Serial.print(conversao);
  Serial.println(" litros");
  delay(5000);
  
//arquivo Json

  StaticJsonDocument<300> dados_temp, dados_umid, dados_ultra;

  dados_temp["variable"] = "temperatura"; 
  dados_temp["value"] = temp;
  dados_umid["variable"] = "umidade";
  dados_umid["value"] = umid;
  dados_ultra["variable"] = "litros";
  dados_ultra["value"] = conversao;

  serializeJson(dados_temp, msg1);
  Serial.println(msg1);

  serializeJson(dados_umid, msg2);
  Serial.println(msg2);

  serializeJson(dados_ultra, msg3);
  Serial.println(msg3);

//envio de dados

  client.publish("info/temperatura", msg1);
  client.publish("info/umidade", msg2);
  client.publish("info/ultrassom", msg3);
  //client.publish("info/vazao", msg4);
  //client.publish("info/bomba", msg5);
  
  delay(5000);

  client.loop();
}
