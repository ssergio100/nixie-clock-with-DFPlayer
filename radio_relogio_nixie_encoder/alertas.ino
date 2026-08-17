const unsigned long INTERVALO_INICIO_BIPE_DUPLO_MS = 160;
const uint16_t DURACAO_BIPE_DUPLO_MS = 100;

bool segundo_bipe_pendente = false;
unsigned long inicio_bipe_duplo_ms = 0;

void beep(int t) {
  segundo_bipe_pendente = false;
  tone(PINO_BUZZER, 1000, t);
}

void beepDuplo() {
  tone(PINO_BUZZER, 1000, DURACAO_BIPE_DUPLO_MS);
  inicio_bipe_duplo_ms = millis();
  segundo_bipe_pendente = true;
}

void atualizarAlertas() {
  if (segundo_bipe_pendente &&
      (millis() - inicio_bipe_duplo_ms) >= INTERVALO_INICIO_BIPE_DUPLO_MS) {
    segundo_bipe_pendente = false;
    tone(PINO_BUZZER, 1000, DURACAO_BIPE_DUPLO_MS);
  }
}
