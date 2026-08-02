uint8_t calcularDiaSemana(uint8_t d, uint8_t m, uint16_t y);

void salvar_e_sair() {
    state.estado_menu = MODO_RELOGIO;
    state.passo_setup = 0;
    beepDuplo(); // Bipe duplo de SAÍDA
    
    // Parar qualquer preview de som e desligar o amplificador
    dfplayer_stop();
    digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
    
    mday = limitarDiaAoMes(mday, mon, year);
    wday = calcularDiaSemana(mday, mon, year);
    sec = 0; // Reseta segundos ao salvar horário editado
    gravarDataHoraRTC();
    
    salvarConfiguracao();
}

void tratarToqueCurto() {
    if (state.alarme_tocando || state.em_soneca) {
        // ENCERRA O ALARME TOTALMENTE
        state.alarme_tocando = false;
        state.em_soneca = false;
        beep(500); // Confirma encerramento com bipe longo
        
        pararAudioAlarme();
        salvarConfiguracao();
        atualiza_lcd();
    } else {
        if (state.radio_ligado) {
            // No modo RÁDIO: Toque curto altera o radio_modo_sintonia (Sintonia SCA -> Volume VOL -> Relógio CLK)
            beep(70);
            if (!state.radio_controla_volume && state.radio_modo_sintonia == 1) {
                // De SCA para VOL
                state.radio_controla_volume = true;
                state.radio_modo_sintonia = 1;
            } else if (state.radio_controla_volume && state.radio_modo_sintonia == 1) {
                // De VOL para CLK (Relógio)
                state.radio_controla_volume = false;
                state.radio_modo_sintonia = 2;
            } else {
                // De CLK para SCA
                state.radio_controla_volume = false;
                state.radio_modo_sintonia = 1;
            }
            atualiza_lcd();
        } else {
            // No modo SETUP / RELÓGIO
            if (state.estado_menu == MODO_RELOGIO) {
                cancelarChime();
                // Acabou de entrar no setup: puxa os dados do RTC pela última vez antes de congelar para edição
                lerDataHoraRTC();
                beepDuplo(); // Bipe duplo de ENTRADA
                state.estado_menu = SETUP_HORARIO;
                state.passo_setup = 0;
            } else {
                uint8_t total_passos = 0;
                if (state.estado_menu == SETUP_HORARIO) total_passos = 2;
                else if (state.estado_menu == SETUP_ALARME) total_passos = 4;
                else if (state.estado_menu == SETUP_DATA) total_passos = 3;
                else if (state.estado_menu == SETUP_SONS) total_passos = 4;

                if (state.passo_setup + 1 < total_passos) {
                    // Avança passo na tela atual
                    state.passo_setup++;
                    beep(30);

                    // Controle específico de áudio para preview de sons e volume
                    if (state.estado_menu == SETUP_SONS) {
                        if (state.passo_setup == 2) { // Entrou no Som Chime, para alarme e inicia chime
                            dfplayer_stop();
                            delay(50);
                            digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
                            dfplayer_volume(cfg.chime_volume);
                            delay(50);
                            dfplayer_play(cfg.chime_som);
                        }
                    }
                } else {
                    // Fim dos passos desta tela: avança de tela
                    if (state.estado_menu == SETUP_SONS) {
                        // Fim do setup
                        salvar_e_sair();
                    } else {
                        // Para o preview caso esteja saindo de alguma tela com som (embora no fluxo atual só a última tenha som)
                        dfplayer_stop();
                        digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);

                        state.estado_menu++;
                        state.passo_setup = 0;
                        beep(30);

                        // Se entrou na tela de sons, inicia o preview do som de alarme (passo 0)
                        if (state.estado_menu == SETUP_SONS) {
                            digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
                            dfplayer_volume(cfg.alarme_mp3_vol);
                            delay(50);
                            dfplayer_play(cfg.alarme_som);
                        }
                    }
                }
            }
            // Sincroniza o encoder para o número não pular quando você for girar
            state.encoder_pos_anterior = encoder_pos_atual;
        }
    }
}

void processarBotao() {
  // 1. Se estiver no período de bloqueio pós-despertar, consome os eventos
  if (millis() < state.bloqueio_clique_ate) {
      encoderButton.wasPressed();
      encoderButton.wasReleased();
      return;
  }

  // 2. Se o botão for mantido pressionado (Toque Longo)
  if (encoderButton.pressedFor(LONG_PRESS) && !state.pressionadoLongo) {
      state.pressionadoLongo = true; // Trava para não repetir
      
      if (state.alarme_tocando || state.em_soneca) {
          // SONECA!
          state.alarme_tocando = false;
          state.em_soneca = true;
          state.soneca_end_time = millis() + TEMPO_SONECA_MS; // Reseta a contagem
          beepDuplo();
          
          pararAudioAlarme();
          atualiza_lcd();
      } else if (state.estado_menu > MODO_RELOGIO) {
          // MODO SETUP: Toque longo salva e sai imediatamente
          salvar_e_sair();
      } else {
          cancelarChime();
          cancelarBuscaRadio();
          if (!state.radio_ligado) {
              rx.setAudioOutputHighImpedance(false); // Conecta o rádio à linha
              state.radio_controla_volume = false; // Inicia sempre no modo sintonia (SCA)
              state.radio_modo_sintonia = 1; // Inicia no modo sintonia (SCA)
              rx.setVolume(state.volume_radio); // Define o volume digital configurado
              if (state.volume_radio == 0) {
                  rx.setMute(true);
              } else {
                  rx.setMute(false);
              }
              digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
              state.radio_ligado = true;
          } else {
              rx.setAudioOutputHighImpedance(true); // Desconecta o rádio da linha
              digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
              beep(70);
              state.radio_ligado = false;
              salvarConfiguracao();
          }
          atualiza_lcd();
      }
  }

  // 3. Detecção de Clique Simples (sem delay para toque duplo)
  if (encoderButton.wasReleased()) {
      if (state.pressionadoLongo) {
          // Acabou de soltar após um toque longo. Apenas reseta a trava.
          state.pressionadoLongo = false;
      } else {
          tratarToqueCurto();
      }
  }
}

void checkPosition() {
  encoder->tick();
}
