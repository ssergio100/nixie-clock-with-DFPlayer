
void state() {

  // Se o botão for mantido pressionado (Toque Longo)
  if (myBtn.pressedFor(LONG_PRESS) && !pressionadoLongo) {
      pressionadoLongo = true; // Trava para não repetir
      
      if (radio_ligado == 0) {
          Serial.println("Radio ON");
          rfm.setStandby(0);
          digitalWrite(vcc_amp, LOW);
          radio_ligado = 1;
      } else {
          Serial.print("Rádio OFF");
          rfm.setStandby(1);
          digitalWrite(vcc_amp, HIGH);
          beep(70);
          radio_ligado = 0;
      }
  }

  // Quando o botão for solto
  if (myBtn.wasReleased()) {
      if (pressionadoLongo) {
          // Acabou de soltar após um toque longo. Apenas reseta a trava.
          pressionadoLongo = false;
      } else {
          // Foi um TOQUE CURTO genuíno
          beep(70);
          
          if (radio_ligado == 0) {
              // No modo RELÓGIO: Toque curto altera o ajuste (Horas -> Minutos -> Normal)
              ajuste = (ajuste >= 2) ? 0 : ajuste + 1;
              Serial.print("ajuste:\n");
              Serial.print(ajuste);
          } else {
              // No modo RÁDIO: Toque curto altera o f_step (Memória -> Busca -> Relógio)
              f_step = (f_step >= 2) ? 0 : f_step + 1;
          }
      }
  }
}
