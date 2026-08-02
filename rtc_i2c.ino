// Protecao e recuperacao do barramento I2C compartilhado pelo RTC, LCD e radio.

void configurarWire() {
  Wire.begin();
  // No AVR o timeout vem desativado por padrao. O segundo argumento reinicia
  // o periferico TWI quando a espera ultrapassa o limite.
  Wire.setWireTimeout(TIMEOUT_I2C_US, true);
  Wire.clearWireTimeoutFlag();
}

void recuperarBarramentoI2C() {
  Wire.end();

  // I2C e open-drain: nivel alto e obtido liberando o pino, nunca dirigindo
  // HIGH contra um dispositivo que possa estar mantendo a linha em LOW.
  digitalWrite(SDA, LOW);
  pinMode(SDA, INPUT_PULLUP);
  digitalWrite(SCL, LOW);
  pinMode(SCL, INPUT_PULLUP);
  delayMicroseconds(5);

  // Um escravo interrompido no meio de um byte deve liberar SDA depois de no
  // maximo nove clocks.
  for (uint8_t pulso = 0; pulso < 9 && digitalRead(SDA) == LOW; pulso++) {
    digitalWrite(SCL, LOW);
    pinMode(SCL, OUTPUT);
    delayMicroseconds(5);
    pinMode(SCL, INPUT_PULLUP);
    delayMicroseconds(5);
  }

  // Gera uma condicao STOP: SDA sobe enquanto SCL esta liberado em HIGH.
  digitalWrite(SDA, LOW);
  pinMode(SDA, OUTPUT);
  delayMicroseconds(5);
  pinMode(SCL, INPUT_PULLUP);
  delayMicroseconds(5);
  pinMode(SDA, INPUT_PULLUP);
  delayMicroseconds(5);

  configurarWire();
  i2c_requer_recuperacao = false;
}

void prepararTransacaoI2C() {
  if (Wire.getWireTimeoutFlag()) {
    Wire.clearWireTimeoutFlag();
    i2c_requer_recuperacao = true;
  }

  if (i2c_requer_recuperacao) {
    recuperarBarramentoI2C();
  }
}

void marcarRTCIndisponivel(const __FlashStringHelper *etapa) {
  bool houve_timeout = Wire.getWireTimeoutFlag();
  Wire.clearWireTimeoutFlag();
  i2c_requer_recuperacao = i2c_requer_recuperacao || houve_timeout;
  rtc_disponivel = false;

  // Evita inundar a Serial enquanto o RTC permanecer desconectado.
  if (!rtc_falha_reportada) {
    Serial.print(F("RTC indisponivel em: "));
    Serial.print(etapa);
    if (houve_timeout) {
      Serial.print(F(" (timeout I2C)"));
    }
    Serial.println();
  }
  rtc_falha_reportada = true;
}

bool lerDataHoraRTC() {
  if (!rtc_disponivel) {
    return false;
  }

  prepararTransacaoI2C();
  bool leitura_ok =
      rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday);
  if (!leitura_ok || Wire.getWireTimeoutFlag()) {
    marcarRTCIndisponivel(F("leitura"));
    return false;
  }

  return true;
}

bool gravarDataHoraRTC() {
  if (!rtc_disponivel) {
    return false;
  }

  prepararTransacaoI2C();
  // O projeto usa 1=domingo..7=sabado; a ErriezDS1307 espera 0..6.
  uint8_t wday_rtc = (wday >= 1 && wday <= 7) ? wday - 1 : 0;
  bool gravacao_ok =
      rtc.setDateTime(hour, min, sec, mday, mon, year, wday_rtc);
  if (!gravacao_ok || Wire.getWireTimeoutFlag()) {
    marcarRTCIndisponivel(F("gravacao"));
    return false;
  }

  return true;
}

bool inicializarRTC() {
  tempo_ultima_tentativa_rtc = millis();
  prepararTransacaoI2C();

  if (!rtc.begin() || Wire.getWireTimeoutFlag()) {
    marcarRTCIndisponivel(F("deteccao"));
    return false;
  }

  Wire.clearWireTimeoutFlag();
  bool oscilador_ativo = rtc.isRunning();
  if (Wire.getWireTimeoutFlag()) {
    marcarRTCIndisponivel(F("oscilador"));
    return false;
  }

  if (!oscilador_ativo) {
    Serial.println(F("RTC oscillator is stopped. Starting it..."));
    Wire.clearWireTimeoutFlag();
    if (!rtc.clockEnable(true) || Wire.getWireTimeoutFlag()) {
      marcarRTCIndisponivel(F("partida do oscilador"));
      return false;
    }
  }

  Wire.clearWireTimeoutFlag();
  if (!rtc.setSquareWave(SquareWaveDisable) || Wire.getWireTimeoutFlag()) {
    marcarRTCIndisponivel(F("configuracao SQW"));
    return false;
  }

  // A inicializacao so termina quando uma data/hora valida puder ser lida.
  Wire.clearWireTimeoutFlag();
  bool leitura_ok =
      rtc.getDateTime(&hour, &min, &sec, &mday, &mon, &year, &wday);
  if (!leitura_ok || Wire.getWireTimeoutFlag()) {
    marcarRTCIndisponivel(F("leitura inicial"));
    return false;
  }

  bool foi_reconectado = rtc_falha_reportada;
  rtc_disponivel = true;
  rtc_falha_reportada = false;
  if (foi_reconectado) {
    Serial.println(F("RTC reconectado"));
  }

  return true;
}

bool tentarReconectarRTC() {
  if (rtc_disponivel) {
    return true;
  }

  if ((millis() - tempo_ultima_tentativa_rtc) < INTERVALO_RECONEXAO_RTC) {
    return false;
  }

  return inicializarRTC();
}
