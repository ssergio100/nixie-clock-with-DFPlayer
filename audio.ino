// Gerenciador das fontes de audio usadas pelo alarme e pelo chime.

enum EstadoAudioGerenciado : uint8_t {
  AUDIO_GERENCIADO_LIVRE = 0,
  AUDIO_GERENCIADO_ALARME_MP3,
  AUDIO_GERENCIADO_ALARME_RADIO,
  AUDIO_GERENCIADO_CHIME
};

EstadoAudioGerenciado estado_audio_gerenciado = AUDIO_GERENCIADO_LIVRE;
bool radio_ativo_antes_chime = false;
unsigned long chime_inicio_ms = 0;

const unsigned long DURACAO_CHIME_MS = 2000;

void finalizarChime() {
  if (estado_audio_gerenciado != AUDIO_GERENCIADO_CHIME) {
    return;
  }

  dfplayer_stop();

  if (radio_ativo_antes_chime && state.radio_ligado) {
    rx.setAudioOutputHighImpedance(false);
    digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
  } else {
    rx.setAudioOutputHighImpedance(true);
    digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
  }

  estado_audio_gerenciado = AUDIO_GERENCIADO_LIVRE;
}

void cancelarChime() {
  finalizarChime();
}

bool iniciarChime() {
  // O alarme sempre tem prioridade sobre o aviso de hora cheia.
  if (state.alarme_tocando || state.em_soneca ||
      estado_audio_gerenciado == AUDIO_GERENCIADO_ALARME_MP3 ||
      estado_audio_gerenciado == AUDIO_GERENCIADO_ALARME_RADIO ||
      estado_audio_gerenciado == AUDIO_GERENCIADO_CHIME) {
    return false;
  }

  cancelarBuscaRadio();
  radio_ativo_antes_chime = state.radio_ligado;
  if (radio_ativo_antes_chime) {
    rx.setAudioOutputHighImpedance(true);
  } else {
    digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
  }

  dfplayer_volume(cfg.chime_volume);
  dfplayer_play(cfg.chime_som);
  chime_inicio_ms = millis();
  estado_audio_gerenciado = AUDIO_GERENCIADO_CHIME;
  return true;
}

void iniciarAudioAlarme() {
  cancelarBuscaRadio();
  cancelarChime();

  EstadoAudioGerenciado estado_desejado =
      cfg.alarme_radio ? AUDIO_GERENCIADO_ALARME_RADIO
                       : AUDIO_GERENCIADO_ALARME_MP3;
  if (estado_audio_gerenciado == estado_desejado) {
    return;
  }

  if (estado_audio_gerenciado == AUDIO_GERENCIADO_ALARME_MP3) {
    dfplayer_stop();
  } else if (estado_audio_gerenciado == AUDIO_GERENCIADO_ALARME_RADIO) {
    rx.setAudioOutputHighImpedance(true);
  }

  if (cfg.alarme_radio) {
    state.radio_ligado = true;
    state.volume_radio = 5;
    rx.setFrequency(cfg.radio_frequencia);
    rx.setVolume(state.volume_radio);
    rx.setMute(false);
    rx.setAudioOutputHighImpedance(false);
    digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
  } else {
    rx.setAudioOutputHighImpedance(true);
    state.radio_ligado = false;
    digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
    dfplayer_volume(cfg.alarme_mp3_vol);
    dfplayer_loop(cfg.alarme_som);
  }

  estado_audio_gerenciado = estado_desejado;
}

void pararAudioAlarme() {
  if (estado_audio_gerenciado == AUDIO_GERENCIADO_ALARME_MP3) {
    dfplayer_stop();
  } else if (estado_audio_gerenciado == AUDIO_GERENCIADO_ALARME_RADIO) {
    rx.setAudioOutputHighImpedance(true);
  }

  digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
  state.radio_ligado = false;
  estado_audio_gerenciado = AUDIO_GERENCIADO_LIVRE;
}

void atualizarGerenciadorAudio() {
  if (estado_audio_gerenciado == AUDIO_GERENCIADO_CHIME &&
      (millis() - chime_inicio_ms) >= DURACAO_CHIME_MS) {
    finalizarChime();
  }
}
