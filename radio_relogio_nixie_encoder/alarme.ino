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
          iniciarAudioAlarme();
      }
  }
  // Reseta a trava no minuto seguinte
  if (min != cfg.alarme_min) {
      last_alarm_min = -1;
  }
}
