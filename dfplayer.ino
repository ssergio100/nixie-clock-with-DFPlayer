// ==========================================
// DFPlayer Mini - Usando a Biblioteca DFRobotDFPlayerMini
// ==========================================

// Toca uma faixa específica 1 vez e para (ex: bip de hora, avisos curtos)
void df_play(uint16_t track) {
  myDFPlayer.play(track);
}

// Toca uma faixa e fica repetindo eternamente (ex: Alarme)
void df_loop(uint16_t track) {
  myDFPlayer.loop(track);
}

// Para a música (usado no botão soneca ou desligar alarme)
void df_stop() {
  myDFPlayer.stop();
}

// Configura o volume (De 0 a 30)
void df_volume(uint8_t vol) {
  if (vol > 30) vol = 30;
  myDFPlayer.volume(vol);
}

// Wrapper para retrocompatibilidade
void df_send_cmd(uint8_t cmd, uint8_t param1, uint8_t param2) {
  if (cmd == 0x0F) { // Play Folder
    myDFPlayer.playFolder(param1, param2);
  } else if (cmd == 0x06) { // Volume
    myDFPlayer.volume(param2);
  } else if (cmd == 0x03) { // Play Track
    myDFPlayer.play(param2);
  } else if (cmd == 0x16) { // Stop
    myDFPlayer.stop();
  } else if (cmd == 0x08) { // Loop Track
    myDFPlayer.loop(param2);
  }
}
