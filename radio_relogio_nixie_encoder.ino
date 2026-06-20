#include <Wire.h>
#include <EEPROM.h>
#include <ErriezDS1307.h>
#include <JC_Button.h>
#include <RotaryEncoder.h>
#include <RDA5807.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "freqs.h"
#include <TimerOne.h>

ErriezDS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial dfSerial(11, 10); // RX (Pino 11), TX (Pino 10) - Corrigido cruzamento

RDA5807 rx;

#define PINO_ANODO_HORA_DEZ 14  //~  14 A0
#define PINO_ANODO_HORA_UNI 15  //~  15 A1
#define PINO_ANODO_MIN_DEZ 17  //~  17 A3
#define PINO_ANODO_MIN_UNI 12
#define PINO_ENCODER_A 2
#define PINO_ENCODER_B 3
#define PINO_BOTAO_ENCODER 4
#define PINO_VCC_AMPLIFICADOR A2
#define PINO_BUZZER 13

Button myBtn(PINO_BOTAO_ENCODER);

int8_t nixie_hora_dez;  //dezena da hora
int8_t nixie_hora_uni;  //unidade da hora
int8_t nixie_min_dez;  //dezena dos minutos
int8_t nixie_min_uni;  //unidade dos minutos


const uint8_t pinos_bcd_k155id1[] = { 5, 6, 7, 8 };
#define PINO_LED_BASE 9

// Variáveis de controle e Alarme
uint8_t estado_menu = 0; // 0 = Relógio Normal, 1-5 = Menus, 6 = Salvar/Sair
uint8_t radio_modo_sintonia = 1; // Fixo no modo Scanner (1) por padrão
int encoder_pos_anterior = 0;
bool radio_ligado = false;

// Variáveis para controle de volume digital via encoder (remoção do potenciômetro)
bool radio_controla_volume = false; // true = encoder altera volume, false = altera frequência (sintonia)
int volume_radio = 5;               // Volume padrão do rádio (escala 0-15)
int volume_mp3 = 10;                // Volume padrão do MP3 (escala 0-30)
int volume_sono = 0;                // Volume do som de dormir (chuva, faixa 3, 0 a 30)

// Controle de Brilho das Nixies e LED base (LDR em A6)
int nixie_brilho_atual = 10;                        // Brilho físico ativo (1 a 10) usado pela ISR


uint8_t alarme_hora = 6;
uint8_t alarme_min = 0;
bool alarme_ligado = false;
bool alarme_radio = false; // 0 = Buzzer, 1 = Rádio
bool alarme_fds = false;

// Estado do Alarme
// Mapeamento de Sons (SD Card)
uint8_t alarme_som = 1;            // Faixa editável do alarme
uint8_t alarme_mp3_vol = 25;       // Volume editável do alarme MP3
uint8_t chime_som = 2;             // Faixa editável do chime (hora cheia)
uint8_t chime_volume = 15;         // Volume editável do chime
uint16_t total_faixas = 0;         // Total de faixas detectado no cartão SD
const uint16_t SOM_CHUVA = 3;      // Faixa 3 do SD (Chuva para dormir)

bool alarme_tocando = false;
bool em_soneca = false;
unsigned long soneca_end_time = 0;
const unsigned long TEMPO_SONECA_MS = 300000; // 5 minutos (5 * 60 * 1000)

// Caça-Níqueis
bool is_slot_machine = false;
unsigned long slot_machine_start = 0;

// Frequência e posicionamento
int encoder_pos_atual;
uint8_t radio_total_estacoes;
uint8_t radio_estacao_atual = 12;
const int SIGNAL_THRESHOLD = 12;

// Tempo e RGB


// Estrutura de tempo
uint8_t hour, min, sec, mday, mon, wday;
uint16_t year;

// Configuração de botões e estados

bool pressionadoLongo = false;
unsigned long tempo_atual_ms, tempo_ultima_leitura_rtc;
const unsigned long LONG_PRESS = 700;
const unsigned long INTERVALO_ATUALIZACAO_RTC = 1000;


RotaryEncoder *encoder = nullptr;


void setup() {
  Serial.begin(9600);

  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_VCC_AMPLIFICADOR, OUTPUT);
  pinMode(PINO_LED_BASE, OUTPUT);

  pinMode(pinos_bcd_k155id1[0], OUTPUT);
  pinMode(pinos_bcd_k155id1[1], OUTPUT);
  pinMode(pinos_bcd_k155id1[2], OUTPUT);
  pinMode(pinos_bcd_k155id1[3], OUTPUT);

  pinMode(PINO_ANODO_HORA_DEZ, OUTPUT);
  pinMode(PINO_ANODO_HORA_UNI, OUTPUT);
  pinMode(PINO_ANODO_MIN_DEZ, OUTPUT);
  pinMode(PINO_ANODO_MIN_UNI, OUTPUT);

  Wire.begin();

  if (!rtc.begin()) {
    Serial.println(F("RTC not found"));  // Apenas avisa, sem travar
  }

  //    if (!rtc.setDateTime(12, 34, 56,  31, 12, 2020, 0)) {
  //          Serial.println(F("Set date/time failed"));
  //          ++falhas;
  //      }


  rtc.setSquareWave(SquareWaveDisable); // SQW em LOW permanente → LED do RTC sempre aceso
  digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);  //amplificador desligado
  myBtn.begin();
  dfSerial.begin(9600); // Inicia a comunicação com o DFPlayer Mini
  
  // Aguarda boot elétrico do DFPlayer
  Serial.println(F("Aguardando 2s para boot do DFPlayer..."));
  delay(2000);

  // Inicialização raiz - O DFPlayer não envia handshake confiável, apenas damos o tempo.
  Serial.println(F("DFPlayer Mini Online (Modo Nativo)."));
  
  // Configura volume inicial digital (padrão fixado)
  df_volume(volume_mp3);
  delay(100);
  df_stop();
  delay(100);

  // Bloco de teste: Consultar quantidade de faixas
  Serial.println(F("--- TESTE DE CONSULTA DE FAIXAS ---"));
  total_faixas = df_get_total_tracks();
  Serial.print(F("DFPlayer retornou total de faixas: "));
  Serial.println(total_faixas);
  Serial.println(F("-----------------------------------"));

  Serial.println(F("Inicializando Rádio RDA5807..."));
  rx.setup();
  rx.setVolume(3); // Volume fixo em 5 (1/3 de 15) conforme solicitado para testes
  rx.setSoftmute(false); // Desativa o mute gradual para agilizar a sintonização
  rx.setAudioOutputHighImpedance(true); // Coloca as saídas em alta impedância (desconecta da linha)
  rx.setVolume(0);   // Deixa no mudo via software (volume zero)
  rx.setMono(true); // Mono garante sinal mais limpo em transmissões fracas
  rx.setFrequency((uint16_t)(freqs[radio_estacao_atual] * 100)); // PU2CLR usa uint16_t (ex: 10630)


  encoder = new RotaryEncoder(PINO_ENCODER_B,PINO_ENCODER_A, RotaryEncoder::LatchMode::FOUR3);
  attachInterrupt(digitalPinToInterrupt(PINO_ENCODER_A), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PINO_ENCODER_B), checkPosition, CHANGE);

  rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday);

  // Iniciar LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Carregar Alarmes e Configurações
  alarme_hora = EEPROM.read(0);
  if (alarme_hora > 23) alarme_hora = 6;
  alarme_min = EEPROM.read(1);
  if (alarme_min > 59) alarme_min = 0;
  alarme_ligado = EEPROM.read(2) == 1;
  alarme_radio = EEPROM.read(3) == 1;
  alarme_fds = EEPROM.read(4) == 1;
  alarme_som = EEPROM.read(5);
  if (alarme_som == 0 || alarme_som > 99) alarme_som = 1;
  alarme_mp3_vol = EEPROM.read(6);
  if (alarme_mp3_vol > 30) alarme_mp3_vol = 25;
  chime_som = EEPROM.read(7);
  if (chime_som == 0 || chime_som > 99) chime_som = 2;
  chime_volume = EEPROM.read(8);
  if (chime_volume > 30) chime_volume = 15;

  radio_total_estacoes = getFreqsSize();

  // O DS1307 só suporta 100kHz! Acelerar para 400kHz pode corromper as leituras ou travar o chip.
  // Como resolvemos o pisca-pisca das válvulas com o "cache do LCD", não precisamos mais do I2C em overclock.
  // Wire.setClock(400000);

  // Inicializa a multiplexação por interrupção via Timer1 (a cada 250 microssegundos)
  Timer1.initialize(250);
  Timer1.attachInterrupt(atualizaNixieISR);
}

void loop() {
  encoder_pos_atual = encoder->getPosition();

  tempo_atual_ms = millis();
  myBtn.read();
  state();

  if ((tempo_atual_ms - tempo_ultima_leitura_rtc) >= INTERVALO_ATUALIZACAO_RTC) {
    if (estado_menu == 0) { // Só puxa a hora do chip físico se não estivermos no menu
      if (!rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday)) {
        Serial.println(F("Read date/time failed"));
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
                if (volume_sono > 0) {
                    df_stop();
                    volume_sono = 0;
                }
                if (alarme_radio) {
                    if (!radio_ligado) {
                        Serial.println(F("Ligando rádio para o alarme..."));
                        radio_ligado = true;
                        rx.setAudioOutputHighImpedance(false); // Conecta o rádio à linha
                        rx.setVolume(volume_radio); // Aplica volume digital configurado (nível 5 fixo)
                        if (volume_radio == 0) {
                            rx.setMute(true);
                        } else {
                            rx.setMute(false);
                        }
                        digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
                    }
                } else {
                    Serial.println(F("Tocando alarme MP3..."));
                    rx.setAudioOutputHighImpedance(true); // Desconecta o rádio da linha
                    radio_ligado = false; // Garante a sincronia do estado
                    digitalWrite(PINO_VCC_AMPLIFICADOR, LOW); // LIGA O AMPLIFICADOR!
                    df_play(alarme_som); // Toca o alarme PRIMEIRO (clones ignoram volume se estiverem parados)
                    delay(150); // Dá tempo do clone carregar o MP3 na memória
                    df_volume(alarme_mp3_vol); // Sobe o volume agora que está tocando
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
    tempo_ultima_leitura_rtc = tempo_atual_ms;
  }
  
  if (estado_menu > 0) {
    run_setup_menu();
  } else {
    if (radio_ligado) {
      radio();
    } else {
      rgb(18, 20, 0);  // laranja
      relogio();
      gerenciarSomSono(); // Gerencia o som de dormir e volume via encoder
    }
  }
  
  // --- LEITURA E FILTRAGEM DO LDR EM A6 (SUAVIZAÇÃO E HISTERESE) ---
  static unsigned long last_ldr_read = 0;
  static int ldr_filtrado = -1;
  
  if (tempo_atual_ms - last_ldr_read >= 200) {
      last_ldr_read = tempo_atual_ms;
      int ldr_raw = analogRead(A6);
      
      if (ldr_filtrado < 0) {
          ldr_filtrado = ldr_raw;
      } else {
          // Filtro EMA (Média Móvel Exponencial): Peso de 5% na nova leitura. Constante de tempo ~2 segundos.
          ldr_filtrado = ((long)ldr_filtrado * 95 + (long)ldr_raw * 5) / 100;
      }
      
      // Tabelas de histerese para os limites de transição de brilho (1 a 10)
      // THRESHOLD_UP[i] define o limite para subir do nível i+1 para i+2
      // THRESHOLD_DOWN[i] define o limite para descer do nível i+2 para i+1
      static const int THRESHOLD_UP[9]   = {180, 350, 520, 650, 740, 800, 835, 855, 868};
      static const int THRESHOLD_DOWN[9] = {140, 310, 480, 610, 705, 770, 810, 835, 853};
      
      int novo_brilho = nixie_brilho_atual;
      if (novo_brilho < 1) novo_brilho = 1;
      if (novo_brilho > 10) novo_brilho = 10;
      
      // Tenta subir o brilho
      while (novo_brilho < 10 && ldr_filtrado > THRESHOLD_UP[novo_brilho - 1]) {
          novo_brilho++;
      }
      // Tenta descer o brilho
      while (novo_brilho > 1 && ldr_filtrado < THRESHOLD_DOWN[novo_brilho - 2]) {
          novo_brilho--;
      }
      
      // Se houve alteração real, aplica e imprime no console serial
      if (novo_brilho != nixie_brilho_atual) {
          Serial.print(F("LDR Mudanca Brilho: "));
          Serial.print(nixie_brilho_atual);
          Serial.print(F(" -> "));
          Serial.print(novo_brilho);
          Serial.print(F(" (LDR Raw: "));
          Serial.print(ldr_raw);
          Serial.print(F(", Filt: "));
          Serial.print(ldr_filtrado);
          Serial.println(F(")"));
          
          nixie_brilho_atual = novo_brilho;
      }
  }

  // --- ATUALIZAÇÃO DO DISPLAY LCD 16x2 ---
  static unsigned long tempo_ultimo_lcd = 0;
  unsigned long intervalo_lcd = (estado_menu > 0) ? 50 : 1000;
  if (tempo_atual_ms - tempo_ultimo_lcd >= intervalo_lcd) {
    tempo_ultimo_lcd = tempo_atual_ms;
    atualiza_lcd();
  }

  // A multiplexação agora roda em background via interrupção do Timer1
}



void atualizaNixieISR() {
  static uint8_t digito_ativo = 0;
  static uint8_t sub_estado = 0; // Varia de 0 a 11 (ciclo total de 3ms por dígito)
  
  // 1. Determina se o dígito deve estar ativo na animação do setup (piscar)
  bool show_digits = (estado_menu == 0) || ((millis() / 300) % 2 == 0);
  
  // 2. Determina se o LED base deve estar aceso neste momento
  bool led_aceso_ciclo = false;
  if (estado_menu == 0 && !radio_ligado) {
    static unsigned long ultimo_toggle_led = 0;
    static bool led_estado_pisca = true;
    if (millis() - ultimo_toggle_led >= 2000) {
      ultimo_toggle_led = millis();
      led_estado_pisca = !led_estado_pisca;
    }
    led_aceso_ciclo = led_estado_pisca;
  } else if (radio_ligado && radio_controla_volume) {
    led_aceso_ciclo = true;
  } else if (estado_menu > 0) {
    led_aceso_ciclo = true;
  }

  // 3. Controle PWM do brilho Nixie + LED base (sem nenhum delay bloqueante na ISR)
  if (show_digits && sub_estado < nixie_brilho_atual) {
    if (digito_ativo == 0 && nixie_hora_dez > 0) {
      escreve_byte_fast(nixie_hora_dez);
      digitalWrite(PINO_ANODO_HORA_DEZ, HIGH);
    } else if (digito_ativo == 1 && nixie_hora_uni > -1) {
      escreve_byte_fast(nixie_hora_uni);
      digitalWrite(PINO_ANODO_HORA_UNI, HIGH);
    } else if (digito_ativo == 2 && nixie_min_dez > -1) {
      escreve_byte_fast(nixie_min_dez);
      digitalWrite(PINO_ANODO_MIN_DEZ, HIGH);
    } else if (digito_ativo == 3 && nixie_min_uni > -1) {
      escreve_byte_fast(nixie_min_uni);
      digitalWrite(PINO_ANODO_MIN_UNI, HIGH);
    }
    
    if (led_aceso_ciclo) {
      digitalWrite(PINO_LED_BASE, LOW); // Liga LED
    } else {
      digitalWrite(PINO_LED_BASE, HIGH); // Apaga LED
    }
  } else {
    // Fora da largura de pulso de brilho (ou durante os passos de blanking 10 e 11): Apaga tudo!
    digitalWrite(PINO_ANODO_HORA_DEZ, LOW);
    digitalWrite(PINO_ANODO_HORA_UNI, LOW);
    digitalWrite(PINO_ANODO_MIN_DEZ, LOW);
    digitalWrite(PINO_ANODO_MIN_UNI, LOW);
    digitalWrite(PINO_LED_BASE, HIGH);
  }

  // 4. Incrementa o sub-estado a cada 250us
  sub_estado++;
  if (sub_estado >= 12) { // 12 x 250us = 3ms (tempo do dígito completo)
    sub_estado = 0;
    digito_ativo = (digito_ativo + 1) % 4; // Avança para o próximo tubo
  }
}

void escreve_byte_fast(byte x) {
  if (x & 0b00000001) digitalWrite(pinos_bcd_k155id1[0], HIGH); else digitalWrite(pinos_bcd_k155id1[0], LOW);
  if (x & 0b00000010) digitalWrite(pinos_bcd_k155id1[1], HIGH); else digitalWrite(pinos_bcd_k155id1[1], LOW);
  if (x & 0b00000100) digitalWrite(pinos_bcd_k155id1[2], HIGH); else digitalWrite(pinos_bcd_k155id1[2], LOW);
  if (x & 0b00001000) digitalWrite(pinos_bcd_k155id1[3], HIGH); else digitalWrite(pinos_bcd_k155id1[3], LOW);
}

void rgb(int R, int G, int B) {
  // O brilho e o piscar do LED base agora são controlados sincronizadamente na ISR do Timer1
}

void gerenciarSomSono() {
    if (encoder_pos_anterior != encoder_pos_atual) {
        int dir = (encoder_pos_atual > encoder_pos_anterior) ? 1 : -1;
        encoder_pos_anterior = encoder_pos_atual;
        
        bool alterou = false;

        if (dir < 0) { // Horário (Liga / Aumenta volume)
            if (volume_sono == 0) {
                volume_sono = 1;
                digitalWrite(PINO_VCC_AMPLIFICADOR, LOW); // Liga amplificador
                df_volume(volume_sono);
                delay(50);
                df_loop(SOM_CHUVA); // Toca faixa de chuva em loop
                alterou = true;
            } else if (volume_sono < 30) {
                volume_sono++;
                df_volume(volume_sono);
                alterou = true;
            }
        } else { // Anti-horário (Diminui volume / desliga)
            if (volume_sono > 0) {
                volume_sono--;
                if (volume_sono == 0) {
                    df_stop();
                    if (!radio_ligado) {
                        digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH); // Desliga amp
                    }
                } else {
                    df_volume(volume_sono);
                }
                alterou = true;
            }
        }
        
        if (alterou) {
            beep(30);
            atualiza_lcd();
            Serial.print(F("Volume Chuva: "));
            Serial.println(volume_sono);
        }
    }
}

void checkPosition() {
  encoder->tick();
}
