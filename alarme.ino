void verificarDisparoAlarme() {
  static int last_alarm_min = -1;
  
  if (cfg.alarme_ligado && hour == cfg.alarme_hora && min == cfg.alarme_min && last_alarm_min != min && !state.em_soneca) {
      bool toca_hoje = true;
      // DS1307: 1 = Domingo, 7 = Sábado. Se alarme_fds for false (só dias de semana), ignoramos dom(1) e sab(7).
      if (!cfg.alarme_fds && (wday == 1 || wday == 7)) {
          toca_hoje = false; 
      }
      if (toca_hoje) {
          last_alarm_min = min; // Trava para não tocar de novo no mesmo minuto
          state.alarme_tocando = true; // Flag de Soneca/Cancelamento
          state.alarme_start_time = millis(); // Salva o tempo inicial do disparo do alarme
          if (cfg.alarme_radio) {
              if (!state.radio_ligado) {
                  state.radio_ligado = true;
                  rx.setAudioOutputHighImpedance(false); // Conecta o rádio à linha
                  rx.setFrequency(cfg.radio_frequencia); // Sintoniza na frequência salva
                  state.volume_radio = 5; // Aplica volume 5 fixo para o alarme
                  rx.setVolume(state.volume_radio);
                  rx.setMute(false);
                  digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
              }
          } else {
              rx.setAudioOutputHighImpedance(true); // Desconecta o rádio da linha
              state.radio_ligado = false; // Garante a sincronia do estado
              
              digitalWrite(PINO_VCC_AMPLIFICADOR, LOW); // LIGA O AMPLIFICADOR!
              
              df_volume(cfg.alarme_mp3_vol); // Ajusta o volume primeiro
              
              delay(100); // Dá tempo do clone processar
              
              df_loop(cfg.alarme_som); // Agora toca e entra em repetição
          }
      }
  }
  // Reseta a trava no minuto seguinte
  if (min != cfg.alarme_min) {
      last_alarm_min = -1;
  }
}
