void radio() {
    ajustarFrequencia();
    atualizarDisplay();
}

void ajustarFrequencia() {
    if (pos != newPos && radio_ligado == 1) {
        int dir = (newPos > pos) ? 1 : -1;
        pos = newPos;

        if (dir < 0) { // Girou para direita (Avança frequência)
              if (f_step == 1 || f_step == 2) {
                  rfm.setMuted(true); // Silencia o rádio durante a busca
                  int steps = 0;
                  do {
                      freqs[f_atual] += 0.1;
                      if (freqs[f_atual] > 107.90) freqs[f_atual] = 76.10;
                      rfm.setFrequency(freqs[f_atual]);
                      atualizarDisplay(); 
                      
                      unsigned long start = millis();
                      while (millis() - start < 50) {
                          atualiza();
                      }
                      steps++;
                  } while (rfm.getSignalLevel() < SIGNAL_THRESHOLD && steps < 320);
                  rfm.setMuted(false); // Restaura o áudio ao travar na estação
              } else {
                  f_atual = (f_atual < f_size) ? f_atual + 1 : 0;
              }
        } else { // Girou para esquerda (Recua frequência)
              if (f_step == 1 || f_step == 2) {
                  rfm.setMuted(true); // Silencia o rádio durante a busca
                  int steps = 0;
                  do {
                      freqs[f_atual] -= 0.1;
                      if (freqs[f_atual] < 76.10) freqs[f_atual] = 107.90;
                      rfm.setFrequency(freqs[f_atual]);
                      atualizarDisplay(); 
                      
                      unsigned long start = millis();
                      while (millis() - start < 50) {
                          atualiza();
                      }
                      steps++;
                  } while (rfm.getSignalLevel() < SIGNAL_THRESHOLD && steps < 320);
                  rfm.setMuted(false); // Restaura o áudio ao travar na estação
              } else {
                  f_atual = (f_atual > 0) ? f_atual - 1 : f_size - 1;
              }
        }
        int sg =  rfm.getSignalLevel();
         Serial.println(sg);
        // if(sg >=4){
        //     rfm.setMuted(0);
        // } else {
        //     rfm.setMuted(1);
        // }
        rfm.setFrequency(freqs[f_atual]);
        Serial.println("Freq.:");
        Serial.println(freqs[f_atual]);
    }
}

void atualizarDisplay() {

    if (f_step == 2) {
        rgb_time = 1500;
        rgb(18, 20, 0);
        h_d = (hour % 100) / 10;
        h_u = hour % 10;
        m_d = (min % 100) / 10;
        m_u = min % 10;

    } else {
        rgb(0, 0, 0);
        char buf[10];
        dtostrf(freqs[f_atual], 6, 1, buf);
        h_d = (buf[1] == '1') ? 1 : -1;
        h_u = buf[2] - '0';
        m_d = buf[3] - '0';
        m_u = buf[5] - '0';
    }
}