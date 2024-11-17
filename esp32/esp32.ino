#include <WiFi.h>

// Credenciais Wi-Fi
const char* ssid = "XXXXX";  // Substitua pelo seu SSID
const char* password = "XXXXX";         // Substitua pela sua senha

// Pinos para os sensores LM35
const int lm35Pins[] = { 36, 39, 34, 35, 32 };                  // Pinos analógicos do ESP32
const int controlPins[] = { 5, 18, 19, 21, 22 };                // Pinos de controle
const int numSensors = sizeof(lm35Pins) / sizeof(lm35Pins[0]);  // Número de sensores

// Configurações do servidor
const char* server = "192.168.1.3";  // IP do servidor
const int serverPort = 8000;

WiFiClient client;  // Cliente Wi-Fi

// Variáveis
unsigned long previousMillis = 0;
long interval = 60;  // Intervalo em segundos (padrão de 60 segundos)

// Função para conectar ao Wi-Fi
void connectWiFi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);

  // Configuração em modo Station (Cliente)
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_11dBm); // Define a potência de transmissão

  // Iniciar a conexão Wi-Fi
  WiFi.begin(ssid, password);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");

    // Verificar o status da conexão a cada tentativa
    if (attempt > 30) {
      Serial.println("\nFalha ao conectar. Status do Wi-Fi: ");
      Serial.println(WiFi.status());  // Exibe o status do Wi-Fi
      return;
    }
    attempt++;
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
    if (digitalRead(controlPins[i]) == LOW) {
      int milliVolts = analogReadMilliVolts(lm35Pins[i]);
      temps[i] = milliVolts / 10.0;  // Conversão direta para °C
    } else {
      temps[i] = NAN;
    }
  }
}

// Função para enviar os dados para o servidor
void sendData(float* temps) {
  if (client.connect(server, serverPort)) {
    Serial.print("Conectado! ");

    String data = "{";
    bool first = true;

    for (int i = 0; i < numSensors; i++) {
      if (!isnan(temps[i])) {  // Apenas adiciona sensores conectados
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

    float temperatures[numSensors];  // Usando uma variável local
    readTemperatures(temperatures);
    sendData(temperatures);
  }

  delay(100);  // Delay curto para alimentar o watchdog e evitar resets
}
