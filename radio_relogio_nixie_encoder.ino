#include <Wire.h>
#include <EEPROM.h>
#include <ErriezDS1307.h>
#include <JC_Button.h>
#include <RotaryEncoder.h>
#include <RDA5807.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "freqs.h"

#include <DFRobotDFPlayerMini.h>

ErriezDS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial dfSerial(11, 10); // RX (Pino 11), TX (Pino 10) - Corrigido cruzamento
DFRobotDFPlayerMini myDFPlayer;

RDA5807 rx;

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
#define LED_BASE 9

// Variáveis de controle e Alarme
uint8_t ajuste = 0; // 0 = Relógio Normal, 1-5 = Menus, 6 = Salvar/Sair
uint8_t f_step = 0;
int pos = 0;
bool radio_ligado = false;

uint8_t alarme_hora = 6;
uint8_t alarme_min = 0;
bool alarme_ligado = false;
bool alarme_radio = false; // 0 = Buzzer, 1 = Rádio
bool alarme_fds = false;

// Estado do Alarme
bool alarme_tocando = false;
bool em_soneca = false;
unsigned long soneca_end_time = 0;
const unsigned long TEMPO_SONECA_MS = 300000; // 5 minutos (5 * 60 * 1000)

// Caça-Níqueis
bool is_slot_machine = false;
unsigned long slot_machine_start = 0;

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
const unsigned long MSLERTIME = 1000;
unsigned long millisTarefa1;  // Inicializar depois no setup()

RotaryEncoder *encoder = nullptr;


void setup() {
  Serial.begin(9600);

  pinMode(buzzer, OUTPUT);
  pinMode(vcc_amp, OUTPUT);
  pinMode(LED_BASE, OUTPUT);

  pinMode(sends[0], OUTPUT);
  pinMode(sends[1], OUTPUT);
  pinMode(sends[2], OUTPUT);
  pinMode(sends[3], OUTPUT);

  pinMode(set_h_d, OUTPUT);
  pinMode(set_h_u, OUTPUT);
  pinMode(set_m_d, OUTPUT);
  pinMode(set_m_u, OUTPUT);

  Wire.begin();

  if (!rtc.begin()) {
    Serial.println(F("RTC not found"));  // Apenas avisa, sem travar
  }

  //    if (!rtc.setDateTime(12, 34, 56,  31, 12, 2020, 0)) {
  //          Serial.println(F("Set date/time failed"));
  //          ++falhas;
  //      }


  rtc.setSquareWave(SquareWave1Hz); // Ativa o pulso de 1Hz no pino SQW para piscar o LED
  digitalWrite(vcc_amp, HIGH);  //amplificador desligado
  myBtn.begin();
  dfSerial.begin(9600); // Inicia a comunicação com o DFPlayer Mini
  
  // Aguarda boot elétrico do DFPlayer
  Serial.println(F("Aguardando 2s para boot do DFPlayer..."));
  delay(2000);

  // Inicializa a biblioteca DFRobotDFPlayerMini
  if (!myDFPlayer.begin(dfSerial, true, false)) {
    Serial.println(F("Falha ao inicializar DFPlayer pela biblioteca."));
  } else {
    Serial.println(F("DFPlayer Mini Online pela biblioteca."));
  }
  
  // Lê volume inicial do potenciômetro e define silencioso
  int pot_val = analogRead(A6);
  int vol_mp3 = map(pot_val, 0, 1023, 0, 30);
  myDFPlayer.volume(vol_mp3);
  delay(100);
  myDFPlayer.stop();

  rx.setup();
  rx.setVolume(5); // Volume fixo em 5 (1/3 de 15) conforme solicitado para testes
  rx.setSoftmute(false); // Desativa o mute gradual para agilizar a sintonização
  rx.setMute(false);
  rx.setMono(true); // Mono garante sinal mais limpo em transmissões fracas
  rx.setFrequency((uint16_t)(freqs[f_atual] * 100)); // PU2CLR usa uint16_t (ex: 10630)


  encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);
  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);

  rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday);

  // Iniciar LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Carregar Alarmes
  alarme_hora = EEPROM.read(0);
  if (alarme_hora > 23) alarme_hora = 6;
  alarme_min = EEPROM.read(1);
  if (alarme_min > 59) alarme_min = 0;
  alarme_ligado = EEPROM.read(2) == 1;
  alarme_radio = EEPROM.read(3) == 1;
  alarme_fds = EEPROM.read(4) == 1;

  f_size = getFreqsSize();

  // O DS1307 só suporta 100kHz! Acelerar para 400kHz pode corromper as leituras ou travar o chip.
  // Como resolvemos o pisca-pisca das válvulas com o "cache do LCD", não precisamos mais do I2C em overclock.
  // Wire.setClock(400000);
}

void loop() {
  newPos = encoder->getPosition();

  ms = millis();
  myBtn.read();
  state();

  if ((ms - msLast) >= MSLERTIME) {
    if (ajuste == 0) { // Só puxa a hora do chip físico se não estivermos no menu
      if (!rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday)) {
        Serial.println(F("Read date/time failed"));
        ++falhas;
      } else {
        // --- LÓGICA DE DISPARO DO ALARME ---
        static int last_alarm_min = -1;
        if (alarme_ligado && hour == alarme_hora && min == alarme_min && last_alarm_min != min && !em_soneca) {
            bool toca_hoje = true;
            // DS1307: 1 = Domingo, 7 = Sábado. Se alarme_fds for false (só dias de semana), ignoramos dom(1) e sab(7).
            if (!alarme_fds && (wday == 1 || wday == 7)) {
                toca_hoje = false; 
            }
            if (toca_hoje) {
                last_alarm_min = min; // Trava para não tocar de novo no mesmo minuto
                alarme_tocando = true; // Flag de Soneca/Cancelamento
                Serial.println(F(">>> ALARME DISPARADO! <<<"));
                if (alarme_radio) {
                    if (!radio_ligado) {
                        Serial.println(F("Ligando rádio para o alarme..."));
                        radio_ligado = true;
                        rx.setMute(false); // Liga o áudio do rádio
                        digitalWrite(vcc_amp, LOW);
                    }
                } else {
                    Serial.println(F("Tocando alarme MP3..."));
                    // O rádio precisa estar mudo para não misturar áudios
                    if (radio_ligado) rx.setMute(true); 
                    digitalWrite(vcc_amp, LOW); // LIGA O AMPLIFICADOR! (Faltava isso)
                    df_volume(25); // Volume alto para acordar
                    df_loop(1); // Toca a primeira faixa do SD em repetição
                }
            }
        }
        // Reseta a trava no minuto seguinte
        if (min != alarme_min) {
            last_alarm_min = -1;
        }
      }
    }
    transicao();
    atualiza_lcd(); // Atualiza a tela 16x2 sempre que lemos o RTC
    msLast = ms;
  }
  
  if (ajuste > 0) {
    run_setup_menu();
  } else {
    if (radio_ligado) {
      radio();
    } else {
      rgb_time = 1000; // 1000ms aceso, 1000ms apagado = 1 ciclo a cada 2 segundos (0.5Hz)
      rgb(18, 20, 0);  // laranja
      relogio();
    }
  }
  
  // --- LEITURA DO POTENCIÔMETRO (A6) PARA VOLUME ---
  static unsigned long last_vol_read_orig = 0;
  if (ms - last_vol_read_orig > 200) {
      last_vol_read_orig = ms;
      int pot_val = analogRead(A6);
      
      // O potenciômetro físico já é Logarítmico (Audio Taper)! 
      // Fazer a matemática no software estava criando uma curva dupla distorcida.
      // Voltamos para a leitura 100% linear:
      int novo_volume_radio = map(pot_val, 0, 1023, 0, 15); // Escala RDA5807
      
      static int volume_atual_orig = -1;
      if (novo_volume_radio != volume_atual_orig) {
          volume_atual_orig = novo_volume_radio;
          
          if (radio_ligado) {
              rx.setVolume(volume_atual_orig);
          }
          // MP3 usa escala 0-30
          int novo_volume_mp3 = map(pot_val, 0, 1023, 0, 30);
          df_volume(novo_volume_mp3);
      }
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
  // Ignoramos G e B, pois agora o hardware usa apenas o pino LED_BASE (Pino 9).
  // A cor será definida fisicamente por resistores, então R atua apenas como intensidade.
  if ((millis() - millisTarefa1) < rgb_time) {
    analogWrite(LED_BASE, R);
  } else {
    analogWrite(LED_BASE, 0);
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
