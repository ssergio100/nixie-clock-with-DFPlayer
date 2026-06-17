
void relogio() {

  // Lógica de Soneca (Override do Display)
  if (em_soneca) {
      long tempo_restante = (long)soneca_end_time - (long)millis();
      if (tempo_restante <= 0) {
          // Fim da soneca! O alarme volta a tocar.
          em_soneca = false;
          alarme_tocando = true;
          Serial.println(F(">>> SONECA ACABOU! ACORDANDO <<<"));
          if (alarme_radio) {
              radio_ligado = true;
              rx.setMute(false);
              digitalWrite(vcc_amp, LOW);
          } else {
              Serial.println(F("Tocando alarme MP3..."));
              digitalWrite(vcc_amp, LOW); // LIGA O AMPLIFICADOR!
              df_volume(25);
              df_loop(1);
          }
      } else {
          // Exibe a contagem regressiva (Minutos : Segundos)
          int minutos_restantes = (tempo_restante / 1000) / 60;
          int segundos_restantes = (tempo_restante / 1000) % 60;
          
          h_d = (minutos_restantes < 10) ? 0 : (minutos_restantes / 10);
          h_u = (minutos_restantes % 10);
          m_d = (segundos_restantes / 10);
          m_u = (segundos_restantes % 10);
          return; // Retorna para não exibir a hora real!
      }
  }

  // Lógica do Efeito Caça-Níqueis (Anti-Poisoning)
  // A cada 15 minutos (00, 15, 30, 45) exatamente no segundo 0, inicia o efeito.
  if (min % 15 == 0 && sec == 0 && !is_slot_machine && !alarme_tocando) {
      is_slot_machine = true;
      slot_machine_start = millis();
  }

  if (is_slot_machine) {
      // Duração de 3 segundos
      if (millis() - slot_machine_start > 3000) {
          is_slot_machine = false;
      } else {
          // Gira os números loucamente (muda a cada 50ms)
          h_d = (millis() / 50) % 10;
          h_u = ((millis() / 50) + 3) % 10;
          m_d = ((millis() / 50) + 6) % 10;
          m_u = ((millis() / 50) + 9) % 10;
          return; // Sai da função para não sobrescrever com a hora real
      }
  }

  // Se não estiver no modo caça-níqueis ou soneca, exibe a hora real
  // Desliga o zero à esquerda das horas para economizar válvula
  h_d = (hour < 10) ? -1 : (hour / 10);
  h_u = (hour % 10);
  m_d = (min / 10);
  m_u = (min % 10);
}
