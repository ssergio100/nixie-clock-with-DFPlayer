void transicao() {
  static bool executou = false;

  if (min == 59 && sec == 59) {
    
    if (executou) return;
    executou = true;
    
    while (min > 0) {

      min--;

      m_d = (min % 100) / 10;
      m_u = min % 10;
      if (min == 1 ) {
        hour++;
      }
      h_d = (hour % 100) / 10;
      h_u = hour % 10;

      unsigned long start = millis();
      while (millis() - start < 4) {
        atualiza();
      }
    }
    
    // No aniversário do Arthur (21 de setembro), alterna a música a cada hora:
    // Horas pares tocam a faixa 2 (Parabéns MP3), horas ímpares tocam a faixa 3 (Som especial MP3).
    if (mday == 21 && mon == 9 && ajuste == 0) {
      df_volume(20);
      if (hour % 2 == 0) {
        df_play(2);
      } else {
        df_play(3);
      }
    } else {
      if (ajuste == 0) {
        tone(buzzer, 1500, 100); // Bip discreto a cada hora cheia no buzzer físico
      }
    }
    
  } else {
    executou = false;
  }
}
