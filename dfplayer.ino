// ==========================================
// DFPlayer Mini - Modo "Raiz" (Sem biblioteca pesada)
// Economiza memória RAM preciosa do Arduino
// ==========================================

void df_send_cmd(uint8_t cmd, uint8_t param1, uint8_t param2) {
  // O pacote padrão do DFPlayer Mini tem sempre 10 bytes:
  // [Start] [Version] [Length] [CMD] [Feedback] [Param1] [Param2] [Checksum_H] [Checksum_L] [End]
  
  uint8_t buffer[10] = {0x7E, 0xFF, 0x06, cmd, 0x00, param1, param2, 0x00, 0x00, 0xEF};
  
  // Calcula o Checksum
  uint16_t checksum = 0;
  for (int i = 1; i < 7; i++) {
    checksum += buffer[i];
  }
  checksum = -checksum;
  
  buffer[7] = (uint8_t)(checksum >> 8);
  buffer[8] = (uint8_t)(checksum & 0xFF);
  
  // Envia o pacote para o módulo via SoftwareSerial
  for (int i = 0; i < 10; i++) {
    dfSerial.write(buffer[i]);
  }
  delay(20); // Pequeno atraso para o chip processar o comando
}

// Toca uma faixa específica 1 vez e para (ex: bip de hora, avisos curtos)
void df_play(uint16_t track) {
  df_send_cmd(0x03, (uint8_t)(track >> 8), (uint8_t)(track & 0xFF));
}

// Toca uma faixa e fica repetindo eternamente (ex: Alarme)
void df_loop(uint16_t track) {
  df_send_cmd(0x08, (uint8_t)(track >> 8), (uint8_t)(track & 0xFF));
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
