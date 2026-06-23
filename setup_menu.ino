uint8_t calcularDiaSemana(uint8_t d, uint8_t m, uint16_t y);
void atualiza_nixies_setup();

void run_setup_menu() {
    int dir = 0;
    if (encoder_pos_anterior != encoder_pos_atual) {
        dir = (encoder_pos_atual > encoder_pos_anterior) ? 1 : -1;
        encoder_pos_anterior = encoder_pos_atual;
        beep(20); // Bipe de estado_menu ainda mais curto
    }

    if (dir != 0) {
        // Processa a mudança com base no estado e no passo ativo
        switch (estado_menu) {
            case SETUP_HORARIO:
                if (passo_setup == 0) { // Hora
                    if (dir > 0) hour = (hour >= 23) ? 0 : hour + 1;
                    else hour = (hour == 0) ? 23 : hour - 1;
                } else if (passo_setup == 1) { // Minuto
                    if (dir > 0) min = (min >= 59) ? 0 : min + 1;
                    else min = (min == 0) ? 59 : min - 1;
                }
                break;

            case SETUP_ALARME:
                if (passo_setup == 0) { // Hora Alarme
                    if (dir > 0) alarme_hora = (alarme_hora >= 23) ? 0 : alarme_hora + 1;
                    else alarme_hora = (alarme_hora == 0) ? 23 : alarme_hora - 1;
                } else if (passo_setup == 1) { // Minuto Alarme
                    if (dir > 0) alarme_min = (alarme_min >= 59) ? 0 : alarme_min + 1;
                    else alarme_min = (alarme_min == 0) ? 59 : alarme_min - 1;
                } else if (passo_setup == 2) { // Ativação (Atv: Sim / Sim/FS / Nao)
                    // Atv: 0 = Nao, 1 = Sim (dias úteis apenas), 2 = Sim/FS (todos os dias)
                    uint8_t atv = alarme_ligado ? (alarme_fds ? 2 : 1) : 0;
                    if (dir > 0) atv = (atv >= 2) ? 0 : atv + 1;
                    else atv = (atv == 0) ? 2 : atv - 1;

                    if (atv == 0) { alarme_ligado = false; alarme_fds = false; }
                    else if (atv == 1) { alarme_ligado = true; alarme_fds = false; }
                    else { alarme_ligado = true; alarme_fds = true; }
                } else if (passo_setup == 3) { // Tipo (Tp: Fm/mp3)
                    alarme_radio = !alarme_radio;
                }
                break;

            case SETUP_DATA:
                if (passo_setup == 0) { // Dia
                    if (dir > 0) mday = (mday >= 31) ? 1 : mday + 1;
                    else mday = (mday <= 1) ? 31 : mday - 1;
                } else if (passo_setup == 1) { // Mês
                    if (dir > 0) mon = (mon >= 12) ? 1 : mon + 1;
                    else mon = (mon <= 1) ? 12 : mon - 1;
                } else if (passo_setup == 2) { // Ano
                    if (dir > 0) year = (year >= 2099) ? 2000 : year + 1;
                    else year = (year <= 2000) ? 2099 : year - 1;
                }
                // Sincroniza o dia da semana após ajuste de data
                wday = calcularDiaSemana(mday, mon, year);
                break;

            case SETUP_SONS:
                if (passo_setup == 0) { // Faixa Alarme
                    uint16_t max_faixas = (total_faixas > 0) ? total_faixas : 99;
                    if (dir > 0) alarme_som = (alarme_som >= max_faixas) ? 1 : alarme_som + 1;
                    else alarme_som = (alarme_som <= 1) ? max_faixas : alarme_som - 1;

                    digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
                    df_volume(alarme_mp3_vol);
                    delay(50);
                    df_play(alarme_som);
                } else if (passo_setup == 1) { // Vol Alarme
                    if (dir > 0) alarme_mp3_vol = (alarme_mp3_vol >= 30) ? 0 : alarme_mp3_vol + 1;
                    else alarme_mp3_vol = (alarme_mp3_vol == 0) ? 30 : alarme_mp3_vol - 1;

                    df_volume(alarme_mp3_vol);
                } else if (passo_setup == 2) { // Faixa Chime
                    uint16_t max_faixas = (total_faixas > 0) ? total_faixas : 99;
                    if (dir > 0) chime_som = (chime_som >= max_faixas) ? 1 : chime_som + 1;
                    else chime_som = (chime_som <= 1) ? max_faixas : chime_som - 1;

                    digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
                    df_volume(chime_volume);
                    delay(50);
                    df_play(chime_som);
                } else if (passo_setup == 3) { // Vol Chime
                    if (dir > 0) chime_volume = (chime_volume >= 30) ? 0 : chime_volume + 1;
                    else chime_volume = (chime_volume == 0) ? 30 : chime_volume - 1;

                    df_volume(chime_volume);
                }
                break;
        }
    }

    atualiza_nixies_setup();
}

void atualiza_nixies_setup() {
    switch (estado_menu) {
        case SETUP_HORARIO:
            nixie_hora_dez = (hour < 10) ? -1 : (hour / 10);
            nixie_hora_uni = hour % 10;
            nixie_min_dez = min / 10;
            nixie_min_uni = min % 10;
            break;

        case SETUP_ALARME:
            if (passo_setup == 0 || passo_setup == 1) {
                nixie_hora_dez = (alarme_hora < 10) ? -1 : (alarme_hora / 10);
                nixie_hora_uni = alarme_hora % 10;
                nixie_min_dez = alarme_min / 10;
                nixie_min_uni = alarme_min % 10;
            } else if (passo_setup == 2) {
                // Exibe a ativação (0 = Nao, 1 = Sim, 2 = Sim/FS) no dígito 2 (unidade de horas)
                uint8_t atv = alarme_ligado ? (alarme_fds ? 2 : 1) : 0;
                nixie_hora_dez = -1;
                nixie_hora_uni = atv;
                nixie_min_dez = -1;
                nixie_min_uni = -1;
            } else if (passo_setup == 3) {
                // Exibe o tipo de alarme (0 = Fm, 1 = mp3) no dígito 4 (unidade de minutos)
                nixie_hora_dez = -1;
                nixie_hora_uni = -1;
                nixie_min_dez = -1;
                nixie_min_uni = alarme_radio ? 0 : 1;
            }
            break;

        case SETUP_DATA:
            if (passo_setup == 0 || passo_setup == 1) {
                nixie_hora_dez = (mday < 10) ? -1 : (mday / 10);
                nixie_hora_uni = mday % 10;
                nixie_min_dez = (mon < 10) ? -1 : (mon / 10);
                nixie_min_uni = mon % 10;
            } else {
                // Exibe 20 no lado esquerdo e ano no lado direito
                nixie_hora_dez = 2;
                nixie_hora_uni = 0;
                nixie_min_dez = ((year % 100) < 10) ? 0 : ((year % 100) / 10);
                nixie_min_uni = (year % 100) % 10;
            }
            break;

        case SETUP_SONS:
            if (passo_setup == 0 || passo_setup == 1) {
                nixie_hora_dez = (alarme_som < 10) ? -1 : (alarme_som / 10);
                nixie_hora_uni = alarme_som % 10;
                nixie_min_dez = alarme_mp3_vol / 10;
                nixie_min_uni = alarme_mp3_vol % 10;
            } else {
                nixie_hora_dez = (chime_som < 10) ? -1 : (chime_som / 10);
                nixie_hora_uni = chime_som % 10;
                nixie_min_dez = chime_volume / 10;
                nixie_min_uni = chime_volume % 10;
            }
            break;
    }
}
