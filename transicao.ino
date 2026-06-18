void transicao() {
  static bool executou = false;

  if (min == 59 && sec == 59) {
    
    if (executou) return;
    executou = true;
    
    while (min > 0) {

      min--;

      nixie_min_dez = (min % 100) / 10;
      nixie_min_uni = min % 10;
      if (min == 1 ) {
        hour++;
      }
      nixie_hora_dez = (hour % 100) / 10;
      nixie_hora_uni = hour % 10;

      unsigned long start = millis();
      while (millis() - start < 4) {
        atualiza();
      }
    }
    
    if (estado_menu == 0) {
      // Se o rádio está tocando, precisamos desconectar eletricamente o RDA5807 da linha
      // de áudio (alta impedância) para o DFPlayer ser ouvido. O amplificador permanece ligado.
      // Se o rádio não está tocando, gerenciamos o amplificador normalmente.
      bool amp_gerenciado_aqui = !radio_ligado;

      if (radio_ligado) {
        rx.setAudioOutputHighImpedance(true); // Desconecta o rádio da linha — libera para o DFPlayer
      } else {
        digitalWrite(PINO_VCC_AMPLIFICADOR, LOW); // Liga o amplificador
      }

      df_play(ALDEAO);
      tone(PINO_BUZZER, 1500, 100); // Bip discreto a cada hora cheia

      // Aguarda 2 segundos para o som tocar, mantendo a multiplexação das Nixies ativas
      unsigned long start_chime = millis();
      while (millis() - start_chime < 2000) {
        atualiza();
      }

      if (radio_ligado) {
        rx.setAudioOutputHighImpedance(false); // Reconecta o rádio à linha de áudio
      } else {
        digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH); // Desliga o amplificador
      }
    }
    
  } else {
    executou = false;
  }
}
