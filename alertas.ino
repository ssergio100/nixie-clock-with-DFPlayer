void preee(int m, int t) {
  for (int v = 0; v <= m; v++) {
    rgb(24, 0, 26);
    tone(PINO_BUZZER, 100);
    delay(t);
    noTone(PINO_BUZZER);
    delay(t);
  }
}

void beep(int t) {
  tone(PINO_BUZZER, 1000, t);
}
