// freqs2.cpp
#include "freqs2.h"

int freqs[] = {
  7670,   //Classic Pan
  8810,   //Gazeta
  8910,   //89 FM A Rádio Rock
  8970,   //Novabrasil
  9050,   //CBN
  9090,   //Bandeirantes
  9130,   //Disney
  9210,   //Play
  9250,   //Kiss /boa
  9290,   //Massa FM
  9330,   //Estilo
  9370,   //Rádio USP
  9470,   //Antena 1
  9530,   //Nativa
  9610,   //Band
  9690,   //BandNews
  9770,   //Energia
  9850,   //Metropolitana
  10010,  //Transamérica
  10090,  //Jovem Pan
  10170,  //Alfa Fm
  10250,  //Imprensa
  10330,  //Cultura
  10430,  //Dumont
  10470,  //Transcontinental
  10510,  //Rádio 105
  10570,  //Rádio Musical
  10630,  //Mix
  10730   //Eldorado
};

int getFreqsSize() {
  return (sizeof(freqs) / sizeof(freqs[0]))-1;
}