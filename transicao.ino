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
    // Horas pares tocam Parabéns, horas ímpares tocam a melodia normal.
    if (mday == 21 && mon == 9 && !in_adjust) {
      if (hour % 2 == 0) {
        inicia_parabens();
      } else {
        inicia_musica();
      }
    } else {
      if (!in_adjust) inicia_musica(); // Melodia normal (Minecraft) nos outros dias
    }
    
  } else {
    executou = false;
  }
}
