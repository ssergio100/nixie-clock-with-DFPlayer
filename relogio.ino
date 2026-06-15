
void relogio() {

  //int newPos = encoder->getPosition();
  // Serial.println("Posicao:\n");
  // Serial.println(newPos);

  if (pos != newPos) {
    int dir = (newPos > pos) ? 1 : -1;
    pos = newPos;

    if (ajuste == 1) {  //hora
      if (in_adjust == 0) {
        Serial.println("Congelando horas");
        hora_relogio = hour;
        minuto_relogio = min;
        in_adjust = 1;
      }

      // Baseado no seu encoder: dir < 0 é Direita (minutos), dir > 0 é Esquerda (horas)
      if (dir < 0) {  
         minuto_relogio = (minuto_relogio + 1) % 60;
      } else {
        hora_relogio = (hora_relogio + 1) % 24;
      }
    }
  }

  if (ajuste == 2) {
      if (in_adjust == 1) {
        if (hora_relogio != hour or minuto_relogio != min) {
          Serial.println("Gravando horas");
          rtc.setDateTime(hora_relogio, minuto_relogio, sec, mday, mon, year, wday);
        }
      }
    preee(3, 50);
    in_adjust = 0;
    ajuste = 0;
  }

  int hora_display = in_adjust ? hora_relogio : hour;
  int minuto_display = in_adjust ? minuto_relogio : min;

  h_d = (hora_display / 10);
  h_u = (hora_display % 10);
  m_d = (minuto_display / 10);
  m_u = (minuto_display % 10);
}
