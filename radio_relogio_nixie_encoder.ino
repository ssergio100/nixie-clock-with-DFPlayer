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

// Endereços de Memória EEPROM para salvar as configurações
#define EEPROM_ADDR_ALARME_HORA 0
#define EEPROM_ADDR_ALARME_MIN 1
#define EEPROM_ADDR_ALARME_LIGADO 2
#define EEPROM_ADDR_ALARME_RADIO 3
#define EEPROM_ADDR_ALARME_FDS 4
#define EEPROM_ADDR_ALARME_SOM 5
#define EEPROM_ADDR_ALARME_MP3_VOL 6
#define EEPROM_ADDR_CHIME_SOM 7
#define EEPROM_ADDR_CHIME_VOLUME 8

Button myBtn(PINO_BOTAO_ENCODER);

int8_t nixie_hora_dez;  //dezena da hora
int8_t nixie_hora_uni;  //unidade da hora
int8_t nixie_min_dez;  //dezena dos minutos
int8_t nixie_min_uni;  //unidade dos minutos

const uint8_t pinos_bcd_k155id1[] = { 5, 6, 7, 8 };
#define PINO_LED_BASE 9

enum EstadoMenu {
    MODO_RELOGIO = 0,
    SETUP_HORARIO = 1,
    SETUP_ALARME = 2,
    SETUP_DATA = 3,
    SETUP_SONS = 4
};
uint8_t estado_menu = MODO_RELOGIO; // 0 = Relógio Normal, 1-4 = Menus de Setup
uint8_t passo_setup = 0;           // Passo ativo em cada tela de setup
uint8_t radio_modo_sintonia = 1; // Fixo no modo Scanner (1) por padrão
int encoder_pos_anterior = 0;
bool radio_ligado = false;

// Variáveis para controle de volume digital via encoder (remoção do potenciômetro)
bool radio_controla_volume = false; // true = encoder altera volume, false = altera frequência (sintonia)
int volume_radio = 1;               // Volume padrão do rádio (escala 0-15)
int volume_mp3 = 10;                // Volume padrão do MP3 (escala 0-30)

// Controle de Brilho das Nixies e LED base
int nixie_brilho_atual = 6;                        // Brilho físico ativo (1 a 11) usado pela ISR

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

bool alarme_tocando = false;
unsigned long alarme_start_time = 0;
bool em_soneca = false;
unsigned long soneca_end_time = 0;
const unsigned long TEMPO_SONECA_MS = 300000; // 5 minutos (5 * 60 * 1000)

// Caça-Níqueis
bool is_slot_machine = false;
unsigned long slot_machine_start = 0;

// Screen Saver (Salva Displays) Manual
bool screen_saver_ativo = false;
unsigned long bloqueio_clique_ate = 0;

// Frequência e posicionamento
int encoder_pos_atual;
uint8_t radio_total_estacoes;
uint8_t radio_estacao_atual = 12;
const int SIGNAL_THRESHOLD = 30;

// Estrutura de tempo
uint8_t hour, min, sec, mday, mon, wday;
uint16_t year;

bool pressionadoLongo = false;
unsigned long tempo_atual_ms, tempo_ultima_leitura_rtc;
const unsigned long LONG_PRESS = 700;
const unsigned long INTERVALO_ATUALIZACAO_RTC = 1000;

RotaryEncoder *encoder = nullptr;

uint8_t calcularDiaSemana(uint8_t d, uint8_t m, uint16_t y) {
    if (m < 1 || m > 12 || d < 1 || d > 31 || y < 2000) {
        return 1; // Retorno de segurança (1 = Domingo) se os dados forem inválidos/não inicializados
    }
    static const uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) {
        y -= 1;
    }
    return ((y + y/4 - y/100 + y/400 + t[m-1] + d) % 7) + 1; // 1 = Domingo, 7 = Sábado
}

const char* obterNomeDiaSemana(uint8_t wd) {
    switch (wd) {
        case 1: return "Domingo";
        case 2: return "Segunda";
        case 3: return "Terca";
        case 4: return "Quarta";
        case 5: return "Quinta";
        case 6: return "Sexta";
        case 7: return "Sabado";
        default: return "Erro";
    }
}

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

  rtc.setSquareWave(SquareWaveDisable); // SQW em LOW permanente → LED do RTC sempre aceso
  digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH);  //amplificador desligado
  myBtn.begin();
  dfSerial.begin(9600); // Inicia a comunicação com o DFPlayer Mini
  
  // Aguarda boot elétrico do DFPlayer
  // Serial.println(F("Aguardando 2s para boot do DFPlayer..."));
  delay(2000);

  // Inicialização raiz - O DFPlayer não envia handshake confiável, apenas damos o tempo.
  // Serial.println(F("DFPlayer Mini Online (Modo Nativo)."));
  
  // Configura volume inicial digital (padrão fixado)
  df_volume(volume_mp3);
  delay(100);
  df_stop();
  delay(100);

  // Bloco de teste: Consultar quantidade de faixas
  // Serial.println(F("--- TESTE DE CONSULTA DE FAIXAS ---"));
  total_faixas = df_get_total_tracks();
  // Serial.print(F("DFPlayer retornou total de faixas: "));
  // Serial.println(total_faixas);
  // Serial.println(F("-----------------------------------"));

  // Serial.println(F("Inicializando Rádio RDA5807..."));
  rx.setup();
  rx.setBass(false); // Desativa reforço de graves para atenuar o volume excessivo no nível 1
  rx.setBand(2); // Configura banda mundial (76-108 MHz) para suportar a faixa de 76.7 MHz (Classic Pan)
  rx.setSoftmute(false); // Desativa o mute gradual para agilizar a sintonização
  rx.setAudioOutputHighImpedance(true); // Coloca as saídas em alta impedância (desconecta da linha)
  rx.setVolume(0);   // Deixa no mudo via software (volume zero) no boot
  rx.setMono(true); // Mono garante sinal mais limpo em transmissões fracas
  rx.setFrequency((uint16_t)(freqs[radio_estacao_atual] * 100 + 0.5)); // Arredondamento evita truncagem de float (+0.1 MHz offset)

  encoder = new RotaryEncoder(PINO_ENCODER_A,PINO_ENCODER_B, RotaryEncoder::LatchMode::FOUR3);
  attachInterrupt(digitalPinToInterrupt(PINO_ENCODER_A), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PINO_ENCODER_B), checkPosition, CHANGE);

  rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday);

  // Iniciar LCD
  lcd.init();
  lcd.noBacklight();
  lcd.clear();

  // Carregar Alarmes e Configurações
  alarme_hora = EEPROM.read(EEPROM_ADDR_ALARME_HORA);
  if (alarme_hora > 23) alarme_hora = 6;
  alarme_min = EEPROM.read(EEPROM_ADDR_ALARME_MIN);
  if (alarme_min > 59) alarme_min = 0;
  alarme_ligado = EEPROM.read(EEPROM_ADDR_ALARME_LIGADO) == 1;
  alarme_radio = EEPROM.read(EEPROM_ADDR_ALARME_RADIO) == 1;
  alarme_fds = EEPROM.read(EEPROM_ADDR_ALARME_FDS) == 1;
  alarme_som = EEPROM.read(EEPROM_ADDR_ALARME_SOM);
  if (alarme_som == 0 || alarme_som > 99) alarme_som = 1;
  alarme_mp3_vol = EEPROM.read(EEPROM_ADDR_ALARME_MP3_VOL);
  if (alarme_mp3_vol > 30) alarme_mp3_vol = 25;
  chime_som = EEPROM.read(EEPROM_ADDR_CHIME_SOM);
  if (chime_som == 0 || chime_som > 99) chime_som = 2;
  chime_volume = EEPROM.read(EEPROM_ADDR_CHIME_VOLUME);
  if (chime_volume > 30) chime_volume = 15;

  radio_total_estacoes = getFreqsSize();

  // Inicializa a multiplexação por interrupção via Timer1 (a cada 250 microssegundos)
  Timer1.initialize(250);
  Timer1.attachInterrupt(atualizaNixieISR);
}

void loop() {
  encoder_pos_atual = encoder->getPosition();

  // Trata o despertar através do giro do encoder
  if (encoder_pos_anterior != encoder_pos_atual) {
      if (screen_saver_ativo) {
          screen_saver_ativo = false;
          encoder_pos_anterior = encoder_pos_atual;
          beep(50);
      }
  }

  tempo_atual_ms = millis();
  myBtn.read();
  state();

  // Controle de luz de fundo do LCD (apenas liga no setup)
  static bool last_lcd_on = false;
  bool current_lcd_on = (estado_menu > MODO_RELOGIO);
  if (current_lcd_on != last_lcd_on) {
      last_lcd_on = current_lcd_on;
      if (current_lcd_on) {
          lcd.backlight();
          atualiza_lcd(); // Desenha imediatamente
      } else {
          lcd.noBacklight();
          lcd.clear(); // Limpa a tela ao sair do setup
      }
  }

  if ((tempo_atual_ms - tempo_ultima_leitura_rtc) >= INTERVALO_ATUALIZACAO_RTC) {
    if (estado_menu == MODO_RELOGIO) { // Só puxa a hora do chip físico se não estivermos no menu
      if (!rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday)) {
        Serial.println(F("Read date/time failed"));
      } else {
        verificarDisparoAlarme();
        verificarChime();
      }
    }
    tempo_ultima_leitura_rtc = tempo_atual_ms;
  }
  
  if (estado_menu > MODO_RELOGIO) {
    run_setup_menu();
  } else {
    if (radio_ligado) {
      radio();
    } else {
      relogio();
      
      // Controle de brilho manual das Nixies via encoder em modo Relógio
      if (encoder_pos_anterior != encoder_pos_atual && !screen_saver_ativo) {
          int dir = (encoder_pos_atual > encoder_pos_anterior) ? 1 : -1;
          encoder_pos_anterior = encoder_pos_atual;
          
          if (dir > 0) { // Horário / Direita (Aumenta brilho)
              if (nixie_brilho_atual < 11) {
                  nixie_brilho_atual++;
                  beep(15);
              }
          } else { // Anti-horário / Esquerda (Diminui brilho)
              if (nixie_brilho_atual > 1) {
                  nixie_brilho_atual--;
                  beep(15);
              }
          }
      }
    }
  }
  
  // Auto-desativação do alarme após 30 minutos (1800000 ms) tocando ininterruptamente
  if (alarme_tocando && (tempo_atual_ms - alarme_start_time >= 1800000)) {
      alarme_tocando = false;
      em_soneca = false;
      df_stop();
      rx.setAudioOutputHighImpedance(true);
      digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH); // Desliga amplificador
      radio_ligado = false;
      atualiza_lcd();
  }
  
  // --- ATUALIZAÇÃO DO DISPLAY LCD 16x2 ---
  static unsigned long tempo_ultimo_lcd = 0;
  unsigned long intervalo_lcd = (estado_menu > MODO_RELOGIO) ? 50 : 1000;
  if (tempo_atual_ms - tempo_ultimo_lcd >= intervalo_lcd) {
    tempo_ultimo_lcd = tempo_atual_ms;
    atualiza_lcd();
  }
}
