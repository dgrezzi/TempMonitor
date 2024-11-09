#include <WiFi.h>
#include <SD.h>

// Credenciais Wi-Fi
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA";

// Pinos para os sensores LM35
const int lm35Pins[] = {A0, A1, A2, A3, A4}; // Pinos dos sensores
const int controlPins[] = {5, 6, 7, 8, 9};   // Pinos de controle
const int numSensors = sizeof(lm35Pins) / sizeof(lm35Pins[0]); // Número de sensores

// Configurações do servidor
const char* server = "192.168.0.100"; // IP do servidor
const int serverPort = 8000;

WiFiClient client; // Cliente Wi-Fi

// Variáveis
unsigned long previousMillis = 0;
long interval = 60; // Intervalo em segundos (padrão de 60 segundos)

// Função para conectar ao Wi-Fi
void connectWiFi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Configuração inicial
void setup() {
  Serial.begin(115200);

  // Configura os pinos de controle como INPUT_PULLUP
  for (int i = 0; i < numSensors; i++) {
    pinMode(controlPins[i], INPUT_PULLUP);
  }

  // Conectar ao Wi-Fi
  connectWiFi();
}

// Função para ler temperatura dos sensores LM35
void readTemperatures(float* temps) {
  for (int i = 0; i < numSensors; i++) {
    // Verifica se o pino de controle está LOW (jumper conectado)
    if (digitalRead(controlPins[i]) == LOW) {
      int sensorValue = analogRead(lm35Pins[i]);
      temps[i] = sensorValue * (5000.0 / 1023.0) / 10.0; // Converte para Celsius
    } else {
      temps[i] = NAN; // Define como não disponível (sensor desconectado)
    }
  }
}

// Função para enviar os dados para o servidor
void sendData(float* temps) {
  if (client.connect(server, serverPort)) {
    Serial.println("Conectado ao servidor!");

    String data = "{";
    bool first = true;

    for (int i = 0; i < numSensors; i++) {
      if (!isnan(temps[i])) { // Apenas adiciona sensores conectados
        if (!first) data += ", ";
        data += "\"sensor_" + String(i + 1) + "\": " + String(temps[i], 1);
        first = false;
      }
    }
    data += "}";

    // Envia a solicitação HTTP
    client.print("POST /data HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(server);
    client.print("\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Connection: close\r\n");
    client.print("Content-Length: " + String(data.length()) + "\r\n\r\n");
    client.print(data);
    client.stop();

    Serial.println("Dados enviados: " + data);
  } else {
    Serial.println("Falha ao conectar ao servidor.");
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= (interval * 1000)) {
    previousMillis = currentMillis;

    float temperatures[numSensors]; // Usando uma variável local
    readTemperatures(temperatures);
    sendData(temperatures);
  }

  delay(500);
}
