#include <RDA5807.h> // It is a minimal receicer with two push buttons (ATmega328 - Uno, Nano etc)
RDA5807 rx; 
void setup() {
  pinMode(4, INPUT_PULLUP); // Arduino pin 4 - Seek station down
  pinMode(5, INPUT_PULLUP); // Arduino pin 5 - Seek station up
  rx.setup(); // Starts the receiver with default parameters
  rx.setBand(1);  // 0 = 87.5–108 MHz, 1 = 76–108 MHz, 2 = 76–90 MHz (Japão)
  rx.setFrequency(7750); // Tunes in 103.9 MHz  - Switch to your local favorite station
  rx.setVolume(15); // Define o volume no máximo
  
}
void loop() {
  if (digitalRead(4) == LOW) rx.seek(RDA_SEEK_WRAP,RDA_SEEK_DOWN);
  if (digitalRead(5) == LOW) rx.seek(RDA_SEEK_WRAP,RDA_SEEK_UP);
  delay(200);
}