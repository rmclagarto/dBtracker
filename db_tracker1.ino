#include <WiFi.h>
#include <time.h>

// Definição do pino ao qual o sensor está conectado
const unsigned int sensor = 34;

// Configurações de rede Wi-Fi
const char *ssid = "Rrt";
const char *pass = "rx250702lx";
const char *host = "192.168.50.180";
const unsigned int porta = 8080;
WiFiClient client;

// Estrutura para armazenar dados do sensor e tempo
struct DataVar {
  String decibelValue;
  String currentTime;
};
DataVar var;

void setup() {
  Serial.begin(115200);

  // Aguarda até que a comunicação serial esteja disponível
  while(!Serial);

  // Configura o pino do sensor como entrada
  pinMode(sensor, INPUT);

  // Inicializa a conexão Wi-Fi e configura o tempo usando um servidor NTP externo
  ligarWIFI();
  configTime(0, 0, "pool.ntp.org");
}
void captarDB() {
  float tensao;
  float db = 0;
  int sensibilidade = 2.5;
  float presaoSonora;

  // Realiza 200 leituras do sensor e calcula a média dos decibéis
  for (int i = 0; i < 200; i++)
  {
    // Converte a leitura do sensor para tensão
    tensao = (digitalRead(sensor) / 1023.0) * 5;

    // Calcula a pressão sonora
    presaoSonora = tensao /sensibilidade;

    // Calcula os decibéis e acumula
    db += (20 * log10(presaoSonora / 20e-6)); 
    delay(10);
  }

  // Calcula a média dos decibéis e armazena na estrutura
  var.decibelValue = String(db / 200.00);
}
void obterHoraAtual() {
  struct tm timeinfo;

  // Obtém a hora local do servidor NTP
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha a Obter tempo");
    return;
  }
  // Converte a hora para string usando a função asctime
  var.currentTime = String(asctime(&timeinfo));
}
void ligarWIFI(){
  // Inicia a conexão Wi-Fi com as credenciais fornecidas
  WiFi.begin(ssid,pass);

  int tentativas = 0;

  // Aguarda até que a conexão seja estabelecida ou o número máximo de tentativas seja atingido
  while(WiFi.status() != WL_CONNECTED && tentativas < 20)
  {
    delay(1000);
    tentativas++;
  }

  // Verifica se a conexão Wi-Fi foi estabelecida com sucesso
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Falha a Conectar na Rede WiFi");
  }
}

void loop() {
  // Captura dados do sensor e obtém a hora atual
  captarDB();
  obterHoraAtual();

  // Verifica se o cliente está conectado
  if (!client.connected()) 
  {
    // Tenta reconectar se não estiver conectado
    ligarWIFI();
    if (!client.connect(host, porta)) 
    {
      delay(1000);
      return;
    }
  }

  // Cria uma mensagem concatenando nível de decibéis e tempo
  String msg = String(var.decibelValue) + "," + var.currentTime;
  
  // Envia a mensagem para o servidor remoto
  client.print(msg);

  // Aguarda um intervalo antes de continuar
  delay(3500);
}