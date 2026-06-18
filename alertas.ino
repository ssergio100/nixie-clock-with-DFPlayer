void preee(int m, int t) {
  for (int v = 0; v <= m; v++) {
    rgb(24, 0, 26);
    tone(PINO_BUZZER, 700);
    
    unsigned long start = millis();
    while (millis() - start < (unsigned long)t) {
      atualiza();
    }
    
    noTone(PINO_BUZZER);
    
    start = millis();
    while (millis() - start < (unsigned long)t) {
      atualiza();
    }
  }
}

void beep(int t) {
  tone(PINO_BUZZER, 300, t);
}
