void transicao() {
  static bool executou = false;

  if (min == 59 && sec == 59) {
    
    if (executou) return;
    executou = true;
    
    while (min > 0) {

      min--;

      nixie_min_dez = (min % 100) / 10;
      nixie_min_uni = min % 10;
      if (min == 1 ) {
        hour++;
      }
      nixie_hora_dez = (hour % 100) / 10;
      nixie_hora_uni = hour % 10;

      unsigned long start = millis();
      while (millis() - start < 4) {
        atualiza();
      }
    }
    
    if (estado_menu == 0) {
      digitalWrite(PINO_VCC_AMPLIFICADOR, LOW); // LIGA O AMPLIFICADOR
      df_play(ALDEAO); 
      tone(PINO_BUZZER, 1500, 100); // Bip discreto a cada hora cheia no PINO_BUZZER físico
      
      // Aguarda 2 segundos para o som tocar, mantendo a multiplexação das Nixies ativas
      unsigned long start_chime = millis();
      while (millis() - start_chime < 2000) {
        atualiza();
      }
      
      digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH); // DESLIGA O AMPLIFICADOR para evitar chiado
    }
    
  } else {
    executou = false;
  }
}
