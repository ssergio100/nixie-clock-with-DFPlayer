bool busca_radio_ativa = false;
int8_t busca_radio_direcao = 0;
uint8_t busca_radio_passos = 0;
unsigned long busca_radio_sintonia_ms = 0;

const uint8_t MAX_PASSOS_BUSCA_RADIO = 170;
const unsigned long INTERVALO_BUSCA_RADIO_MS = 100;

void restaurarMuteRadio() {
    rx.setMute(state.volume_radio == 0);
}

void cancelarBuscaRadio() {
    if (!busca_radio_ativa) return;

    busca_radio_ativa = false;
    restaurarMuteRadio();
}

void sintonizarProximaFrequenciaBusca() {
    if (busca_radio_direcao > 0) {
        cfg.radio_frequencia += 20;
        if (cfg.radio_frequencia > 10790) cfg.radio_frequencia = 7610;
    } else {
        cfg.radio_frequencia -= 20;
        if (cfg.radio_frequencia < 7610) cfg.radio_frequencia = 10790;
    }

    rx.setFrequency(cfg.radio_frequencia);
    atualizarDisplay();
    busca_radio_passos++;
    busca_radio_sintonia_ms = millis();
}

void iniciarBuscaRadio(int8_t direcao) {
    busca_radio_ativa = true;
    busca_radio_direcao = (direcao > 0) ? 1 : -1;
    busca_radio_passos = 0;
    rx.setMute(true);
    sintonizarProximaFrequenciaBusca();
}

void atualizarBuscaRadio() {
    if (!busca_radio_ativa) return;

    if ((millis() - busca_radio_sintonia_ms) < INTERVALO_BUSCA_RADIO_MS) {
        return;
    }

    bool encontrou_estacao = rx.getRssi() >= SIGNAL_THRESHOLD;
    if (encontrou_estacao || busca_radio_passos >= MAX_PASSOS_BUSCA_RADIO) {
        busca_radio_ativa = false;
        restaurarMuteRadio();
        atualiza_lcd();
        return;
    }

    sintonizarProximaFrequenciaBusca();
}

void radio() {
    if (busca_radio_ativa &&
        (state.alarme_tocando || state.radio_controla_volume ||
         state.radio_modo_sintonia != 1)) {
        cancelarBuscaRadio();
    }

    ajustarFrequencia();
    atualizarBuscaRadio();
    atualizarDisplay();
}

void ajustarFrequencia() {
    if (state.alarme_tocando) return; // Ignora o encoder se o alarme estiver tocando

    if (state.encoder_pos_anterior != encoder_pos_atual && state.radio_ligado) {
        int dir = (encoder_pos_atual > state.encoder_pos_anterior) ? 1 : -1;
        state.encoder_pos_anterior = encoder_pos_atual;

        if (state.radio_modo_sintonia == 2) {
            // No modo Relógio do rádio, o encoder ajusta o brilho das Nixies
            ajustarBrilhoNixie(dir);
        } else if (state.radio_controla_volume) {
            if (dir > 0) { // Girou para direita (Aumenta volume)
                if (state.volume_radio < 15) state.volume_radio++;
            } else { // Girou para esquerda (Diminui volume)
                if (state.volume_radio > 0) state.volume_radio--;
            }
            rx.setVolume(state.volume_radio);
            
            // Mute de verdade no volume zero para evitar vazamento analógico
            if (state.volume_radio == 0) {
                rx.setMute(true);
            } else {
                rx.setMute(false);
            }

            atualizarDisplay(); // Atualiza a exibição Nixie imediatamente
            atualiza_lcd();
            // Serial.print(F("Volume digital: "));
            // Serial.println(state.volume_radio);
        } else {
            iniciarBuscaRadio(dir);
            atualiza_lcd();
        }
    }
}

void atualizarDisplay() {
    if (state.radio_controla_volume) {
        int sinal = rx.getRssi();
        nixie_hora_dez = (sinal < 10) ? -1 : (sinal / 10);
        nixie_hora_uni = sinal % 10;
        nixie_min_dez = (state.volume_radio < 10) ? 0 : (state.volume_radio / 10);
        nixie_min_uni = state.volume_radio % 10;
    } else if (state.radio_modo_sintonia == 2) {
        nixie_hora_dez = (hour % 100) / 10;
        nixie_hora_uni = hour % 10;
        nixie_min_dez = (min % 100) / 10;
        nixie_min_uni = min % 10;
    } else {
        nixie_hora_dez = (cfg.radio_frequencia >= 10000) ? 1 : -1;
        nixie_hora_uni = (cfg.radio_frequencia / 1000) % 10;
        nixie_min_dez = (cfg.radio_frequencia / 100) % 10;
        nixie_min_uni = (cfg.radio_frequencia / 10) % 10;
    }
}
