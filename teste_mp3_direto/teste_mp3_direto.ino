#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Configuração dos Pinos no Arduino (Invertidos conforme sua correção física):
// RX = Pino 11 (conecta no TX do DFPlayer)
// TX = Pino 10 (conecta no RX do DFPlayer - ideal com resistor de 1k em série)
SoftwareSerial mySoftwareSerial(11, 10); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

void printDetail(uint8_t type, int value);

void setup() {
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);
  
  Serial.println();
  Serial.println(F("--- TESTE MP3 ISOLADO (BIBLIOTECA SIMPLIFICADA) ---"));
  Serial.println(F("Inicializando DFPlayer Mini..."));
  
  // begin(serial, isACK, doReset)
  // Deixamos isACK como true para testar se há resposta física do chip no boot.
  // Desativamos o reset automático (doReset = false) para maior estabilidade
  if (!myDFPlayer.begin(mySoftwareSerial, true, false)) {
    Serial.println(F("Falha ao inicializar DFPlayer. Verifique a fiação ou se o SD está inserido."));
    while(true); // Trava aqui se falhar de vez
  }
  
  Serial.println(F("DFPlayer Mini Online com sucesso!"));
  
  // Configura volume inicial
  myDFPlayer.volume(25);
  delay(200);
  
  // Toca a música indexada como 1 fisicamente no cartão
  Serial.println(F("Tentando tocar com myDFPlayer.play(1)..."));
  myDFPlayer.play(1); 
}

void loop() {
  // Eliminamos o readState() bloqueante que travava a placa.
  // Em vez disso, apenas lemos eventos assíncronos enviados pelo módulo.
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); // Imprime retornos do DFPlayer
  }

  // Pisca o LED do Arduino para provar que a placa está rodando livre
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(900);
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out (Sem resposta do modulo)!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Errado (Erro de comunicacao)!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Cartao Inserido!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Cartao Removido!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Cartao Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Musica terminada: "));
      Serial.println(value);
      break;
    case DFPlayerError:
      Serial.print(F("Erro retornado pelo DFPlayer: "));
      switch (value) {
        case Busy:
          Serial.println(F("Busy"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Serial Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("CheckSum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("File Mismatch"));
          break;
        /*
        case AdvertiseMismatch:
          Serial.println(F("Advertise Mismatch"));
          break;
        case CardTooLarge:
          Serial.println(F("Card Too Large"));
          break;
        case CardSleeping:
          Serial.println(F("Card Sleeping"));
          break;
        */
        default:
          Serial.println(value);
          break;
      }
      break;
    default:
      break;
  }
}
