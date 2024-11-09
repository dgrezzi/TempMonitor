#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// Pinos para os sensores LM35
const int lm35Pins[] = {A0, A1, A2, A3, A4}; // Pinos dos sensores
const int lm35ControlPins[] = {5, 6, 7, 8, 9}; // Pinos de controle para cada sensor
const int numSensors = sizeof(lm35Pins) / sizeof(lm35Pins[0]); // Número de sensores

// Configurações de Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 100);
IPAddress dns(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress server(192, 168, 0, 100);
IPAddress subnet(255, 255, 255, 0);

EthernetClient client;

// Variáveis
unsigned long previousMillis = 0;
long interval = 60; // Intervalo em segundos

// Função para ler configurações de rede do arquivo
void readNetworkConfig() {
  File configFile = SD.open("config.txt");
  if (configFile) {
    while (configFile.available()) {
      String line = configFile.readStringUntil('\n');
      if (line.startsWith("IP:")) {
        ip.fromString(line.substring(3));
      } else if (line.startsWith("GATEWAY:")) {
        gateway.fromString(line.substring(8));
      } else if (line.startsWith("SUBNET:")) {
        subnet.fromString(line.substring(7));
      } else if (line.startsWith("DNS:")) {
        dns.fromString(line.substring(4));
      } else if (line.startsWith("SERVER:")) {
        server.fromString(line.substring(7));
      } else if (line.startsWith("DELAY:")) {
        interval = line.substring(6).toInt();
      } else {
        Serial.print(F("Linha ignorada: "));
        Serial.println(line);
      }
    }
    Serial.println(F("Arquivo de configuração lido com sucesso!"));
    configFile.close();
  } else {
    Serial.println(F("Falha ao abrir o arquivo de configuração. Usando configuração padrão de IP."));
  }
}

// Configuração inicial
void setup() {
  Serial.begin(9600);
  
  // Inicializa o SD
  if (!SD.begin(4)) {
    Serial.println(F("Falha ao inicializar o cartão SD. Usando configuração padrão de IP."));
  } else {
    readNetworkConfig();
  }

  // Inicia a conexão Ethernet
  Ethernet.begin(mac, ip, dns, gateway, subnet);
  delay(1000);

  if (Ethernet.localIP() == IPAddress(0, 0, 0, 0)) {
    Serial.println(F("Falha ao configurar Ethernet."));
    while (true);
  } else {
    Serial.print(F("Conectado com sucesso! Endereço IP: "));
    Serial.println(Ethernet.localIP());
  }

  // Configura os pinos de controle como entrada
  for (int i = 0; i < numSensors; i++) {
    pinMode(lm35ControlPins[i], INPUT_PULLUP);
  }
}

// Função para ler temperatura do LM35
void readTemperatures(int* temps) {
  for (int i = 0; i < numSensors; i++) {
    if (digitalRead(lm35ControlPins[i]) == LOW) { // Verifica se o pino de controle está jumpeado
      temps[i] = analogRead(lm35Pins[i]) * (5000 / 1023);
    } else {
      temps[i] = -1; // Define como -1 para indicar que não foi lido
    }
  }
}

// Função para enviar os dados para o servidor
void sendData(int* temps) {
  if (client.connect(server, 8000)) {
    client.print("POST /data HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(server);
    client.print("\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Connection: close\r\n");

    String data = "{";
    bool firstEntry = true;

    // Construir o JSON apenas com sensores lidos
    for (int i = 0; i < numSensors; i++) {
      if (temps[i] != -1) { // Inclui no JSON apenas os sensores lidos
        if (!firstEntry) {
          data += ", ";
        }
        data += "\"sensor_" + String(i + 1) + "\": " + String(temps[i] / 100.0);
        firstEntry = false;
      }
    }

    data += "}";

    client.print("Content-Length: " + String(data.length()) + "\r\n\r\n");
    client.println(data);
    client.stop();

    Serial.println(F("Dados enviados para o servidor:"));
    Serial.println(data);
  } else {
    Serial.println(F("Falha ao conectar ao servidor."));
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= (interval * 1000)) {
    previousMillis = currentMillis;

    int temperatures[numSensors];
    readTemperatures(temperatures);
    sendData(temperatures);
  }

  delay(500);
}
