/*
IPS CTESP-SEC 
Introduçao Sistemas Embebidos 23/24

Rui Lagarto           202200818
Rúben Monteiro        202200817
Gabriel Figueredo     202201067
*/

// Inclusão de bibliotecas necessárias para o código
#include <WiFi.h>
#include <time.h>

// Declaração de handles para tarefas, semáforo e fila
TaskHandle_t captarDBHandle;
TaskHandle_t obterHoraHandle;
TaskHandle_t enviarDadosHandle;
SemaphoreHandle_t semaforoBinario;
QueueHandle_t fila;

// Definição de bits para notificações de tarefas
#define CAPTAR_DB_NOTIFY  (1 << 0)
#define OBTER_HORA_NOTIFY (1 << 1)
#define ENVIAR_DADOS_NOTIFY (1 << 2)

// Configurações de rede Wi-Fi
const char *ssid = "Rrt";
const char *pass = "rx250702lx";
const char *host = "192.168.5.180";
const unsigned int porta = 8080;
WiFiClient client;

// Configuração da taxa de comunicação serial
const unsigned int baudRate = 115200;

void ligarRede(){
  //incializa o contador de tentativas
  int tentativas = 0;
  //Incializa a conexão Wi-Fi com as credenciais fornecidas
  WiFi.begin(ssid,pass);
  //Aguarda até que a conexão seja estabelecida ou o numero maximo de tentativas seja atingido
  while((WiFi.status() != WL_CONNECTED) && (tentativas < 40)){
    // Aguarda 250 milissegundos entre tentativas
    delay(250);
    //incrementa o COntador de tentativas
    tentativas++;
  }
  //verifica se a conexao Wi-Fi foi estabelecida com Sucesso
  if(WiFi.status() == WL_CONNECTED){
    //Se conectado imprime o endereço IP atribuido
    Serial.println(WiFi.localIP());

    //Configura o tempo do ESP32 usando o servidor NTP externo
    configTime(0, 0, "pool.ntp.org");
  }else{
    // SE a conexao Falhou, imprime uma mensagem de falha
    Serial.println("Falha ao Conectar no WiFi");
  }
}

void captarDB(void *pvParameters){
  //Variaveis para armazenar valores e parametros do sensor
  float tensao,decibel;
  float pressao = 20.0E-6;
  float sensibilidade = 5.0;

  int sensorPin = 34;         // Pino ao qual o sensor está conectado
  pinMode(sensorPin,INPUT);   // Configuração do pino como entrada analógica

  while(1){
    //Aguarda notificaçao para iniciar a tarefa
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);

    //Obtem o semaforo binario para evitar concorrencia
    if(xSemaphoreTake(semaforoBinario, portMAX_DELAY)){
      // Imprime mensagem indicando que a tarefa está sendo executada
      Serial.println("Executando -> Tarefa CaptarDB");
      decibel = 0.0;

      // Loop para realizar 200 leituras do sensor e calcular a média de decibéis
      for(int i=0;i<200;i++){
        int sensorValor = analogRead(sensorPin);
        
        //Converte a leitura do sensor para tensao
        tensao = (sensorValor / 4095.0) * 5.0;

        //calcula o nivel de decibeis e acumula
        decibel += (20 * log10(tensao/sensibilidade/pressao));
      }
      //Calcula a media dos decibeis e converte para String
      String db = String(decibel / 200.0);
      
      //Envia o Resultado para a fila
      xQueueSend(fila,&db,portMAX_DELAY);

      //liberta o semaforo binario
      xSemaphoreGive(semaforoBinario);

      //Notifica a tarefa de obtençao de horas para iniciar
      xTaskNotifyGive(obterHoraHandle);
    }
  }
}

void obterHoraAtual(void *pvParameters){
  //Variavel para armazenar a hora atual como String
  String tempoAtual;
  while(1){
    //Estrutura para armazenar as informaçoes de tempo
    struct tm timeinfo;

    //Aguarda notificaçao para iniciar a tarefa
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);

    //Obtem o semaforo binario para evitar concorrencia
    if(xSemaphoreTake(semaforoBinario,portMAX_DELAY)){
      //Imprime mensagem indicando qua tarefa esta a ser executada
      Serial.println("Executando -> Tarefa Obter_Hora");

      //Obtem a hora local do servidor NTP
      if(!getLocalTime(&timeinfo)){
        //Se falhar a obter a hora imprime uma menssagem e retorna
        Serial.println("-----------> Falha a Obter Tempo");
        return;
      }

      //converte a hora para string usando funçao asctime
      tempoAtual = String(asctime(&timeinfo));

      //envia a hora atual para a fila
      xQueueSend(fila,&tempoAtual,portMAX_DELAY);

      //liberta o semaforo binario
      xSemaphoreGive(semaforoBinario);

      //Notifica a tarefa de envio de dados para iniciar
      xTaskNotifyGive(enviarDadosHandle);
    }
  }
}

void enviarDados(void *pvParameters){
  //Variaveis para armazenar dados e serem enviados
  String db,tempoAtual;

  while(1){
    //aguarda notificaçao para iniciar a tarefa
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    //obtem o semaforo binario para evitar concorrencia
    if(xSemaphoreTake(semaforoBinario,portMAX_DELAY)){
      // Imprime mensagem indicando que a tarefa está sendo executada
      Serial.println("Executando -> Tarefa Enviar_Dados");
      
      //verifica se o client nao esta conectado
      if(!client.connected()){
        //se nao estiver,tentar reconectar
        ligarRede();
        if(!client.connect(host,porta)){
          //se falhar a reconexao,espera e retorna 
          delay(1000);
          return;
        }
      }

      //recebe dados da Fila(decibeis e tempo)
      if(xQueueReceive(fila,&db,portMAX_DELAY)){
        if(xQueueReceive(fila,&tempoAtual,portMAX_DELAY)){
          // Cria uma mensagem concatenando nível de decibéis e tempo
          String msg = db+","+tempoAtual;
          Serial.println(msg);
          //envia a mensagem para o servidor remoto
          client.print(msg);
          
          //aguarda um intervalo entes de continuar
          delay(3500);
        }
      }
      //liberta o semaforo binario
      xSemaphoreGive(semaforoBinario);

      // Reinicializa a fila, removendo todos os itens
      xQueueReset(fila);

      //notifica a tarefa de captura de dados para inciar
      xTaskNotifyGive(captarDBHandle);
    }
  }
}
void setup(){
  //inicia a comunicaçao serial
  Serial.begin(baudRate);

  //agurda até que a comunicaçao esteja disponivel
  while(!Serial);

  //inicia a conexao Wi-Fi e configura o tempo
  ligarRede();

  //Define o tamanho para a criaçao das tarefas
  unsigned int tamanho = 5000;

  //Cria um semaforo binario e inicializa-o
  semaforoBinario = xSemaphoreCreateBinary();
  xSemaphoreGive(semaforoBinario);

  //Cria uma fila com capacidade para 2 itens do tipo String
  fila = xQueueCreate(2,sizeof(String));

  //verifica se a fila foi criada com sucesso
  if (fila == NULL){
    //Reinicia o ESP32 se a criaçao da fila falhar
    ESP.restart();
  }else{
    // Cria as tarefas captarDB, obterHoraAtual e enviarDados
    xTaskCreate(captarDB,"Task_captar_db",tamanho,NULL,1,&captarDBHandle);
    xTaskCreate(obterHoraAtual,"Task_obter_hora",tamanho,NULL,1,&obterHoraHandle);
    xTaskCreate(enviarDados,"Task_Enviar_Dados",tamanho,NULL,2,&enviarDadosHandle);
    
    // Notifica a tarefa captarDB para iniciar sua execução
    xTaskNotifyGive(captarDBHandle);
  }
}
void loop(){}