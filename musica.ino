#include "pitches.h"

// Melodia "Sweden" (Minecraft - C418) - Introdução
const int melodia_virada[] PROGMEM = {
    NOTE_FS4, NOTE_A4, NOTE_CS5, NOTE_D5,
    NOTE_FS4, NOTE_A4, NOTE_CS5, NOTE_E5, NOTE_CS5, NOTE_A4
};

// Batidas (durações) - números maiores tocam por mais tempo
const int duracao_virada[] PROGMEM = {
    4, 4, 4, 16,
    4, 4, 4, 8, 4, 16
};

// Melodia Parabéns
const int melodia_parabens[] PROGMEM = {
    392, 392, 440, 392, 523, 494, 392, 392, 440, 392, 587, 523,
    392, 392, 784, 659, 523, 494, 440, 698, 698, 659, 523, 587, 523
};

const int duracao_parabens[] PROGMEM = {
    2, 2, 4, 4, 4, 8, 2, 2, 4, 4, 4, 8,
    2, 2, 4, 4, 4, 4, 8, 2, 2, 4, 4, 4, 8
};

// Variáveis de controle da máquina de estados
const int* melodia_atual = NULL;
const int* duracoes_atual = NULL;
int total_notas = 0;
int nota_atual = 0;
int andamento_atual = 150;
unsigned long tempo_proxima_nota = 0;
bool tocando = false;

void inicia_musica() {
  melodia_atual = melodia_virada;
  duracoes_atual = duracao_virada;
  total_notas = sizeof(melodia_virada) / sizeof(melodia_virada[0]);
  nota_atual = 0;
  andamento_atual = 150; // Tempo mais calmo
  tocando = true;
  tempo_proxima_nota = millis(); // Inicia imediatamente
}

void inicia_parabens() {
  melodia_atual = melodia_parabens;
  duracoes_atual = duracao_parabens;
  total_notas = sizeof(melodia_parabens) / sizeof(melodia_parabens[0]);
  nota_atual = 0;
  andamento_atual = 100; // Tempo original do parabéns
  tocando = true;
  tempo_proxima_nota = millis();
}

void toca_musica_loop() {
  if (!tocando) return;

  if (millis() >= tempo_proxima_nota) {
    if (nota_atual >= total_notas) {
      // Fim da música
      noTone(buzzer);
      tocando = false;
      return;
    }

    // Calcula a duração da nota conforme a lógica: beats[i] * tempo
    int duracao_ms = pgm_read_word(&duracoes_atual[nota_atual]) * andamento_atual; 
    
    // A pausa entre as notas no seu exemplo era de: tempo / 2
    int pausa_ms = andamento_atual / 2;
    
    // Toca a nota
    tone(buzzer, pgm_read_word(&melodia_atual[nota_atual]), duracao_ms);
    
    // Define a hora que a próxima nota deve tocar (duração da nota + a pausa)
    tempo_proxima_nota = millis() + duracao_ms + pausa_ms;
    
    nota_atual++;
  }
}
