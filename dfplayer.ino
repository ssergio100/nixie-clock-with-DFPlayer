// ==========================================
// DFPlayer Mini - Protocolo Raiz (Direto via Serial)
// ==========================================

// Função raiz para enviar os 10 bytes hexadecimais puros do DFPlayer
void df_send_cmd(uint8_t cmd, uint8_t param1, uint8_t param2) {
  uint8_t buf[10] = {0x7E, 0xFF, 0x06, cmd, 0x00, param1, param2, 0x00, 0x00, 0xEF};
  uint16_t checksum = 0;
  for (int i=1; i<7; i++) {
    checksum += buf[i];
  }
  checksum = -checksum;
  buf[7] = (uint8_t)(checksum >> 8);
  buf[8] = (uint8_t)(checksum & 0xFF);
  for (int i=0; i<10; i++) {
    dfSerial.write(buf[i]);
  }
  delay(30); // Pausa essencial para o DFPlayer processar o comando antes de receber o próximo
}

// Toca uma faixa específica 1 vez e para (ex: bip de hora, avisos curtos)
void df_play(uint16_t track) {
  df_send_cmd(0x03, (uint8_t)(track >> 8), (uint8_t)(track & 0xFF));
}

// Toca uma faixa e fica repetindo eternamente (ex: Alarme)
void df_loop(uint16_t track) {
  df_play(track); // Usa 0x03 primeiro, garantindo que o chip comece a tocar
  delay(100);
  df_send_cmd(0x19, 0x00, 0x00); // 0x19: Single Loop (Repete a faixa atual)
}

// Para a música (usado no botão soneca ou desligar alarme)
void df_stop() {
  df_send_cmd(0x16, 0x00, 0x00);
}

// Configura o volume (De 0 a 30)
void df_volume(uint8_t vol) {
  if (vol > 30) vol = 30;
  df_send_cmd(0x06, 0x00, vol);
}

// Retorna o número total de faixas presentes no cartão SD (TF)
uint16_t df_get_total_tracks() {
  // Limpa o buffer de entrada serial para evitar leituras antigas
  while(dfSerial.available()) {
    dfSerial.read();
  }

  // Comando 0x48: Query TF (Micro SD) Card files count
  df_send_cmd(0x48, 0x00, 0x00);

  // Aguarda a resposta com timeout de 300ms
  uint8_t response[10];
  uint8_t bytes_lidos = 0;
  unsigned long timeout = millis() + 300;

  while (millis() < timeout && bytes_lidos < 10) {
    if (dfSerial.available()) {
      response[bytes_lidos++] = dfSerial.read();
    }
  }

  // Se lemos os 10 bytes com o formato esperado da resposta do comando 0x48
  if (bytes_lidos == 10 && response[0] == 0x7E && response[9] == 0xEF && response[3] == 0x48) {
    uint16_t total = ((uint16_t)response[5] << 8) | response[6];
    return total;
  }

  return 0; // Retorna 0 em caso de falha/timeout ou incompatibilidade de clone
}
