
void salvar_e_sair() {
    estado_menu = 0;
    beep(100); delay(160); beep(100); // Bipe duplo de SAÍDA mais longo
    
    rtc.setDateTime(hour, min, sec, mday, mon, year, wday);
    
    // Debug no Console
    Serial.println(F("--- SALVANDO AJUSTES ---"));
    Serial.print(F("Hora: ")); Serial.print(hour); Serial.print(":"); Serial.println(min);
    Serial.print(F("Data: ")); Serial.print(mday); Serial.print("/"); Serial.print(mon); Serial.print("/"); Serial.println(year);
    Serial.print(F("Alarme: ")); Serial.print(alarme_hora); Serial.print(":"); Serial.println(alarme_min);
    Serial.print(F("Alarme Ligado: ")); Serial.println(alarme_ligado);
    Serial.print(F("Modo Radio: ")); Serial.println(alarme_radio);
    
    EEPROM.update(0, alarme_hora);
    EEPROM.update(1, alarme_min);
    EEPROM.update(2, alarme_ligado ? 1 : 0);
    EEPROM.update(3, alarme_radio ? 1 : 0);
    EEPROM.update(4, alarme_fds ? 1 : 0);
}

void state() {

  // Se o botão for mantido pressionado (Toque Longo)
  if (myBtn.pressedFor(LONG_PRESS) && !pressionadoLongo) {
      pressionadoLongo = true; // Trava para não repetir
      
      if (alarme_tocando || em_soneca) {
          // SONECA!
          Serial.println(F(">>> SONECA ATIVADA / REINICIADA <<<"));
          alarme_tocando = false;
          em_soneca = true;
          soneca_end_time = millis() + TEMPO_SONECA_MS; // Reseta a contagem
          beep(100); delay(160); beep(100);
          
          df_stop();
          rx.setAudioOutputHighImpedance(true);
          digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
          radio_ligado = false;
          atualiza_lcd();
      } else if (estado_menu > 0) {
          // MODO SETUP: Toque longo salva e sai imediatamente
          salvar_e_sair();
      } else {
          if (radio_ligado == 0) {
              Serial.println("Radio ON");
              rx.setAudioOutputHighImpedance(false); // Conecta o rádio à linha
              radio_controla_volume = false; // Inicia sempre no modo sintonia (SCA)
              rx.setVolume(volume_radio); // Define o volume digital configurado (nível 5 fixo)
              digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
              radio_ligado = 1;
          } else {
              Serial.print(F("Rádio OFF"));
              rx.setAudioOutputHighImpedance(true); // Desconecta o rádio da linha
              digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
              beep(70);
              radio_ligado = 0;
          }
          atualiza_lcd();
      }
  }

  // Quando o botão for solto
  if (myBtn.wasReleased()) {
      if (pressionadoLongo) {
          // Acabou de soltar após um toque longo. Apenas reseta a trava.
          pressionadoLongo = false;
      } else {
          // Foi um TOQUE CURTO genuíno
          if (alarme_tocando || em_soneca) {
              // ENCERRA O ALARME TOTALMENTE
              Serial.println(F(">>> ALARME CANCELADO <<<"));
              alarme_tocando = false;
              em_soneca = false;
              beep(500); // Confirma encerramento com bipe longo
              
              df_stop();
              rx.setAudioOutputHighImpedance(true);
              digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);
              radio_ligado = false;
              atualiza_lcd();
          } else {
              if (radio_ligado == 1) {
                  // No modo RÁDIO: Toque curto altera entre sintonia (SCA) e volume digital (VOL)
                  beep(70);
                  radio_controla_volume = !radio_controla_volume;
                  atualiza_lcd();
              } else {
                  // No modo RELÓGIO: Toque curto avança as páginas de Setup
                  if (estado_menu >= 6) {
                      // Estado 7: Sai do menu e salva tudo
                      salvar_e_sair();
                  } else {
                      if (estado_menu == 0) {
                          // Acabou de entrar no setup: puxa os dados do RTC pela última vez antes de congelar para edição
                          rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday);
                          beep(100); delay(160); beep(100); // Bipe duplo de ENTRADA mais longo
                      } else {
                          beep(30); // Bipe curto para avançar página
                      }
                      estado_menu++;
                  }
                  // Sincroniza o encoder para o número não pular quando você for girar
                  encoder_pos_anterior = encoder_pos_atual;
              }
          }
      }
  }
}
