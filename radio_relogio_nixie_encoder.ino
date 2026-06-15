#include <Wire.h>
#include <EEPROM.h>
#include <ErriezDS1307.h>
#include <JC_Button.h>
#include <RotaryEncoder.h>
#include <TEA5767.h>
#include "freqs.h"
ErriezDS1307 rtc;

TEA5767 rfm = TEA5767();

#define set_h_d 14  //~  14 A0
#define set_h_u 15  //~  15 A1
#define set_m_d 17  //~  17 A3
#define set_m_u 12
#define PIN_IN1 2
#define PIN_IN2 3
#define SW 4
#define vcc_amp A2
#define buzzer 13

Button myBtn(SW);

int8_t h_d;  //dezena da hora
int8_t h_u;  //unidade da hora
int8_t m_d;  //dezena dos minutos
int8_t m_u;  //unidade dos minutos


const uint8_t sends[] = { 5, 6, 7, 8 };
const uint8_t RGB[] = { 9, 10, 11 };

// Variáveis de controle
uint8_t ajuste;
uint8_t in_adjust;
uint8_t hora_relogio;
uint8_t minuto_relogio;
uint8_t f_step;
int pos;
bool radio_ligado;

// Frequência e posicionamento
int newPos;
uint8_t f_size;
uint8_t f_atual = 12;
const int SIGNAL_THRESHOLD = 5;

// Tempo e RGB
int rgb_time = 1500;

// Estrutura de tempo
uint8_t hour, min, sec, mday, mon, wday;
uint16_t year;

// Configuração de botões e estados

bool pressionadoLongo = false;
unsigned long falhas = 0;
unsigned long ms, msLast;
const unsigned long LONG_PRESS = 700;
const unsigned long MSLERTIME = 500;
unsigned long millisTarefa1;  // Inicializar depois no setup()

RotaryEncoder *encoder = nullptr;


void setup() {
  Serial.begin(9600);

  pinMode(buzzer, OUTPUT);
  pinMode(vcc_amp, OUTPUT);
  pinMode(RGB[0], OUTPUT);
  pinMode(RGB[1], OUTPUT);
  pinMode(RGB[2], OUTPUT);

  pinMode(sends[0], OUTPUT);
  pinMode(sends[1], OUTPUT);
  pinMode(sends[2], OUTPUT);
  pinMode(sends[3], OUTPUT);

  pinMode(set_h_d, OUTPUT);
  pinMode(set_h_u, OUTPUT);
  pinMode(set_m_d, OUTPUT);
  pinMode(set_m_u, OUTPUT);

  Wire.begin();
  Wire.setClock(100000);

  if (!rtc.begin()) {
    Serial.println(F("RTC not found"));  // Apenas avisa, sem travar
  }

  //    if (!rtc.setDateTime(12, 34, 56,  31, 12, 2020, 0)) {
  //          Serial.println(F("Set date/time failed"));
  //          ++falhas;
  //      }


  //rtc.setSquareWave(SquareWaveDisable);
  digitalWrite(vcc_amp, HIGH);  //amplificador desligado
  myBtn.begin();
  rfm.init(12);
  rfm.setStandby(1);
  rfm.setMono(1);
  //rfm.setStereoNC(true);
  rfm.setFrequency(freqs[f_atual]);


  encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);
  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);

  rtc.getTime(&hour, &min, &sec);

  f_size = getFreqsSize();
}

void loop() {

  //digitalWrite(vcc_amp, LOW);  //amplificador desligado

  // while(1){
  //    preee(3, 50);

  // };

  newPos = encoder->getPosition();

  ms = millis();
  myBtn.read();
  state();
  toca_musica_loop(); // Máquina de estados da música

  if ((ms - msLast) >= MSLERTIME) {
    //debug();
    if (!rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday)) {
      Serial.println(F("Read date/time failed"));
      ++falhas;
      //  return;
    }
    transicao();
    msLast = ms;
    // int sg =  rfm.getSignalLevel();
    //  Serial.println(sg);
  }
  
  if (radio_ligado) {
    radio();
  } else {

    if (ajuste == 1) {
      rgb_time = 100;
      rgb(18, 20, 0);
    } else if (ajuste == 2) {  // efeito ao sair do modo de ajuste
      rgb(18, 0, 18);
    } else {
      rgb_time = 1500;
      rgb(18, 20, 0);  //laranja
    }
    relogio();
  }

  atualiza(); // Chama a máquina de estados a cada ciclo do loop
}



unsigned long last_mux_time = 0;
uint8_t mux_state = 0; 

void atualiza() {
  unsigned long current_micros = micros();
  
  if (mux_state % 2 == 0) {
    // ESTADO: Tubo LIGADO
    if (current_micros - last_mux_time >= 3000) { // Fica 3ms aceso
      // Desliga anodos para iniciar tempo morto (blanking)
      digitalWrite(set_h_d, LOW);
      digitalWrite(set_h_u, LOW);
      digitalWrite(set_m_d, LOW);
      digitalWrite(set_m_u, LOW);
      
      mux_state++;
      last_mux_time = current_micros;
    }
  } else {
    // ESTADO: Tubo DESLIGADO (Tempo Morto / Blanking para eliminar os fantasmas)
    if (current_micros - last_mux_time >= 250) { // 250us apagado para os TILs desarmarem
      mux_state++;
      if (mux_state > 7) mux_state = 0;
      
      // Liga o próximo tubo
      uint8_t digit = mux_state / 2;
      
      if (digit == 0 && h_d > 0) {
        escreve_byte_fast(h_d);
        digitalWrite(set_h_d, HIGH);
      } else if (digit == 1 && h_u > -1) {
        escreve_byte_fast(h_u);
        digitalWrite(set_h_u, HIGH);
      } else if (digit == 2 && m_d > -1) {
        escreve_byte_fast(m_d);
        digitalWrite(set_m_d, HIGH);
      } else if (digit == 3 && m_u > -1) {
        escreve_byte_fast(m_u);
        digitalWrite(set_m_u, HIGH);
      }
      
      last_mux_time = current_micros;
    }
  }
}

void escreve_byte_fast(byte x) {
  if (x & 0b00000001) digitalWrite(sends[0], HIGH); else digitalWrite(sends[0], LOW);
  if (x & 0b00000010) digitalWrite(sends[1], HIGH); else digitalWrite(sends[1], LOW);
  if (x & 0b00000100) digitalWrite(sends[2], HIGH); else digitalWrite(sends[2], LOW);
  if (x & 0b00001000) digitalWrite(sends[3], HIGH); else digitalWrite(sends[3], LOW);
}


void rgb(int R, int G, int B) {

  // Verifica se já passou 200 milisegundos
  if ((millis() - millisTarefa1) < rgb_time) {
    analogWrite(RGB[0], R);
    analogWrite(RGB[1], G);
    analogWrite(RGB[2], B);
  } else {
    analogWrite(RGB[0], 0);
    analogWrite(RGB[1], 0);
    analogWrite(RGB[2], 0);
  }

  if ((millis() - millisTarefa1) > rgb_time * 2) {
    millisTarefa1 = millis();
  }
}



void debug() {
  Serial.print("Falhas: ");
  Serial.println(falhas);
  Serial.print("Ajuste: ");
  Serial.println(ajuste);
  Serial.print("f_step: ");
  Serial.println(f_step);
  Serial.println("----------------------------");
}
void checkPosition() {
  encoder->tick();
}
