uint8_t calcularDiaSemana(uint8_t d, uint8_t m, uint16_t y);

void salvar_e_sair() {
    estado_menu = MODO_RELOGIO;
    passo_setup = 0;
    beep(100); delay(160); beep(100); // Bipe duplo de SAÍDA mais longo
    
    // Parar qualquer preview de som e desligar o amplificador
    df_stop();
    digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
    
    wday = calcularDiaSemana(mday, mon, year);
    rtc.setDateTime(hour, min, sec, mday, mon, year, wday);
    
    EEPROM.update(EEPROM_ADDR_ALARME_HORA, alarme_hora);
    EEPROM.update(EEPROM_ADDR_ALARME_MIN, alarme_min);
    EEPROM.update(EEPROM_ADDR_ALARME_LIGADO, alarme_ligado ? 1 : 0);
    EEPROM.update(EEPROM_ADDR_ALARME_RADIO, alarme_radio ? 1 : 0);
    EEPROM.update(EEPROM_ADDR_ALARME_FDS, alarme_fds ? 1 : 0);
    EEPROM.update(EEPROM_ADDR_ALARME_SOM, alarme_som);
    EEPROM.update(EEPROM_ADDR_ALARME_MP3_VOL, alarme_mp3_vol);
    EEPROM.update(EEPROM_ADDR_CHIME_SOM, chime_som);
    EEPROM.update(EEPROM_ADDR_CHIME_VOLUME, chime_volume);
}

void tratarToqueCurto() {
    if (alarme_tocando || em_soneca) {
        // ENCERRA O ALARME TOTALMENTE
        alarme_tocando = false;
        em_soneca = false;
        beep(500); // Confirma encerramento com bipe longo
        
        df_stop();
        rx.setAudioOutputHighImpedance(true);
        digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
        radio_ligado = false;
        atualiza_lcd();
    } else {
        if (radio_ligado) {
            // No modo RÁDIO: Toque curto altera entre sintonia (SCA) e volume digital (VOL)
            beep(70);
            radio_controla_volume = !radio_controla_volume;
            atualiza_lcd();
        } else {
            // No modo SETUP / RELÓGIO
            if (estado_menu == MODO_RELOGIO) {
                // Acabou de entrar no setup: puxa os dados do RTC pela última vez antes de congelar para edição
                rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday);
                beep(100); delay(160); beep(100); // Bipe duplo de ENTRADA mais longo
                estado_menu = SETUP_HORARIO;
                passo_setup = 0;
            } else {
                uint8_t total_passos = 0;
                if (estado_menu == SETUP_HORARIO) total_passos = 2;
                else if (estado_menu == SETUP_ALARME) total_passos = 4;
                else if (estado_menu == SETUP_DATA) total_passos = 3;
                else if (estado_menu == SETUP_SONS) total_passos = 4;

                if (passo_setup + 1 < total_passos) {
                    // Avança passo na tela atual
                    passo_setup++;
                    beep(30);

                    // Controle específico de áudio para preview de sons e volume
                    if (estado_menu == SETUP_SONS) {
                        if (passo_setup == 2) { // Entrou no Som Chime, para alarme e inicia chime
                            df_stop();
                            delay(50);
                            digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
                            df_volume(chime_volume);
                            delay(50);
                            df_play(chime_som);
                        }
                    }
                } else {
                    // Fim dos passos desta tela: avança de tela
                    if (estado_menu == SETUP_SONS) {
                        // Fim do setup
                        salvar_e_sair();
                    } else {
                        // Para o preview caso esteja saindo de alguma tela com som (embora no fluxo atual só a última tenha som)
                        df_stop();
                        digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);

                        estado_menu++;
                        passo_setup = 0;
                        beep(30);

                        // Se entrou na tela de sons, inicia o preview do som de alarme (passo 0)
                        if (estado_menu == SETUP_SONS) {
                            digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
                            df_volume(alarme_mp3_vol);
                            delay(50);
                            df_play(alarme_som);
                        }
                    }
                }
            }
            // Sincroniza o encoder para o número não pular quando você for girar
            encoder_pos_anterior = encoder_pos_atual;
        }
    }
}

void toggleScreenSaver() {
    // Só entra/sai do screen saver se não estivermos no setup, com rádio ligado, ou tocando alarme
    if (estado_menu > MODO_RELOGIO || radio_ligado || alarme_tocando || em_soneca) {
        return;
    }
    screen_saver_ativo = !screen_saver_ativo;
    beep(70); delay(80); beep(70); // bipe duplo indicando transição
    // Serial.print(F("Screen Saver Manual: "));
    // Serial.println(screen_saver_ativo ? F("ATIVADO") : F("DESATIVADO"));
}

void state() {
  // 1. Se o Screen Saver estiver ativo, intercepta o botão para acordar
  if (screen_saver_ativo) {
      static bool botao_acordou_tela = false;
      if (myBtn.wasPressed()) {
          botao_acordou_tela = true;
          screen_saver_ativo = false;
          bloqueio_clique_ate = millis() + 600; // Bloqueia cliques por 600ms
          beep(50);
      }
      if (botao_acordou_tela) {
          if (myBtn.wasReleased()) {
              botao_acordou_tela = false;
          }
          return; // Consome o evento e acorda o relógio
      }
  }

  // 2. Se estiver no período de bloqueio pós-despertar, consome os eventos
  if (millis() < bloqueio_clique_ate) {
      myBtn.wasPressed();
      myBtn.wasReleased();
      return;
  }

  // 3. Se o botão for mantido pressionado (Toque Longo)
  if (myBtn.pressedFor(LONG_PRESS) && !pressionadoLongo) {
      pressionadoLongo = true; // Trava para não repetir
      
      if (alarme_tocando || em_soneca) {
          // SONECA!
          // Serial.println(F(">>> SONECA ATIVADA / REINICIADA <<<"));
          alarme_tocando = false;
          em_soneca = true;
          soneca_end_time = millis() + TEMPO_SONECA_MS; // Reseta a contagem
          beep(100); delay(160); beep(100);
          
          df_stop();
          rx.setAudioOutputHighImpedance(true);
          digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
          radio_ligado = false;
          atualiza_lcd();
      } else if (estado_menu > MODO_RELOGIO) {
          // MODO SETUP: Toque longo salva e sai imediatamente
          salvar_e_sair();
      } else {
          if (!radio_ligado) {
              // Serial.println("Radio ON");
              rx.setAudioOutputHighImpedance(false); // Conecta o rádio à linha
              radio_controla_volume = false; // Inicia sempre no modo sintonia (SCA)
              rx.setVolume(volume_radio); // Define o volume digital configurado (nível 5 fixo)
              if (volume_radio == 0) {
                  rx.setMute(true);
              } else {
                  rx.setMute(false);
              }
              digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
              radio_ligado = true;
          } else {
              // Serial.print(F("Rádio OFF"));
              rx.setAudioOutputHighImpedance(true); // Desconecta o rádio da linha
              digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
              beep(70);
              radio_ligado = false;
          }
          atualiza_lcd();
      }
  }

  // 4. Detecção de Cliques (Toque Curto vs Toque Duplo)
  static unsigned long last_release_time = 0;
  static bool pending_click = false;

  if (myBtn.wasReleased()) {
      if (pressionadoLongo) {
          // Acabou de soltar após um toque longo. Apenas reseta a trava.
          pressionadoLongo = false;
      } else {
          unsigned long current_time = millis();
          if (pending_click && (current_time - last_release_time < 300)) {
              // Foi um Toque Duplo!
              pending_click = false;
              toggleScreenSaver();
          } else {
              pending_click = true;
              last_release_time = current_time;
          }
      }
  }

  // Se passou o tempo limite (300ms) e temos um clique pendente, executa o toque curto
  if (pending_click && (millis() - last_release_time >= 300)) {
      pending_click = false;
      tratarToqueCurto();
  }
}

void checkPosition() {
  encoder->tick();
}
