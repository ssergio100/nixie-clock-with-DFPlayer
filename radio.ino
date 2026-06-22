void radio() {
    ajustarFrequencia();
    atualizarDisplay();
}

void ajustarFrequencia() {
    if (alarme_tocando) return; // Ignora o encoder se o alarme estiver tocando

    if (encoder_pos_anterior != encoder_pos_atual && radio_ligado) {
        int dir = (encoder_pos_atual > encoder_pos_anterior) ? 1 : -1;
        encoder_pos_anterior = encoder_pos_atual;

        if (radio_controla_volume) {
            if (dir < 0) { // Girou para direita (Aumenta volume)
                if (volume_radio < 15) volume_radio++;
            } else { // Girou para esquerda (Diminui volume)
                if (volume_radio > 0) volume_radio--;
            }
            rx.setVolume(volume_radio);
            
            // Mute de verdade no volume zero para evitar vazamento analógico
            if (volume_radio == 0) {
                rx.setMute(true);
            } else {
                rx.setMute(false);
            }

            atualizarDisplay(); // Atualiza a exibição Nixie imediatamente
            atualiza_lcd();
            // Serial.print(F("Volume digital: "));
            // Serial.println(volume_radio);
        } else {
            // Lógica original de sintonia do rádio
            if (dir < 0) { // Girou para direita (Avança frequência)
                  if (radio_modo_sintonia == 1 || radio_modo_sintonia == 2) {
                      rx.setMute(true); // Silencia o rádio durante a busca
                      int steps = 0;
                       do {
                           freqs[radio_estacao_atual] += 0.2;
                           if (freqs[radio_estacao_atual] > 107.90) freqs[radio_estacao_atual] = 76.10;
                           rx.setFrequency((uint16_t)(freqs[radio_estacao_atual] * 100 + 0.5));
                           atualizarDisplay(); 
                           
                           delay(100); // Dá tempo do PLL travar e estabilizar o RSSI
                           steps++;
                       } while (rx.getRssi() < SIGNAL_THRESHOLD && steps < 170);
                      rx.setMute(false); // Restaura o áudio ao travar na estação
                  } else {
                      radio_estacao_atual = (radio_estacao_atual < radio_total_estacoes) ? radio_estacao_atual + 1 : 0;
                  }
            } else { // Girou para esquerda (Recua frequência)
                  if (radio_modo_sintonia == 1 || radio_modo_sintonia == 2) {
                      rx.setMute(true); // Silencia o rádio durante a busca
                      int steps = 0;
                       do {
                           freqs[radio_estacao_atual] -= 0.2;
                           if (freqs[radio_estacao_atual] < 76.10) freqs[radio_estacao_atual] = 107.90;
                           rx.setFrequency((uint16_t)(freqs[radio_estacao_atual] * 100 + 0.5));
                           atualizarDisplay(); 
                           
                           delay(100); // Dá tempo do PLL travar e estabilizar o RSSI
                           steps++;
                       } while (rx.getRssi() < SIGNAL_THRESHOLD && steps < 170);
                      rx.setMute(false); // Restaura o áudio ao travar na estação
                  } else {
                      radio_estacao_atual = (radio_estacao_atual > 0) ? radio_estacao_atual - 1 : radio_total_estacoes - 1;
                  }
            }
            int sg =  rx.getRssi();
            // Serial.println(sg);
            rx.setFrequency((uint16_t)(freqs[radio_estacao_atual] * 100 + 0.5));
            atualiza_lcd();
            // Serial.println("Freq.:");
            // Serial.println(freqs[radio_estacao_atual]);
        }
    }
}

void atualizarDisplay() {
    if (radio_controla_volume) {
        int sinal = rx.getRssi();
        nixie_hora_dez = (sinal < 10) ? -1 : (sinal / 10);
        nixie_hora_uni = sinal % 10;
        nixie_min_dez = (volume_radio < 10) ? 0 : (volume_radio / 10);
        nixie_min_uni = volume_radio % 10;
    } else if (radio_modo_sintonia == 2) {
        nixie_hora_dez = (hour % 100) / 10;
        nixie_hora_uni = hour % 10;
        nixie_min_dez = (min % 100) / 10;
        nixie_min_uni = min % 10;
    } else {
        char buf[10];
        dtostrf(freqs[radio_estacao_atual], 6, 1, buf);
        nixie_hora_dez = (buf[1] == '1') ? 1 : -1;
        nixie_hora_uni = buf[2] - '0';
        nixie_min_dez = buf[3] - '0';
        nixie_min_uni = buf[5] - '0';
    }
}
