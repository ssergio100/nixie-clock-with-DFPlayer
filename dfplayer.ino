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
