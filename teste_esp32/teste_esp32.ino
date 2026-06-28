#include <WiFi.h>
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h" // Usamos o DAC de hardware nativo do ESP32

// ==========================================
// CONFIGURAÇÃO DA REDE WIFI
// ==========================================
const char *ssid     = "Psiquititidadida";
const char *password = "Merovingian@1130";

// URL da rádio web (SomaFM - Deep Space One)
const char *URL = "http://ice4.somafm.com/deepspaceone-128-mp3";

// Objetos para a reprodução de áudio
AudioFileSourceICYStream *file = nullptr;
AudioFileSourceBuffer *buff = nullptr;
AudioGeneratorMP3 *mp3 = nullptr;
AudioOutputI2S *out = nullptr;

// Pino do LED onboard
#if defined(LED_BUILTIN)
  #define LED_PIN LED_BUILTIN
#else
  #define LED_PIN 2
#endif

// Callback executado ao receber metadados da rádio (ex: Nome da música)
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string) {
  (void)cbData;
  (void)isUnicode;
  Serial.printf("[Metadados] %s: %s\n", type, string);
}

// Callback executado para exibir status/erros do buffer
void StatusCallback(void *cbData, int code, const char *string) {
  (void)cbData;
  Serial.printf("[Status] Codigo %d: %s\n", code, string);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("\n=========================================");
  Serial.println("--- Teste Rádio Web MP3 via DAC Físico v2.x ---");
  Serial.printf("Modelo do Chip: %s\n", ESP.getChipModel());
  Serial.println("=========================================\n");

  // Conecta ao Wi-Fi
  Serial.printf("Conectando a rede: %s\n", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Pisca o LED durante conexão
  }
  
  digitalWrite(LED_PIN, HIGH); // LED aceso fixo indica WiFi conectado
  Serial.println("\nWiFi Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("-----------------------------------------\n");

  // Inicializa o DAC físico analógico do ESP32 na porta I2S 0
  // O áudio sairá de forma analógica real de 8 bits em:
  //   -> GPIO25 (Canal Esquerdo / Left) - Pino 9 da shield do usuário
  //   -> GPIO26 (Canal Direito / Right)
  out = new AudioOutputI2S(0, AudioOutputI2S::INTERNAL_DAC);
  
  // Define o ganho de software máximo (4.0)
  out->SetGain(4.0);
  
  Serial.println("[Áudio] DAC de hardware ativado nos pinos GPIO25 e GPIO26.");

  // Inicializa a stream de áudio HTTP/ICY
  file = new AudioFileSourceICYStream(URL);
  file->RegisterMetadataCB(MDCallback, NULL);

  // Configura um buffer de áudio de 32KB para fluxo de rede 100% contínuo
  buff = new AudioFileSourceBuffer(file, 32768);
  buff->RegisterStatusCB(StatusCallback, NULL);

  // Inicializa o decodificador MP3
  mp3 = new AudioGeneratorMP3();
  
  Serial.println("Iniciando reprodução da rádio...");
  if (mp3->begin(buff, out)) {
    Serial.println("Decodificador MP3 iniciado com sucesso!");
  } else {
    Serial.println("Falha ao iniciar o decodificador MP3.");
  }
}

void loop() {
  static unsigned long ultimoRelatorio = 0;

  // Processa o áudio continuamente
  if (mp3->isRunning()) {
    if (millis() - ultimoRelatorio > 5000) {
      ultimoRelatorio = millis();
      Serial.println("[Audio] Reproduzindo rádio web...");
    }

    // Processa a decodificação de um frame MP3.
    if (!mp3->loop()) {
      mp3->stop();
      Serial.println("[Audio] Parado ou Fim da Stream.");
    }
  } else {
    Serial.println("[Audio] Rádio não está executando. Tentando reiniciar em 5 segundos...");
    delay(5000);
    if (!mp3->begin(buff, out)) {
      Serial.println("[Erro] Falha ao reiniciar a rádio.");
    }
  }
}
