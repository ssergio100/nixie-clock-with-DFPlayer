void verificarChime() {
  static bool executou = false;

  // O Chime toca no primeiro segundo da hora cheia (ex: 12:00:00)
  if (min == 0 && sec == 0) {
    if (executou) return;
    executou = true;

    if (state.radio_ligado) {
      rx.setAudioOutputHighImpedance(true); // Desconecta o rádio da linha para o DFPlayer
    } else {
      digitalWrite(PINO_VCC_AMPLIFICADOR, LOW); // Liga o amplificador
    }

    dfplayer_volume(cfg.chime_volume);
    delay(50);
    dfplayer_play(cfg.chime_som);

    // Aguarda 2 segundos para o som tocar
    delay(2000);

    if (state.radio_ligado) {
      rx.setAudioOutputHighImpedance(false); // Reconecta o rádio à linha de áudio
    } else {
      dfplayer_stop();
      digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH); // Desliga o amplificador
    }
  } else {
    executou = false;
  }
}
