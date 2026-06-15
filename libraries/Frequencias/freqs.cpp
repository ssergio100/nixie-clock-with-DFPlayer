// freqs.cpp
#include "freqs.h"

float freqs[] = {
  76.7,   //Classic Pan
  88.1,   //Gazeta
  89.1,   //89 FM A Rádio Rock
  89.7,   //Novabrasil
  90.5,   //CBN
  90.9,   //Bandeirantes
  91.3,   //Disney
  92.1,   //Play
  92.5,   //Kiss /boa
  92.9,   //Massa FM
  93.3,   //Estilo
  93.7,   //Rádio USP
  94.7,   //Antena 1
  95.3,   //Nativa
  96.1,   //Band
  96.9,   //BandNews
  97.7,   //Energia
  98.5,   //Metropolitana
  100.1,  //Transamérica
  100.9,  //Jovem Pan
  101.7,  //Alfa Fm
  102.5,  //Imprensa
  103.3,  //Cultura
  104.3,  //Dumont
  104.7,  //Transcontinental
  105.1,  //Rádio 105
  105.7,  //Rádio Musical
  106.3,  //Mix
  107.3   //Eldorado
};

int getFreqsSize() {
  return (sizeof(freqs) / sizeof(freqs[0]))-1;
}