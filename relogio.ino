void relogio() {


  // Lógica de Soneca (Override do Display)
  if (state.em_soneca) {
      long tempo_restante = (long)state.soneca_end_time - (long)millis();
      if (tempo_restante <= 0) {
          // Fim da soneca! O alarme volta a tocar.
          state.em_soneca = false;
          state.alarme_tocando = true;
          state.alarme_start_time = millis(); // Reseta o tempo inicial do alarme ao acordar da soneca
          // Serial.println(F(">>> SONECA ACABOU! ACORDANDO <<<"));
          if (cfg.alarme_radio) {
              state.radio_ligado = true;
              rx.setAudioOutputHighImpedance(false); // Conecta o rádio à linha
              rx.setFrequency(cfg.radio_frequencia); // Sintoniza na frequência salva
              rx.setVolume(state.volume_radio); // Sincroniza o volume do rádio com o nível digital configurado
              if (state.volume_radio == 0) {
                  rx.setMute(true);
              } else {
                  rx.setMute(false);
              }
              digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
          } else {
              // Serial.println(F("Tocando alarme MP3..."));
              rx.setAudioOutputHighImpedance(true); // Garante o rádio desconectado
              digitalWrite(PINO_VCC_AMPLIFICADOR, LOW); // LIGA O AMPLIFICADOR!
              dfplayer_volume(cfg.alarme_mp3_vol);
              dfplayer_loop(cfg.alarme_som);
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

  // Exibição da Temperatura a cada 10 segundos (nos segundos terminados em 8 e 9, total de 2s)
  if ((sec % TEMP_EXIBIR_INTERVALO) >= (TEMP_EXIBIR_INTERVALO - TEMP_EXIBIR_DURACAO) && state.nixie_brilho_atual != -3 && !state.alarme_tocando && !state.is_slot_machine) {
      int temp = lerTemperatura();
      if (temp < 0) temp = 0;
      if (temp > 999) temp = 999;
      
      nixie_hora_dez = -1; // Apagado (X)
      nixie_hora_uni = (temp >= 100) ? (temp / 100) : -1; // Apaga o zero à esquerda se < 10.0°C (ex: 9.2°C -> "  92")
      nixie_min_dez = (temp / 10) % 10;
      nixie_min_uni = temp % 10;
      return; // Retorna para não exibir a hora real!
  }

  // Lógica do Efeito Caça-Níqueis (Anti-Poisoning)
  // A cada 15 minutos (00, 15, 30, 45) exatamente no segundo 0, inicia o efeito.
  static int last_slot_machine_minute = -1;
  if (min % 15 == 0 && sec == 0 && min != last_slot_machine_minute && !state.is_slot_machine && !state.alarme_tocando) {
      state.is_slot_machine = true;
      state.slot_machine_start = millis();
      last_slot_machine_minute = min;
  }

  if (min % 15 != 0) {
      last_slot_machine_minute = -1;
  }

  if (state.is_slot_machine) {
      unsigned long elapsed = millis() - state.slot_machine_start;
      if (elapsed >= 2000) {
          state.is_slot_machine = false;
      } else {
          // Dígito 1 (Dezena de Hora) - Embaralha por 1.0s, depois trava
          if (elapsed < 1000) {
              nixie_hora_dez = (millis() / 50) % 10;
          } else {
              nixie_hora_dez = (hour < 10) ? -1 : (hour / 10);
          }

          // Dígito 2 (Unidade de Hora) - Embaralha por 1.33s (1.0s + 1/3s), depois trava
          if (elapsed < 1333) {
              nixie_hora_uni = ((millis() / 50) + 3) % 10;
          } else {
              nixie_hora_uni = hour % 10;
          }

          // Dígito 3 (Dezena de Minuto) - Embaralha por 1.66s (1.33s + 1/3s), depois trava
          if (elapsed < 1666) {
              nixie_min_dez = ((millis() / 50) + 6) % 10;
          } else {
              nixie_min_dez = min / 10;
          }

          // Dígito 4 (Unidade de Minuto) - Embaralha por 2.0s (1.66s + 1/3s), depois trava
          if (elapsed < 2000) {
              nixie_min_uni = ((millis() / 50) + 9) % 10;
          } else {
              nixie_min_uni = min % 10;
          }
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

int lerTemperatura() {
  static unsigned long ultimaLeitura = 0;
  static int tempRetorno = 250; // valor padrão inicial seguro (25.0 °C)
  static float tempFiltrada = 0.0;
  static bool primeiraLeitura = true;
  
  // Só lê o sensor físico a cada 1000ms para evitar auto-aquecimento e desperdício de CPU
  if (millis() - ultimaLeitura >= 1000 || ultimaLeitura == 0) {
    ultimaLeitura = millis();
    
    long soma = 0;
    for (int i = 0; i < 20; i++) {
      soma += analogRead(PINO_TEMP);
      delayMicroseconds(150);
    }
    float mediaADC = soma / 20.0;
    float tempInst = (mediaADC * 110.0) / 1024.0;
    
    if (primeiraLeitura) {
      tempFiltrada = tempInst;
      primeiraLeitura = false;
    } else {
      tempFiltrada = (tempInst * 0.15) + (tempFiltrada * 0.85);
    }
    tempRetorno = (int)(tempFiltrada * 10.0 + 0.5); // Ex: 17.5 °C -> 175
  }
  return tempRetorno;
}
