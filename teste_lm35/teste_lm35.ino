#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inicializa o LCD no endereço I2C 0x27 (16 colunas e 2 linhas)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definição do pino analógico conectado ao LM35
#define PINO_LM35 A6

// Constante do filtro exponencial (Alpha: 0.1 a 0.2 é excelente para temperatura)
const float ALPHA = 0.15;
float tempFiltrada = 0.0;
bool primeiraLeitura = true;

void setup() {
  // Inicia a comunicação serial para monitoramento e depuração
  Serial.begin(9600);
  Serial.println(F("--- Termometro de Alta Estabilidade (LM35) ---"));

  // BOAS PRÁTICAS: Mudar a referência analógica para o pino de referência interna de 1.1V.
  // Isso resolve dois problemas graves do LM35 no Arduino:
  // 1. Ruído e flutuação da alimentação de 5V da USB (que distorce a leitura).
  // 2. Melhora a resolução do ADC de 4.88mV (escala de 5V) para 1.07mV (escala de 1.1V),
  //    fornecendo precisão de aproximadamente 0.1 °C.
  analogReference(INTERNAL);

  // Inicializa o LCD e liga a luz de fundo
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Mensagem de inicialização
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  lcd.setCursor(0, 1);
  lcd.print("Termometro LM35");
  delay(2000);
  lcd.clear();
}

float lerTemperaturaFiltroMedia() {
  // BOAS PRÁTICAS: Fazer múltiplas leituras consecutivas para remover ruídos aleatórios de alta frequência.
  long soma = 0;
  
  // Realiza 20 amostras com pequeno intervalo para estabilizar o capacitor do ADC interno
  for (int i = 0; i < 20; i++) {
    soma += analogRead(PINO_LM35);
    delay(5);
  }
  
  float mediaADC = soma / 20.0;

  // Conversão para Celsius com referência interna de 1.1V:
  // Volt = (ADC_Lido * 1.1) / 1024
  // Temp = Volt * 100 (visto que o LM35 fornece 10mV/°C)
  // Portanto: Temp = (ADC_Lido * 110.0) / 1024.0
  float tempCelsius = (mediaADC * 110.0) / 1024.0;
  
  return tempCelsius;
}

void loop() {
  // Executa a leitura filtrada por média
  float tempInstantanea = lerTemperaturaFiltroMedia();

  // BOAS PRÁTICAS: Filtro Exponencial Móvel (EMA) para suavizar variações bruscas de leitura
  if (primeiraLeitura) {
    tempFiltrada = tempInstantanea;
    primeiraLeitura = false;
  } else {
    tempFiltrada = (tempInstantanea * ALPHA) + (tempFiltrada * (1.0 - ALPHA));
  }

  // Envia os dados para a porta Serial (útil para plotar gráficos na IDE do Arduino)
  Serial.print(F("Bruto(ADC): "));
  Serial.print(analogRead(PINO_LM35));
  Serial.print(F(" | Temp_Inst: "));
  Serial.print(tempInstantanea, 2);
  Serial.print(F(" C | Temp_Filtrada: "));
  Serial.print(tempFiltrada, 2);
  Serial.println(F(" C"));

  // Exibe no Display LCD 16x2
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(tempFiltrada, 1); // Exibe com 1 casa decimal
  lcd.print(" \xDF""C ");       // Código \xDF imprime o símbolo de grau "°" nos LCDs padrão HD44780

  lcd.setCursor(0, 1);
  lcd.print("ADC: ");
  lcd.print(analogRead(PINO_LM35));
  lcd.print(" Vref:1.1V");

  // Aguarda 1 segundo antes da próxima leitura para evitar o auto-aquecimento do sensor (self-heating)
  delay(1000);
}
