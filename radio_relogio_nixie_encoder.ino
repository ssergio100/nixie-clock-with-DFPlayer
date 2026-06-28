#include <Wire.h>
#include <EEPROM.h>
#include <ErriezDS1307.h>
#include <JC_Button.h>
#include <RotaryEncoder.h>
#include <RDA5807.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
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
#define PINO_TEMP A6

// Structs para agrupamento e desacoplamento de estado e configuração
struct Config {
    uint8_t alarme_hora = 6;
    uint8_t alarme_min = 0;
    bool alarme_ligado = false;
    bool alarme_radio = false; // false = Buzzer, true = Rádio
    bool alarme_fds = false;
    uint8_t alarme_som = 1;
    uint8_t alarme_mp3_vol = 25;
    uint8_t chime_som = 2;
    uint8_t chime_volume = 15;
    uint16_t radio_frequencia = 9470; // 94.7 MHz padrão
};

struct SystemState {
    uint8_t estado_menu = 0; // MODO_RELOGIO = 0
    uint8_t passo_setup = 0;
    uint8_t radio_modo_sintonia = 1;
    int encoder_pos_anterior = 0;
    bool radio_ligado = false;
    bool radio_controla_volume = false;
    int volume_radio = 1;
    int volume_mp3 = 10;
    int nixie_brilho_atual = 5;
    bool alarme_tocando = false;
    unsigned long alarme_start_time = 0;
    bool em_soneca = false;
    unsigned long soneca_end_time = 0;
    bool is_slot_machine = false;
    unsigned long slot_machine_start = 0;
    unsigned long bloqueio_clique_ate = 0;
    bool pressionadoLongo = false;
    uint16_t total_faixas = 0;
};

Config cfg;
SystemState state;

Button myBtn(PINO_BOTAO_ENCODER);

int8_t nixie_hora_dez;  //dezena da hora
int8_t nixie_hora_uni;  //unidade da hora
int8_t nixie_min_dez;  //dezena dos minutos
int8_t nixie_min_uni;  //unidade dos minutos
int encoder_pos_atual = 0;
const unsigned long TEMPO_SONECA_MS = 300000UL; // 5 minutos (5 * 60 * 1000)

const uint8_t pinos_bcd_k155id1[] = { 5, 6, 7, 8 };
#define PINO_LED_BASE 9

enum EstadoMenu {
    MODO_RELOGIO = 0,
    SETUP_HORARIO = 1,
    SETUP_ALARME = 2,
    SETUP_DATA = 3,
    SETUP_SONS = 4
};

const int SIGNAL_THRESHOLD = 30;

// Estrutura de tempo
uint8_t hour, min, sec, mday, mon, wday;
uint16_t year;

unsigned long tempo_atual_ms, tempo_ultima_leitura_rtc;
const unsigned long LONG_PRESS = 700;
const unsigned long INTERVALO_ATUALIZACAO_RTC = 1000;

RotaryEncoder *encoder = nullptr;

void ajustarBrilhoNixie(int dir);

uint8_t calcularDiaSemana(uint8_t d, uint8_t m, uint16_t y) {
    if (y < 100) y += 2000;
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

  analogReference(INTERNAL);

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
  } else {
    if (!rtc.isRunning()) {
      Serial.println(F("RTC oscillator is stopped. Starting it..."));
      rtc.clockEnable(true);
    }
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
  df_volume(state.volume_mp3);
  delay(100);
  df_stop();
  delay(100);

  // Bloco de teste: Consultar quantidade de faixas
  state.total_faixas = df_get_total_tracks();

  // Serial.println(F("Inicializando Rádio RDA5807..."));
  rx.setup();
  rx.setBass(false); // Desativa reforço de graves para atenuar o volume excessivo no nível 1
  rx.setBand(2); // Configura banda mundial (76-108 MHz) para suportar a faixa de 76.7 MHz (Classic Pan)
  rx.setSoftmute(false); // Desativa o mute gradual para agilizar a sintonização
  rx.setAudioOutputHighImpedance(true); // Coloca as saídas em alta impedância (desconecta da linha)
  rx.setVolume(0);   // Deixa no mudo via software (volume zero) no boot
  rx.setMono(true); // Mono garante sinal mais limpo em transmissões fracas

  encoder = new RotaryEncoder(PINO_ENCODER_A,PINO_ENCODER_B, RotaryEncoder::LatchMode::FOUR3);
  attachInterrupt(digitalPinToInterrupt(PINO_ENCODER_A), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PINO_ENCODER_B), checkPosition, CHANGE);

  rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday);

  // Iniciar LCD
  lcd.init();
  lcd.noBacklight();
  lcd.clear();

  // Carregar Alarmes e Configurações
  EEPROM.get(0, cfg);
  // Validação de sanidade após carregar da EEPROM (caso seja a primeira inicialização do chip)
  if (cfg.alarme_hora > 23) cfg.alarme_hora = 6;
  if (cfg.alarme_min > 59) cfg.alarme_min = 0;
  cfg.alarme_ligado = (cfg.alarme_ligado == 1);
  cfg.alarme_radio = (cfg.alarme_radio == 1);
  cfg.alarme_fds = (cfg.alarme_fds == 1);
  if (cfg.alarme_som == 0 || cfg.alarme_som > 99) cfg.alarme_som = 1;
  if (cfg.alarme_mp3_vol > 30) cfg.alarme_mp3_vol = 25;
  if (cfg.chime_som == 0 || cfg.chime_som > 99) cfg.chime_som = 2;
  if (cfg.chime_volume > 30) cfg.chime_volume = 15;
  if (cfg.radio_frequencia < 7610 || cfg.radio_frequencia > 10790 || (cfg.radio_frequencia % 20 != 10)) {
      cfg.radio_frequencia = 9470; // 94.7 MHz padrão
  }

  rx.setFrequency(cfg.radio_frequencia);

  // Inicializa a multiplexação por interrupção via Timer1 (a cada 250 microssegundos)
  Timer1.initialize(250);
  Timer1.attachInterrupt(atualizaNixieISR);
}

void loop() {
  encoder_pos_atual = encoder->getPosition();

  // O despertar do estado totalmente desligado é tratado diretamente na rotação para a direita

  tempo_atual_ms = millis();
  myBtn.read();
  processarBotao();

  // Controle de luz de fundo do LCD (apenas liga no setup)
  static bool last_lcd_on = false;
  bool current_lcd_on = (state.estado_menu > MODO_RELOGIO);
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
    if (state.estado_menu == MODO_RELOGIO) { // Só puxa a hora do chip físico se não estivermos no menu
      if (!rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday)) {
        Serial.println(F("Read date/time failed"));
      } else {
        wday = calcularDiaSemana(mday, mon, year); // Garante dia da semana 100% correto calculado em tempo de execução
        verificarDisparoAlarme();
        verificarChime();
      }
    }
    tempo_ultima_leitura_rtc = tempo_atual_ms;
  }
  
  if (state.estado_menu > MODO_RELOGIO) {
    run_setup_menu();
  } else {
    if (state.radio_ligado) {
      radio();
    } else {
      relogio();
      
      // Controle de brilho manual das Nixies via encoder em modo Relógio
      if (state.encoder_pos_anterior != encoder_pos_atual) {
          int dir = (encoder_pos_atual > state.encoder_pos_anterior) ? 1 : -1;
          state.encoder_pos_anterior = encoder_pos_atual;
          ajustarBrilhoNixie(dir);
      }
    }
  }
  
  // Auto-desativação do alarme após 30 minutos (1800000 ms) tocando ininterruptamente
  if (state.alarme_tocando && (millis() - state.alarme_start_time >= 1800000)) {
      state.alarme_tocando = false;
      state.em_soneca = false;
      df_stop();
      rx.setAudioOutputHighImpedance(true);
      digitalWrite(PINO_VCC_AMPLIFICADOR, HIGH); // Desliga amplificador
      state.radio_ligado = false;
      EEPROM.put(0, cfg);
      atualiza_lcd();
  }
  
  // --- ATUALIZAÇÃO DO DISPLAY LCD 16x2 ---
  static unsigned long tempo_ultimo_lcd = 0;
  unsigned long intervalo_lcd = (state.estado_menu > MODO_RELOGIO) ? 50 : 1000;
  if (tempo_atual_ms - tempo_ultimo_lcd >= intervalo_lcd) {
    tempo_ultimo_lcd = tempo_atual_ms;
    atualiza_lcd();
  }
}

