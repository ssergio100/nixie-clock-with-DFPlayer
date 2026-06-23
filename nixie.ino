void atualizaNixieISR() {
  static uint8_t digito_ativo = 0;
  static uint8_t sub_estado = 0; // Varia de 0 a 11 (ciclo total de 3ms por dígito)
  
  // 1. Determina se o dígito deve estar ativo na animação do setup (piscar)
  bool show_digits = true;
  if (estado_menu > MODO_RELOGIO) {
      bool blink_cycle = ((millis() / 300) % 2 == 0);
      if (!blink_cycle) {
          if (estado_menu == SETUP_HORARIO || estado_menu == SETUP_ALARME) {
              if (passo_setup == 0 && (digito_ativo == 0 || digito_ativo == 1)) show_digits = false;
              else if (passo_setup == 1 && (digito_ativo == 2 || digito_ativo == 3)) show_digits = false;
              else if (estado_menu == SETUP_ALARME && passo_setup == 2 && digito_ativo == 1) show_digits = false;
              else if (estado_menu == SETUP_ALARME && passo_setup == 3 && digito_ativo == 3) show_digits = false;
          } else if (estado_menu == SETUP_DATA) {
              if (passo_setup == 0 && (digito_ativo == 0 || digito_ativo == 1)) show_digits = false;
              else if (passo_setup == 1 && (digito_ativo == 2 || digito_ativo == 3)) show_digits = false;
              else if (passo_setup == 2 && (digito_ativo == 2 || digito_ativo == 3)) show_digits = false;
          } else if (estado_menu == SETUP_SONS) {
              if (passo_setup == 0 && (digito_ativo == 0 || digito_ativo == 1)) show_digits = false;
              else if (passo_setup == 1 && (digito_ativo == 2 || digito_ativo == 3)) show_digits = false;
              else if (passo_setup == 2 && (digito_ativo == 0 || digito_ativo == 1)) show_digits = false;
              else if (passo_setup == 3 && (digito_ativo == 2 || digito_ativo == 3)) show_digits = false;
          }
      }
  }
  
  // 2. Determina se o LED base deve estar aceso neste momento
  bool led_aceso_ciclo = false;
  if (screen_saver_ativo) {
    led_aceso_ciclo = false;
  } else if (estado_menu == MODO_RELOGIO && !radio_ligado) {
    static unsigned long ultimo_toggle_led = 0;
    static bool led_estado_pisca = true;
    if (millis() - ultimo_toggle_led >= 2000) {
      ultimo_toggle_led = millis();
      led_estado_pisca = !led_estado_pisca;
    }
    led_aceso_ciclo = led_estado_pisca;
  } else if (radio_ligado && radio_controla_volume) {
    led_aceso_ciclo = true;
  } else if (estado_menu > MODO_RELOGIO) {
    led_aceso_ciclo = true;
  }

  // 3. Controle PWM do brilho Nixie + LED base (sem nenhum delay bloqueante na ISR, exceto t_settle de 15us no início do ciclo)
  if (show_digits && sub_estado < nixie_brilho_atual) {
    if (digito_ativo == 0 && nixie_hora_dez > 0) {
      escreve_byte_fast(nixie_hora_dez);
      if (sub_estado == 0) delayMicroseconds(15); // WAIT de assentamento (t_settle) apenas no primeiro passo do tubo
      digitalWrite(PINO_ANODO_HORA_DEZ, HIGH);
    } else if (digito_ativo == 1 && nixie_hora_uni > -1) {
      escreve_byte_fast(nixie_hora_uni);
      if (sub_estado == 0) delayMicroseconds(15); // WAIT de assentamento (t_settle) apenas no primeiro passo do tubo
      digitalWrite(PINO_ANODO_HORA_UNI, HIGH);
    } else if (digito_ativo == 2 && nixie_min_dez > -1) {
      escreve_byte_fast(nixie_min_dez);
      if (sub_estado == 0) delayMicroseconds(15); // WAIT de assentamento (t_settle) apenas no primeiro passo do tubo
      digitalWrite(PINO_ANODO_MIN_DEZ, HIGH);
    } else if (digito_ativo == 3 && nixie_min_uni > -1) {
      escreve_byte_fast(nixie_min_uni);
      if (sub_estado == 0) delayMicroseconds(15); // WAIT de assentamento (t_settle) apenas no primeiro passo do tubo
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

