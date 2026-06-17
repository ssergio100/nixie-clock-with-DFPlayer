#include "Arduino.h"
#include "SoftwareSerial.h"

// ==========================================
// CONFIGURAÇÃO DE DIAGNÓSTICO
// Se não funcionar na primeira tentativa, altere para: SoftwareSerial dfSerial(11, 10);
// ==========================================
SoftwareSerial dfSerial(11, 10); // RX (Pino 10), TX (Pino 11)

void df_send_cmd(uint8_t cmd, uint8_t param1, uint8_t param2) {
  uint8_t buffer[10] = {0x7E, 0xFF, 0x06, cmd, 0x00, param1, param2, 0x00, 0x00, 0xEF};
  
  uint16_t checksum = 0;
  for (int i = 1; i < 7; i++) {
    checksum += buffer[i];
  }
  checksum = -checksum;
  
  buffer[7] = (uint8_t)(checksum >> 8);
  buffer[8] = (uint8_t)(checksum & 0xFF);
  
  for (int i = 0; i < 10; i++) {
    dfSerial.write(buffer[i]);
  }
  delay(100); // Maior intervalo para garantir processamento em clones lentos
}

void setup() {
  Serial.begin(9600);
  dfSerial.begin(9600);
  
  Serial.println();
  Serial.println(F("--- TESTE MP3 ISOLADO (MODO DIAGNOSTICO RAIZ) ---"));
  Serial.println(F("Tentando enviar comandos de inicializacao..."));

  delay(2000); // Aguarda boot eletrico do DFPlayer

  // 1. Configurar Volume para 25
  Serial.println(F("Enviando volume 25..."));
  df_send_cmd(0x06, 0x00, 25); 
  delay(500);

  // 2. Tentar tocar com comando 0x0F (Pasta 01, Musica 001) -> Ideal para a estrutura de pastas do seu cartao
  Serial.println(F("Tentando tocar Pasta 01, Arquivo 001 (Comando 0x0F)..."));
  df_send_cmd(0x0F, 0x01, 0x01); 
  delay(500);

  // 3. Tentar tocar com comando 0x03 (Musica 0001 na raiz/ordem fisica) como backup
  Serial.println(F("Tentando tocar Musica 0001 via indice fisico (Comando 0x03)..."));
  df_send_cmd(0x03, 0x00, 0x01);
  delay(500);

  Serial.println(F("Comandos enviados!"));
  Serial.println(F("Se o LED azul do DFPlayer NAO acendeu, experimente editar o codigo e inverter os pinos:"));
  Serial.println(F("Altere para: SoftwareSerial dfSerial(11, 10);"));
}

void loop() {
  // Apenas pisca o LED do Arduino (pino 13) para mostrar que o processador nao travou
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}
