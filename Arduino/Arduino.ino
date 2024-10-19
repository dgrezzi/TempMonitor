#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

// Pinos para o LCD (ajuste conforme a sua conexão)
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Pinos para os sensores LM35 e botão
const int lm35Pin1 = A0;  // Pino do primeiro sensor LM35
const int lm35Pin2 = A1;  // Pino do segundo sensor LM35
const int lm35Pin3 = A2;  // Pino do terceiro sensor LM35
const int buttonPin = 2;  // Pino do botão

// Configurações de Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC Address
IPAddress ip(192, 168, 0, 101);  // IP do Arduino
IPAddress server(192, 168, 0, 100);  // Endereço IP do servidor (Raspberry Pi)

// Cliente Ethernet
EthernetClient client;

// Variáveis
unsigned long previousMillis = 0;
const long interval = 60000; // Envia a cada 60 segundos (60000 ms)

// Configuração inicial
void setup() {
  Serial.begin(9600);

  // Inicializa o LCD
  lcd.begin(16, 2);
  lcd.print("Iniciando...");

  // Inicializa o Ethernet
  Ethernet.begin(mac, ip);

  // Aguarda um tempo para garantir que a inicialização seja concluída
  delay(1000);

  // Verifica se o IP foi atribuído corretamente
  if (Ethernet.localIP() == IPAddress(0, 0, 0, 0)) {
    Serial.println("Falha ao configurar Ethernet.");
    while (true); // Loop infinito se a inicialização falhar
  }

  // Aguarda conexão com o servidor
  delay(1000);
  Serial.println("Conectado!");

  // Configura pinos
  pinMode(buttonPin, INPUT_PULLUP);
  lcd.clear();
}

// Função para ler temperatura do LM35
void readTemperatures(float &temp1, float &temp2, float &temp3) {
  int sensorValue1 = analogRead(lm35Pin1);
  int sensorValue2 = analogRead(lm35Pin2);
  int sensorValue3 = analogRead(lm35Pin3);

  // Conversão para Celsius
  temp1 = sensorValue1 * (5.0 / 1023.0) * 100; // LM35 fornece 10mV por grau Celsius
  temp2 = sensorValue2 * (5.0 / 1023.0) * 100;
  temp3 = sensorValue3 * (5.0 / 1023.0) * 100;
}

// Função para enviar os dados para o servidor
void sendData(float temp1, float temp2, float temp3) {
  if (client.connect(server, 80)) {  // Conecta ao servidor na porta 80
    client.print("POST /temperatures HTTP/1.1\r\n");
    client.print("Host: 192.168.0.100\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Connection: close\r\n");
    client.print("Content-Length: ");

    String data = "{\"temperature1\": " + String(temp1) +
                  ", \"temperature2\": " + String(temp2) +
                  ", \"temperature3\": " + String(temp3) + "}";
    client.println(data.length());
    client.println();
    client.println(data);

    Serial.println("Dados enviados ao servidor.");

    client.stop();
  } else {
    Serial.println("Falha na conexão com o servidor.");
  }
}

void loop() {
  // Lê as temperaturas
  float temperature1, temperature2, temperature3;
  readTemperatures(temperature1, temperature2, temperature3);

  // Exibe no LCD
  lcd.setCursor(0, 0);
  lcd.print("T1:");
  lcd.print(temperature1);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("T2:");
  lcd.print(temperature2);
  lcd.print(" C");

  // Verifica se o botão foi pressionado
  if (digitalRead(buttonPin) == LOW) {
    sendData(temperature1, temperature2, temperature3);
  }

  // Envia os dados automaticamente a cada intervalo de tempo
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendData(temperature1, temperature2, temperature3);
  }

  delay(500); // Aguarda meio segundo para atualização do LCD
}
