
//Carrega as bibliotecas
#include <ArduinoJson.h>
#include "EspMQTTClient.h"
#include "max6675.h"
#include <HCSR04.h>

//define os pinos
#define pinSensorA 13
#define pinSensorD 26
#define pinReleSolenoide 14
#define pinReleBomba 12

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
String umid;

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

//definicao do pino do sensor e de interrupcao
const int INTERRUPCAO_SENSOR = 36; //interrupt = 0 equivale ao pino digital 2
const int PINO_SENSOR = 36;

//definicao da variavel de contagem de voltas
unsigned long contador = 0;

//definicao do fator de calibracao para conversao do valor lido
const float FATOR_CALIBRACAO = 4.5;

//definicao das variaveis de fluxo e volume
float fluxo = 0;
float volume_vaz = 0;
float volume_total = 0;

//definicao da variavel de intervalo de tempo
unsigned long tempo_antes = 0;

//variável relé
String(bomba_onoff);

//configurações da conexão MQTT
EspMQTTClient client
(
  "Poliana2.4", //nome da sua rede Wi-Fi
  "31@BF482a", //senha da sua rede Wi-Fi
  "mqtt.tago.io",  // MQTT Broker server ip padrão da tago
  "Default",   // username
  "4dde55ba-3901-487b-b560-918987e4f8",   // (Código do Token 2d493784-befd-4bbc-9ae0-c93afb5e1fed Sérgio)(4dde55ba-3901-487b-b560-918987e4f83e Thiago)
  "TestClient",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void setup (){
  pinMode(pinSensorD, INPUT);
  pinMode(pinReleSolenoide, OUTPUT);
  pinMode(pinReleBomba, OUTPUT);
  Serial.begin(115200);
  //configuracao do pino do sensor como entrada em nivel logico alto
  pinMode(PINO_SENSOR, INPUT_PULLUP);

  //rele liga desliga bomba
  digitalWrite(pinReleBomba, LOW);
  bomba_onoff = "desligada";

  //rele liga desliga solenoide
  digitalWrite(pinReleSolenoide, LOW);
}

void onConnectionEstablished()
{}
  
void loop()
{
  //leitura ultassom
  //Le as informacoes do sensor, em cm
  dist = distanceSensor.measureDistanceCm();
  Serial.print("Distancia em cm: ");
  Serial.println(dist);
  volume = dist * 300;
  litro = volume / 1000;
  conversao = (map(litro, 0, 5.7, 5.7, 0));
  Serial.print(conversao);
  Serial.println(" litros");

  //leitura de temperatura
  temperatura = thermocouple.readCelsius();
  Serial.println(temperatura);
  temp = temperatura * 1.8 + 32; // Convert Celsius to Fahreinheit

  //leitura umidade
  if (digitalRead(pinSensorD)) {
     umid = "SECO";
     Serial.println("SECO");
     //digitalWrite(pinReleSolenoide, HIGH);
     
  } else {
     umid = "MOLHADO";
     Serial.println("MOLHADO");
     //digitalWrite(pinReleSolenoide, LOW);
  }

  if (conversao >= 5) {
    digitalWrite(pinReleBomba, LOW);
    bomba_onoff = "desligada";
  }
  else if (conversao <= 2) {
    digitalWrite(pinReleBomba, HIGH);
    bomba_onoff = "ligada";
    digitalWrite(pinReleSolenoide, LOW);
  }
  if (temperatura >= 37 || umid == "SECO") {
      Serial.println("temperatura ou umidade é verdade");
      if (conversao >= 3) {
          Serial.println("Caixa de agua mais de 3 litros");
          digitalWrite(pinReleSolenoide, HIGH);
          
          //executa a contagem de pulsos uma vez por segundo
          if((millis() - tempo_antes) > 1000){

          //desabilita a interrupcao para realizar a conversao do valor de pulsos
          detachInterrupt(INTERRUPCAO_SENSOR);

          //conversao do valor de pulsos para L/min
          fluxo = ((1000.0 / (millis() - tempo_antes)) * contador) / FATOR_CALIBRACAO;

          //exibicao do valor de fluxo
          Serial.print("Fluxo de: ");
          Serial.print(fluxo);
          Serial.println(" L/min");

          //calculo do volume em L passado pelo sensor
          volume_vaz = fluxo / 60;

          //armazenamento do volume
          volume_total += volume_vaz;

          //exibicao do valor de volume
          Serial.print("Volume: ");
          Serial.print(volume_total);
          Serial.println(" L");
          Serial.println();
   
          //reinicializacao do contador de pulsos
          contador = 0;

          //atualizacao da variavel tempo_antes
          tempo_antes = millis();

          //contagem de pulsos do sensor
          attachInterrupt(INTERRUPCAO_SENSOR, contador_pulso, FALLING);
          }         
       }
       else {
          digitalWrite(pinReleBomba, HIGH);
          bomba_onoff = "ligada";
       }
  }
  else {
    digitalWrite(pinReleSolenoide, LOW);
  }
  delay(5000);

//arquivo Json
  StaticJsonDocument<300> dados_temp, dados_umid, dados_ultra, dados_vaz, dados_bomb;

  dados_temp["variable"] = "temperatura"; 
  dados_temp["value"] = temp;
  dados_umid["variable"] = "umidade";
  dados_umid["value"] = umid;
  dados_ultra["variable"] = "litros_caixa";
  dados_ultra["value"] = conversao;
  dados_vaz["variable"] = "litro_gasto";
  dados_vaz["value"] = volume_total;
  dados_bomb["variable"] = "status_bomba";
  dados_bomb["value"] = bomba_onoff;

  serializeJson(dados_temp, msg1);
  Serial.println(msg1);

  serializeJson(dados_umid, msg2);
  Serial.println(msg2);

  serializeJson(dados_ultra, msg3);
  Serial.println(msg3);

  serializeJson(dados_vaz, msg4);
  Serial.println(msg4);

  serializeJson(dados_bomb, msg5);
  Serial.println(msg5);

//envio de dados
  client.publish("info/temperatura", msg1);
  client.publish("info/umidade", msg2);
  client.publish("info/ultrassom", msg3);
  client.publish("info/vazao", msg4);
  client.publish("info/bomba", msg5);
  
  delay(1000);
  client.loop();
}

//funcao chamada pela interrupcao para contagem de pulsos
void contador_pulso() {
  contador++;
}
