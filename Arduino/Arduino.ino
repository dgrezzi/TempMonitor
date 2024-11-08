#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// Pinos para os sensores LM35
const int lm35Pins[] = {A0, A1, A2, A3, A4}; // Pinos dos sensores
const int numSensors = sizeof(lm35Pins) / sizeof(lm35Pins[0]); // Número de sensores

// Configurações de Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC Address
IPAddress ip(192, 168, 1, 100); // IP padrão
IPAddress dns(8, 8, 8, 8); // DNS padrão (Google DNS)
IPAddress gateway(192, 168, 1, 1); // Gateway padrão
IPAddress server(192, 168, 0, 100); // Endereço IP do servidor
IPAddress subnet(255, 255, 255, 0); // Máscara de sub-rede padrão

EthernetClient client; // Cliente Ethernet

// Variáveis
unsigned long previousMillis = 0;
long interval = 60; // Intervalo em segundos (padrão de 60 segundos)

// Função para ler configurações de rede do arquivo
void readNetworkConfig() {
  File configFile = SD.open("config.txt");
  if (configFile) {
    while (configFile.available()) {
      String line = configFile.readStringUntil('\n');
      if (line.startsWith("IP:")) {
        String ipString = line.substring(3);
        ip.fromString(ipString);
      } else if (line.startsWith("GATEWAY:")) {
        String gatewayString = line.substring(8);
        gateway.fromString(gatewayString);
      } else if (line.startsWith("SUBNET:")) {
        String subnetString = line.substring(7);
        subnet.fromString(subnetString);
      } else if (line.startsWith("DNS:")) {
        String dnsString = line.substring(4);
        dns.fromString(dnsString);
      } else if (line.startsWith("SERVER:")) {
        String serverString = line.substring(7);
        server.fromString(serverString);
      } else if (line.startsWith("DELAY:")) {
        String intervalString = line.substring(6);
        interval = intervalString.toInt(); // Converte a string para um inteiro
      } else {
        Serial.print(F("Linha ignorada: "));
        Serial.println(line); // Exibe linhas que não correspondem
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
    // Lê as configurações de rede se o SD for inicializado com sucesso
    readNetworkConfig();
  }

  // Inicia a conexão Ethernet
  Ethernet.begin(mac, ip, dns, gateway, subnet); // Adicione subnet aqui
  delay(1000); // Aguarda para garantir que a inicialização seja concluída

  // Verifica se a configuração de IP foi aplicada corretamente
  if (Ethernet.localIP() == IPAddress(0, 0, 0, 0)) {
    Serial.println(F("Falha ao configurar Ethernet."));
    while (true); // Loop infinito se a inicialização falhar
  } else {
    Serial.print(F("Conectado com sucesso!\nEndereço IP: "));
    Serial.println(Ethernet.localIP());
    Serial.print(F("Server IP: "));
    Serial.println(server);
    Serial.print(F("Delay: "));
    Serial.println(interval);
  }
}

// Função para ler temperatura do LM35
void readTemperatures(int* temps) {
  for (int i = 0; i < numSensors; i++) {
    temps[i] = analogRead(lm35Pins[i]) * (5000 / 1023); // Convertendo para milivolts
  }
}

// Função para enviar os dados para o servidor
void sendData(int* temps) {
  if (client.connect(server, 8000)) {
    client.print("POST /data HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(server); // Aqui usamos a variável `server`
    client.print("\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Connection: close\r\n");

    String data = "{\"sensor_1\": " + String(temps[0] / 100.0) +
                  ", \"sensor_2\": " + String(temps[1] / 100.0) +
                  ", \"sensor_3\": " + String(temps[2] / 100.0) +
                  ", \"sensor_4\": " + String(temps[3] / 100.0) +
                  ", \"sensor_5\": " + String(temps[4] / 100.0) + "}";
    
    client.print("Content-Length: " + String(data.length()) + "\r\n\r\n");
    client.println(data);
    client.stop();
  } else {
    Serial.println(F("Falha ao conectar ao servidor."));
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= (interval * 1000)) { // Converte segundos para milissegundos
    previousMillis = currentMillis;

    int temperatures[numSensors]; // Usando uma variável local
    readTemperatures(temperatures);
    sendData(temperatures);
  }

  delay(500);
}
