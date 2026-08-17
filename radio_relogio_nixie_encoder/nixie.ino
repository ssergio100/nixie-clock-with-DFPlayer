const int8_t NIVEL_NIXIE_SEM_PISCA = -1;
const int8_t NIVEL_NIXIE_DESLIGADO = -2;
const int8_t NIVEL_NIXIE_MAXIMO = 10;

struct QuadroNixie {
  int8_t digitos[4];
  uint8_t mascara_digitos;
  uint8_t limite_brilho;
  bool led_aceso;
};

volatile QuadroNixie quadro_nixie = {{-1, -1, -1, -1}, 0, 0, false};

void prepararQuadroNixie() {
  uint8_t mascara_digitos = 0x0F;

  // Calcula no loop quais tubos piscam durante o setup.
  if (state.estado_menu > MODO_RELOGIO && ((millis() / 300) % 2 != 0)) {
    if (state.estado_menu == SETUP_HORARIO ||
        state.estado_menu == SETUP_ALARME) {
      if (state.passo_setup == 0) mascara_digitos &= ~0x03;
      else if (state.passo_setup == 1) mascara_digitos &= ~0x0C;
      else if (state.estado_menu == SETUP_ALARME && state.passo_setup == 2)
        mascara_digitos &= ~0x02;
      else if (state.estado_menu == SETUP_ALARME && state.passo_setup == 3)
        mascara_digitos &= ~0x08;
    } else if (state.estado_menu == SETUP_DATA) {
      if (state.passo_setup == 0) mascara_digitos &= ~0x03;
      else if (state.passo_setup == 1 || state.passo_setup == 2)
        mascara_digitos &= ~0x0C;
    } else if (state.estado_menu == SETUP_SONS) {
      if (state.passo_setup == 0 || state.passo_setup == 2)
        mascara_digitos &= ~0x03;
      else if (state.passo_setup == 1 || state.passo_setup == 3)
        mascara_digitos &= ~0x0C;
    }
  }

  int limite_brilho = state.nixie_brilho_atual;
  if (state.estado_menu > MODO_RELOGIO || state.alarme_tocando ||
      state.em_soneca) {
    limite_brilho = (limite_brilho < 0) ? 6 : limite_brilho + 1;
  } else if (limite_brilho >= 0) {
    limite_brilho++;
  } else if (limite_brilho == NIVEL_NIXIE_SEM_PISCA) {
    limite_brilho = 1;
  } else {
    limite_brilho = 0;
  }
  limite_brilho = constrain(limite_brilho, 0, 11);

  bool led_aceso = false;
  if (state.estado_menu == MODO_RELOGIO &&
      (!state.radio_ligado || state.radio_modo_sintonia == 2)) {
    bool exibindo_temperatura =
        ((sec % TEMP_EXIBIR_INTERVALO) >=
         (TEMP_EXIBIR_INTERVALO - TEMP_EXIBIR_DURACAO)) &&
        state.nixie_brilho_atual != NIVEL_NIXIE_DESLIGADO &&
        !state.alarme_tocando && !state.is_slot_machine;

    if (!exibindo_temperatura &&
        (state.nixie_brilho_atual >= 0 || state.alarme_tocando ||
         state.em_soneca)) {
      static unsigned long ultimo_toggle_led = 0;
      static bool led_estado_pisca = true;
      if ((millis() - ultimo_toggle_led) >= 500) {
        ultimo_toggle_led = millis();
        led_estado_pisca = !led_estado_pisca;
      }
      led_aceso = led_estado_pisca;
    }
  } else if (state.radio_ligado && state.radio_controla_volume) {
    led_aceso = true;
  } else if (state.estado_menu > MODO_RELOGIO) {
    led_aceso = true;
  }

  // A ISR nunca enxerga um quadro parcialmente atualizado.
  noInterrupts();
  quadro_nixie.digitos[0] = nixie_hora_dez;
  quadro_nixie.digitos[1] = nixie_hora_uni;
  quadro_nixie.digitos[2] = nixie_min_dez;
  quadro_nixie.digitos[3] = nixie_min_uni;
  quadro_nixie.mascara_digitos = mascara_digitos;
  quadro_nixie.limite_brilho = (uint8_t)limite_brilho;
  quadro_nixie.led_aceso = led_aceso;
  interrupts();
}

void atualizaNixieISR() {
  static uint8_t digito_ativo = 0;
  static uint8_t sub_estado = 0;

  bool mostrar_digito =
      (quadro_nixie.mascara_digitos & (1 << digito_ativo)) != 0;
  uint8_t limite_brilho = quadro_nixie.limite_brilho;

  if (mostrar_digito && sub_estado < limite_brilho) {
    int8_t valor = quadro_nixie.digitos[digito_ativo];
    if ((digito_ativo == 0 && valor > 0) ||
        (digito_ativo != 0 && valor >= 0)) {
      escreve_byte_fast(valor);
      if (sub_estado == 0) delayMicroseconds(15);

      if (digito_ativo == 0) digitalWrite(PINO_ANODO_HORA_DEZ, HIGH);
      else if (digito_ativo == 1) digitalWrite(PINO_ANODO_HORA_UNI, HIGH);
      else if (digito_ativo == 2) digitalWrite(PINO_ANODO_MIN_DEZ, HIGH);
      else digitalWrite(PINO_ANODO_MIN_UNI, HIGH);
    }

    digitalWrite(PINO_LED_BASE, quadro_nixie.led_aceso ? LOW : HIGH);
  } else {
    digitalWrite(PINO_ANODO_HORA_DEZ, LOW);
    digitalWrite(PINO_ANODO_HORA_UNI, LOW);
    digitalWrite(PINO_ANODO_MIN_DEZ, LOW);
    digitalWrite(PINO_ANODO_MIN_UNI, LOW);
    digitalWrite(PINO_LED_BASE, HIGH);
  }

  sub_estado++;
  if (sub_estado >= 12) {
    sub_estado = 0;
    digito_ativo = (digito_ativo + 1) % 4;
  }
}

void escreve_byte_fast(byte x) {
  if (x & 0b00000001) digitalWrite(pinos_bcd_k155id1[0], HIGH); else digitalWrite(pinos_bcd_k155id1[0], LOW);
  if (x & 0b00000010) digitalWrite(pinos_bcd_k155id1[1], HIGH); else digitalWrite(pinos_bcd_k155id1[1], LOW);
  if (x & 0b00000100) digitalWrite(pinos_bcd_k155id1[2], HIGH); else digitalWrite(pinos_bcd_k155id1[2], LOW);
  if (x & 0b00001000) digitalWrite(pinos_bcd_k155id1[3], HIGH); else digitalWrite(pinos_bcd_k155id1[3], LOW);
}

void ajustarBrilhoNixie(int dir) {
  if (dir > 0) { // Horário / Direita (Aumenta brilho)
    if (state.nixie_brilho_atual < NIVEL_NIXIE_MAXIMO) {
      state.nixie_brilho_atual++;
      beep(15);
    }
  } else { // Anti-horário / Esquerda (Diminui brilho)
    if (state.nixie_brilho_atual > NIVEL_NIXIE_DESLIGADO) {
      state.nixie_brilho_atual--;
      beep(15);
    }
  }
}

