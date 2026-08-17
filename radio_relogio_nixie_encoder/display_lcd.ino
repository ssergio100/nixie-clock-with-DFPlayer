uint8_t calcularDiaSemana(uint8_t d, uint8_t m, uint16_t y);
const char* obterNomeDiaSemana(uint8_t wd);

void atualiza_lcd() {
  if (state.estado_menu == MODO_RELOGIO) {
      return;
  }
  char linha1[17]; // 16 caracteres + terminador nulo
  char linha2[17];
  static char last_linha1[17] = "";
  static char last_linha2[17] = "";

  if (state.alarme_tocando) {
      if (cfg.alarme_radio) {
          strcpy_P(linha1, PSTR("Em alarme: Radio"));
          char freq_buf[6];
          snprintf_P(freq_buf, sizeof(freq_buf), PSTR("%3d.%d"), cfg.radio_frequencia / 100, (cfg.radio_frequencia / 10) % 10);
          snprintf_P(linha2, sizeof(linha2), PSTR("FM %s MHz"), freq_buf);
      } else {
          strcpy_P(linha1, PSTR("Em alarme: MP3"));
          strcpy_P(linha2, PSTR("Som: Alarme"));
      }
  } else if (state.em_soneca) {
      strcpy_P(linha1, PSTR("Em soneca"));
      long tempo_restante = (long)state.soneca_end_time - (long)millis();
      if (tempo_restante < 0) tempo_restante = 0;
      int minutos_restantes = (tempo_restante / 1000) / 60;
      int segundos_restantes = (tempo_restante / 1000) % 60;
      snprintf_P(linha2, sizeof(linha2), PSTR("Tempo: %02d:%02d"), minutos_restantes, segundos_restantes);
  } else if (state.radio_ligado) {
      // MODO RÁDIO: Exibe informações do Rádio
      char freq_buf[6];
      snprintf_P(freq_buf, sizeof(freq_buf), PSTR("%3d.%d"), cfg.radio_frequencia / 100, (cfg.radio_frequencia / 10) % 10); // Ex: "106.3" ou " 88.5"
      
      if (state.radio_controla_volume) {
          snprintf_P(linha1, sizeof(linha1), PSTR("FM %sMHz  VOL"), freq_buf);
          snprintf_P(linha2, sizeof(linha2), PSTR("Volume: %02d/15   "), state.volume_radio);
      } else if (state.radio_modo_sintonia == 2) {
          snprintf_P(linha1, sizeof(linha1), PSTR("FM %sMHz  CLK"), freq_buf);
          snprintf_P(linha2, sizeof(linha2), PSTR("Data: %02d/%02d/%04d"), mday, mon, year);
      } else {
          snprintf_P(linha1, sizeof(linha1), PSTR("FM %sMHz  SCA"), freq_buf);
          const char* stereo_str = rx.isStereo() ? "Stereo" : "Mono";
          snprintf_P(linha2, sizeof(linha2), PSTR("Sig:%-2d  %-8s"), rx.getRssi(), stereo_str);
      }
  } else if (state.estado_menu > MODO_RELOGIO) {
      // MODO SETUP: Menus de Configuração
      switch (state.estado_menu) {
          case SETUP_HORARIO:
              strcpy_P(linha1, PSTR("Horario : HH:MM "));
              snprintf_P(linha2, sizeof(linha2), PSTR("          %02d:%02d "), hour, min);
              break;
          case SETUP_ALARME: {
              strcpy_P(linha1, PSTR("Alarme Atv    Tp"));
              const char* atv_str = cfg.alarme_ligado ? (cfg.alarme_fds ? "Sim/FS" : " Sim  ") : " Nao  ";
              snprintf_P(linha2, sizeof(linha2), PSTR("%02d:%02d %-6s %-3s"), cfg.alarme_hora, cfg.alarme_min, atv_str, cfg.alarme_radio ? "Fm" : "mp3");
              break;
          }
          case SETUP_DATA: {
              strcpy_P(linha1, PSTR("Data/ano/semana "));
              uint8_t temp_wday = calcularDiaSemana(mday, mon, year);
              snprintf_P(linha2, sizeof(linha2), PSTR("%02d/%02d/%02d %-7s"), mday, mon, year % 100, obterNomeDiaSemana(temp_wday));
              break;
          }
          case SETUP_SONS:
              strcpy_P(linha1, PSTR("Alarme e Chime  "));
              snprintf_P(linha2, sizeof(linha2), PSTR("S%02d v%02d  S%02d v%02d"), cfg.alarme_som, cfg.alarme_mp3_vol, cfg.chime_som, cfg.chime_volume);
              break;
          default:
              strcpy_P(linha1, PSTR("Salvando...     "));
              strcpy_P(linha2, PSTR("                "));
              break;
      }
  } else {
      // MODO RELÓGIO/NORMAL: Linha 1 Status do Alarme
      if (cfg.alarme_ligado) {
          char tipo_som[4]; // "Rad" ou "MP3"
          if (cfg.alarme_radio) {
              strcpy_P(tipo_som, PSTR("Rad"));
          } else {
              strcpy_P(tipo_som, PSTR("MP3"));
          }

          char status_fds[4]; // "FDS" ou "   "
          if (cfg.alarme_fds) {
              strcpy_P(status_fds, PSTR("FDS"));
          } else {
              strcpy_P(status_fds, PSTR("   "));
          }

          // Formata: AL:06:00 Rad FDS
          snprintf_P(linha1, sizeof(linha1), PSTR("AL:%02d:%02d %s %s"), cfg.alarme_hora, cfg.alarme_min, tipo_som, status_fds);
      } else {
          // Se desligado, preenche com espaços para limpar o resto da linha
          strcpy_P(linha1, PSTR("AL: Desligado   "));
      }

      // MODO RELÓGIO/NORMAL: Linha 2 Data
      snprintf_P(linha2, sizeof(linha2), PSTR("Data: %02d/%02d/%04d"), mday, mon, year);
  }

  // Rotina de Auto-Limpeza (Padding)
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

  // Pisca o campo sob ajuste no LCD (substitui os caracteres por espaços na fase OFF do blink)
  if (state.estado_menu > MODO_RELOGIO) {
      bool blink_on = ((millis() / 300) % 2 == 0);
      if (!blink_on) {
          int blink_start = -1;
          int blink_len = 0;
          
          if (state.estado_menu == SETUP_HORARIO) {
              if (state.passo_setup == 0) { blink_start = 10; blink_len = 2; }
              else if (state.passo_setup == 1) { blink_start = 13; blink_len = 2; }
          } else if (state.estado_menu == SETUP_ALARME) {
              if (state.passo_setup == 0) { blink_start = 0; blink_len = 2; }
              else if (state.passo_setup == 1) { blink_start = 3; blink_len = 2; }
              else if (state.passo_setup == 2) { blink_start = 6; blink_len = 6; }
              else if (state.passo_setup == 3) { blink_start = 13; blink_len = 3; }
          } else if (state.estado_menu == SETUP_DATA) {
              if (state.passo_setup == 0) { blink_start = 0; blink_len = 2; }
              else if (state.passo_setup == 1) { blink_start = 3; blink_len = 2; }
              else if (state.passo_setup == 2) { blink_start = 6; blink_len = 2; }
          } else if (state.estado_menu == SETUP_SONS) {
              if (state.passo_setup == 0) { blink_start = 1; blink_len = 2; }
              else if (state.passo_setup == 1) { blink_start = 5; blink_len = 2; }
              else if (state.passo_setup == 2) { blink_start = 10; blink_len = 2; }
              else if (state.passo_setup == 3) { blink_start = 14; blink_len = 2; }
          }
          
          if (blink_start != -1) {
              for (int i = 0; i < blink_len; i++) {
                  linha2[blink_start + i] = ' ';
              }
          }
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
