void verificarChime() {
  static bool executou = false;

  // O Chime toca no primeiro segundo da hora cheia (ex: 12:00:00)
  if (min == 0 && sec == 0) {
    if (executou) return;
    executou = true;
    iniciarChime();
  } else {
    executou = false;
  }
}
