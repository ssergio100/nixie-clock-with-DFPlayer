uint8_t calcularDiaSemana(uint8_t d, uint8_t m, uint16_t y);
const char* obterNomeDiaSemana(uint8_t wd);

void atualiza_lcd() {
  if (estado_menu == MODO_RELOGIO) {
      return;
  }
  char linha1[17]; // 16 caracteres + terminador nulo
  char linha2[17];
  static char last_linha1[17] = "";
  static char last_linha2[17] = "";

  if (alarme_tocando) {
      if (alarme_radio) {
          strcpy(linha1, "Em alarme: Radio");
          char freq_buf[6];
          dtostrf(freqs[radio_estacao_atual], 5, 1, freq_buf);
          snprintf(linha2, sizeof(linha2), "FM %s MHz", freq_buf);
      } else {
          strcpy(linha1, "Em alarme: MP3");
          strcpy(linha2, "Som: Alarme");
      }
  } else if (em_soneca) {
      strcpy(linha1, "Em soneca");
      long tempo_restante = (long)soneca_end_time - (long)millis();
      if (tempo_restante < 0) tempo_restante = 0;
      int minutos_restantes = (tempo_restante / 1000) / 60;
      int segundos_restantes = (tempo_restante / 1000) % 60;
      snprintf(linha2, sizeof(linha2), "Tempo: %02d:%02d", minutos_restantes, segundos_restantes);
  } else if (radio_ligado) {
      // MODO RÁDIO: Exibe informações do Rádio
      char freq_buf[6];
      dtostrf(freqs[radio_estacao_atual], 5, 1, freq_buf); // Ex: "106.3" ou " 88.5"
      
      if (radio_controla_volume) {
          snprintf(linha1, sizeof(linha1), "FM %sMHz  VOL", freq_buf);
          snprintf(linha2, sizeof(linha2), "Volume: %02d/15   ", volume_radio);
      } else {
          snprintf(linha1, sizeof(linha1), "FM %sMHz  SCA", freq_buf);
          const char* stereo_str = rx.isStereo() ? "Stereo" : "Mono";
          snprintf(linha2, sizeof(linha2), "Sig:%-2d  %-8s", rx.getRssi(), stereo_str);
      }
  } else if (estado_menu > MODO_RELOGIO) {
      // MODO SETUP: Menus de Configuração
      switch (estado_menu) {
          case SETUP_HORA_MINUTO:
              strcpy(linha1, "Ajustar Relogio ");
              snprintf(linha2, sizeof(linha2), "%02d:%02d", hour, min);
              break;
          case SETUP_ALARME_HORA_MINUTO:
              strcpy(linha1, "Hora do Alarme  ");
              snprintf(linha2, sizeof(linha2), "%02d:%02d", alarme_hora, alarme_min);
              break;
          case SETUP_ALARME_MODO_STATUS:
              strcpy(linha1, "Modo do Alarme  ");
              if (alarme_radio) {
                  snprintf(linha2, sizeof(linha2), "RADIO (%s)", alarme_ligado ? "ON " : "OFF");
              } else {
                  snprintf(linha2, sizeof(linha2), "MP3 (%s)", alarme_ligado ? "ON " : "OFF");
              }
              break;
          case SETUP_DATA_DIA_MES: {
              strcpy(linha1, "Ajuste Data: D/M");
              uint8_t temp_wday = calcularDiaSemana(mday, mon, year);
              snprintf(linha2, sizeof(linha2), "%02d/%02d %s", mday, mon, obterNomeDiaSemana(temp_wday));
              break;
          }
          case SETUP_ANO: {
              snprintf(linha1, sizeof(linha1), "Ajuste Ano: %04d", year);
              uint8_t temp_wday = calcularDiaSemana(mday, mon, year);
              snprintf(linha2, sizeof(linha2), "Dia: %s", obterNomeDiaSemana(temp_wday));
              break;
          }
          case SETUP_ALARME_FDS:
              strcpy(linha1, "Alarme Fim de S.");
              if (alarme_fds) {
                  strcpy(linha2, "     LIGADO     ");
              } else {
                  strcpy(linha2, "   DESLIGADO    ");
              }
              break;
          case SETUP_ALARME_SOM_VOLUME:
              snprintf(linha1, sizeof(linha1), "Som Alarme: %02d  ", alarme_som);
              snprintf(linha2, sizeof(linha2), "Vol Alarme: %02d/30", alarme_mp3_vol);
              break;
          case SETUP_CHIME_SOM_VOLUME:
              snprintf(linha1, sizeof(linha1), "Som Hora: %02d    ", chime_som);
              snprintf(linha2, sizeof(linha2), "Vol Hora: %02d/30  ", chime_volume);
              break;
          default:
              strcpy(linha1, "Salvando...     ");
              strcpy(linha2, "                ");
              break;
      }
  } else {
      // MODO RELÓGIO/NORMAL: Linha 1 Status do Alarme
      if (alarme_ligado) {
          char tipo_som[4]; // "Rad" ou "MP3"
          if (alarme_radio) {
              strcpy(tipo_som, "Rad");
          } else {
              strcpy(tipo_som, "MP3");
          }

          char status_fds[4]; // "FDS" ou "   "
          if (alarme_fds) {
              strcpy(status_fds, "FDS");
          } else {
              strcpy(status_fds, "   ");
          }

          // Formata: AL:06:00 Rad FDS
          snprintf(linha1, sizeof(linha1), "AL:%02d:%02d %s %s", alarme_hora, alarme_min, tipo_som, status_fds);
      } else {
          // Se desligado, preenche com espaços para limpar o resto da linha
          strcpy(linha1, "AL: Desligado   ");
      }

      // MODO RELÓGIO/NORMAL: Linha 2 Data
      snprintf(linha2, sizeof(linha2), "Data: %02d/%02d/%04d", mday, mon, year);
  }

  // Rotina de Auto-Limpeza (Padding)
  // Substitui os terminadores nulos precoces por espaços até completar os 16 caracteres.
  // Isso garante que textos curtos sempre "apaguem" o lixo/fantasmas do texto anterior.
  for (int i = 0; i < 16; i++) {
      if (linha1[i] == '\0') {
          for (int j = i; j < 16; j++) linha1[j] = ' ';
          linha1[16] = '\0';
          break;
      }
  }
  for (int i = 0; i < 16; i++) {
      if (linha2[i] == '\0') {
          for (int j = i; j < 16; j++) linha2[j] = ' ';
          linha2[16] = '\0';
          break;
      }
  }

  // Só imprime no LCD se o texto mudou (Isso salva o I2C de travar a multiplexação da Nixie)
  if (strcmp(linha1, last_linha1) != 0) {
      for (int i = 0; i < 16; i++) {
          if (linha1[i] != last_linha1[i]) {
              lcd.setCursor(i, 0);
              lcd.print(linha1[i]);
              last_linha1[i] = linha1[i];
          }
      }
      last_linha1[16] = '\0';
  }
  
  if (strcmp(linha2, last_linha2) != 0) {
      for (int i = 0; i < 16; i++) {
          if (linha2[i] != last_linha2[i]) {
              lcd.setCursor(i, 1);
              lcd.print(linha2[i]);
              last_linha2[i] = linha2[i];
          }
      }
      last_linha2[16] = '\0';
  }
}
