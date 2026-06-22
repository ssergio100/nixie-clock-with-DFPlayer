void verificarDisparoAlarme() {
  static int last_alarm_min = -1;
  
  if (alarme_ligado && hour == alarme_hora && min == alarme_min && last_alarm_min != min && !em_soneca) {
      bool toca_hoje = true;
      // DS1307: 1 = Domingo, 7 = Sábado. Se alarme_fds for false (só dias de semana), ignoramos dom(1) e sab(7).
      if (!alarme_fds && (wday == 1 || wday == 7)) {
          toca_hoje = false; 
      }
      if (toca_hoje) {
          last_alarm_min = min; // Trava para não tocar de novo no mesmo minuto
          alarme_tocando = true; // Flag de Soneca/Cancelamento
          alarme_start_time = millis(); // Salva o tempo inicial do disparo do alarme
          if (alarme_radio) {
              if (!radio_ligado) {
                  radio_ligado = true;
                  rx.setAudioOutputHighImpedance(false); // Conecta o rádio à linha
                  rx.setVolume(volume_radio); // Aplica volume digital configurado (nível 5 fixo)
                  if (volume_radio == 0) {
                      rx.setMute(true);
                  } else {
                      rx.setMute(false);
                  }
                  digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
              }
          } else {
              rx.setAudioOutputHighImpedance(true); // Desconecta o rádio da linha
              radio_ligado = false; // Garante a sincronia do estado
              digitalWrite(PINO_VCC_AMPLIFICADOR, LOW); // LIGA O AMPLIFICADOR!
              df_play(alarme_som); // Toca o alarme PRIMEIRO (clones ignoram volume se estiverem parados)
              delay(150); // Dá tempo do clone carregar o MP3 na memória
              df_volume(alarme_mp3_vol); // Sobe o volume agora que está tocando
          }
      }
  }
  // Reseta a trava no minuto seguinte
  if (min != alarme_min) {
      last_alarm_min = -1;
  }
}
