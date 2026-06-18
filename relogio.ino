
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
              rx.setAudioOutputHighImpedance(false); // Conecta o rádio à linha
              rx.setVolume(volume_radio); // Sincroniza o volume do rádio com o nível digital configurado
              if (volume_radio == 0) {
                  rx.setMute(true);
              } else {
                  rx.setMute(false);
              }
              digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
          } else {
              Serial.println(F("Tocando alarme MP3..."));
              rx.setAudioOutputHighImpedance(true); // Garante o rádio desconectado
              digitalWrite(PINO_VCC_AMPLIFICADOR, LOW); // LIGA O AMPLIFICADOR!
              df_volume(25);
              df_loop(1);
          }
      } else {
          // Exibe a contagem regressiva (Minutos : Segundos)
          int minutos_restantes = (tempo_restante / 1000) / 60;
          int segundos_restantes = (tempo_restante / 1000) % 60;
          
          nixie_hora_dez = (minutos_restantes < 10) ? 0 : (minutos_restantes / 10);
          nixie_hora_uni = (minutos_restantes % 10);
          nixie_min_dez = (segundos_restantes / 10);
          nixie_min_uni = (segundos_restantes % 10);
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
          nixie_hora_dez = (millis() / 50) % 10;
          nixie_hora_uni = ((millis() / 50) + 3) % 10;
          nixie_min_dez = ((millis() / 50) + 6) % 10;
          nixie_min_uni = ((millis() / 50) + 9) % 10;
          return; // Sai da função para não sobrescrever com a hora real
      }
  }

  // Se não estiver no modo caça-níqueis ou soneca, exibe a hora real
  // Desliga o zero à esquerda das horas para economizar válvula
  nixie_hora_dez = (hour < 10) ? -1 : (hour / 10);
  nixie_hora_uni = (hour % 10);
  nixie_min_dez = (min / 10);
  nixie_min_uni = (min % 10);
}
