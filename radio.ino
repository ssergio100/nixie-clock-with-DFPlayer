void radio() {
    ajustarFrequencia();
    atualizarDisplay();
}

void ajustarFrequencia() {
    if (state.alarme_tocando) return; // Ignora o encoder se o alarme estiver tocando

    if (state.encoder_pos_anterior != encoder_pos_atual && state.radio_ligado) {
        int dir = (encoder_pos_atual > state.encoder_pos_anterior) ? 1 : -1;
        state.encoder_pos_anterior = encoder_pos_atual;

        if (state.radio_modo_sintonia == 2) {
            // No modo Relógio do rádio, o encoder ajusta o brilho das Nixies
            if (dir > 0) { // Horário / Direita (Aumenta brilho)
                if (state.nixie_brilho_atual < 11) {
                    state.nixie_brilho_atual++;
                    beep(15);
                }
            } else { // Anti-horário / Esquerda (Diminui brilho)
                if (state.nixie_brilho_atual > 1) {
                    state.nixie_brilho_atual--;
                    beep(15);
                }
            }
        } else if (state.radio_controla_volume) {
            if (dir > 0) { // Girou para direita (Aumenta volume)
                if (state.volume_radio < 15) state.volume_radio++;
            } else { // Girou para esquerda (Diminui volume)
                if (state.volume_radio > 0) state.volume_radio--;
            }
            rx.setVolume(state.volume_radio);
            
            // Mute de verdade no volume zero para evitar vazamento analógico
            if (state.volume_radio == 0) {
                rx.setMute(true);
            } else {
                rx.setMute(false);
            }

            atualizarDisplay(); // Atualiza a exibição Nixie imediatamente
            atualiza_lcd();
            // Serial.print(F("Volume digital: "));
            // Serial.println(state.volume_radio);
        } else {
            // Lógica de sintonia direta do rádio
            if (dir > 0) { // Girou para direita (Avança frequência)
                  if (state.radio_modo_sintonia == 1 || state.radio_modo_sintonia == 2) {
                      rx.setMute(true); // Silencia o rádio durante a busca
                      int steps = 0;
                       do {
                           cfg.radio_frequencia += 20; // 0.2 MHz (20 * 10 kHz)
                           if (cfg.radio_frequencia > 10790) cfg.radio_frequencia = 7610;
                           rx.setFrequency(cfg.radio_frequencia);
                           atualizarDisplay(); 
                           
                           delay(100); // Dá tempo do PLL travar e estabilizar o RSSI
                           steps++;
                       } while (rx.getRssi() < SIGNAL_THRESHOLD && steps < 170);
                      rx.setMute(false); // Restaura o áudio ao travar na estação
                  } else {
                      cfg.radio_frequencia += 20;
                      if (cfg.radio_frequencia > 10790) cfg.radio_frequencia = 7610;
                      rx.setFrequency(cfg.radio_frequencia);
                  }
            } else { // Girou para esquerda (Recua frequência)
                  if (state.radio_modo_sintonia == 1 || state.radio_modo_sintonia == 2) {
                      rx.setMute(true); // Silencia o rádio durante a busca
                      int steps = 0;
                       do {
                           cfg.radio_frequencia -= 20; // 0.2 MHz (20 * 10 kHz)
                           if (cfg.radio_frequencia < 7610) cfg.radio_frequencia = 10790;
                           rx.setFrequency(cfg.radio_frequencia);
                           atualizarDisplay(); 
                           
                           delay(100); // Dá tempo do PLL travar e estabilizar o RSSI
                           steps++;
                       } while (rx.getRssi() < SIGNAL_THRESHOLD && steps < 170);
                      rx.setMute(false); // Restaura o áudio ao travar na estação
                  } else {
                      cfg.radio_frequencia -= 20;
                      if (cfg.radio_frequencia < 7610) cfg.radio_frequencia = 10790;
                      rx.setFrequency(cfg.radio_frequencia);
                  }
            }
            rx.setFrequency(cfg.radio_frequencia);
            atualiza_lcd();
        }
    }
}

void atualizarDisplay() {
    if (state.radio_controla_volume) {
        int sinal = rx.getRssi();
        nixie_hora_dez = (sinal < 10) ? -1 : (sinal / 10);
        nixie_hora_uni = sinal % 10;
        nixie_min_dez = (state.volume_radio < 10) ? 0 : (state.volume_radio / 10);
        nixie_min_uni = state.volume_radio % 10;
    } else if (state.radio_modo_sintonia == 2) {
        nixie_hora_dez = (hour % 100) / 10;
        nixie_hora_uni = hour % 10;
        nixie_min_dez = (min % 100) / 10;
        nixie_min_uni = min % 10;
    } else {
        nixie_hora_dez = (cfg.radio_frequencia >= 10000) ? 1 : -1;
        nixie_hora_uni = (cfg.radio_frequencia / 1000) % 10;
        nixie_min_dez = (cfg.radio_frequencia / 100) % 10;
        nixie_min_uni = (cfg.radio_frequencia / 10) % 10;
    }
}
