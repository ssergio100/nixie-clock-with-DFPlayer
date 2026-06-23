uint8_t calcularDiaSemana(uint8_t d, uint8_t m, uint16_t y);
void atualiza_nixies_setup();

void run_setup_menu() {
    int dir = 0;
    if (state.encoder_pos_anterior != encoder_pos_atual) {
        dir = (encoder_pos_atual > state.encoder_pos_anterior) ? 1 : -1;
        state.encoder_pos_anterior = encoder_pos_atual;
        beep(20); // Bipe de estado_menu ainda mais curto
    }

    if (dir != 0) {
        // Processa a mudança com base no estado e no passo ativo
        switch (state.estado_menu) {
            case SETUP_HORARIO:
                if (state.passo_setup == 0) { // Hora
                    if (dir > 0) hour = (hour >= 23) ? 0 : hour + 1;
                    else hour = (hour == 0) ? 23 : hour - 1;
                } else if (state.passo_setup == 1) { // Minuto
                    if (dir > 0) min = (min >= 59) ? 0 : min + 1;
                    else min = (min == 0) ? 59 : min - 1;
                }
                break;

            case SETUP_ALARME:
                if (state.passo_setup == 0) { // Hora Alarme
                    if (dir > 0) cfg.alarme_hora = (cfg.alarme_hora >= 23) ? 0 : cfg.alarme_hora + 1;
                    else cfg.alarme_hora = (cfg.alarme_hora == 0) ? 23 : cfg.alarme_hora - 1;
                } else if (state.passo_setup == 1) { // Minuto Alarme
                    if (dir > 0) cfg.alarme_min = (cfg.alarme_min >= 59) ? 0 : cfg.alarme_min + 1;
                    else cfg.alarme_min = (cfg.alarme_min == 0) ? 59 : cfg.alarme_min - 1;
                } else if (state.passo_setup == 2) { // Ativação (Atv: Sim / Sim/FS / Nao)
                    // Atv: 0 = Nao, 1 = Sim (dias úteis apenas), 2 = Sim/FS (todos os dias)
                    uint8_t atv = cfg.alarme_ligado ? (cfg.alarme_fds ? 2 : 1) : 0;
                    if (dir > 0) atv = (atv >= 2) ? 0 : atv + 1;
                    else atv = (atv == 0) ? 2 : atv - 1;

                    if (atv == 0) { cfg.alarme_ligado = false; cfg.alarme_fds = false; }
                    else if (atv == 1) { cfg.alarme_ligado = true; cfg.alarme_fds = false; }
                    else { cfg.alarme_ligado = true; cfg.alarme_fds = true; }
                } else if (state.passo_setup == 3) { // Tipo (Tp: Fm/mp3)
                    cfg.alarme_radio = !cfg.alarme_radio;
                }
                break;

            case SETUP_DATA:
                if (state.passo_setup == 0) { // Dia
                    if (dir > 0) mday = (mday >= 31) ? 1 : mday + 1;
                    else mday = (mday <= 1) ? 31 : mday - 1;
                } else if (state.passo_setup == 1) { // Mês
                    if (dir > 0) mon = (mon >= 12) ? 1 : mon + 1;
                    else mon = (mon <= 1) ? 12 : mon - 1;
                } else if (state.passo_setup == 2) { // Ano
                    if (dir > 0) year = (year >= 2099) ? 2000 : year + 1;
                    else year = (year <= 2000) ? 2099 : year - 1;
                }
                // Sincroniza o dia da semana após ajuste de data
                wday = calcularDiaSemana(mday, mon, year);
                break;

            case SETUP_SONS:
                if (state.passo_setup == 0) { // Faixa Alarme
                    uint16_t max_faixas = (state.total_faixas > 0) ? state.total_faixas : 99;
                    if (dir > 0) cfg.alarme_som = (cfg.alarme_som >= max_faixas) ? 1 : cfg.alarme_som + 1;
                    else cfg.alarme_som = (cfg.alarme_som <= 1) ? max_faixas : cfg.alarme_som - 1;

                    digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
                    df_volume(cfg.alarme_mp3_vol);
                    delay(50);
                    df_play(cfg.alarme_som);
                } else if (state.passo_setup == 1) { // Vol Alarme
                    if (dir > 0) cfg.alarme_mp3_vol = (cfg.alarme_mp3_vol >= 30) ? 0 : cfg.alarme_mp3_vol + 1;
                    else cfg.alarme_mp3_vol = (cfg.alarme_mp3_vol == 0) ? 30 : cfg.alarme_mp3_vol - 1;

                    df_volume(cfg.alarme_mp3_vol);
                } else if (state.passo_setup == 2) { // Faixa Chime
                    uint16_t max_faixas = (state.total_faixas > 0) ? state.total_faixas : 99;
                    if (dir > 0) cfg.chime_som = (cfg.chime_som >= max_faixas) ? 1 : cfg.chime_som + 1;
                    else cfg.chime_som = (cfg.chime_som <= 1) ? max_faixas : cfg.chime_som - 1;

                    digitalWrite(PINO_VCC_AMPLIFICADOR, LOW);
                    df_volume(cfg.chime_volume);
                    delay(50);
                    df_play(cfg.chime_som);
                } else if (state.passo_setup == 3) { // Vol Chime
                    if (dir > 0) cfg.chime_volume = (cfg.chime_volume >= 30) ? 0 : cfg.chime_volume + 1;
                    else cfg.chime_volume = (cfg.chime_volume == 0) ? 30 : cfg.chime_volume - 1;

                    df_volume(cfg.chime_volume);
                }
                break;
        }
    }

    atualiza_nixies_setup();
}

void atualiza_nixies_setup() {
    switch (state.estado_menu) {
        case SETUP_HORARIO:
            nixie_hora_dez = (hour < 10) ? -1 : (hour / 10);
            nixie_hora_uni = hour % 10;
            nixie_min_dez = min / 10;
            nixie_min_uni = min % 10;
            break;

        case SETUP_ALARME:
            if (state.passo_setup == 0 || state.passo_setup == 1) {
                nixie_hora_dez = (cfg.alarme_hora < 10) ? -1 : (cfg.alarme_hora / 10);
                nixie_hora_uni = cfg.alarme_hora % 10;
                nixie_min_dez = cfg.alarme_min / 10;
                nixie_min_uni = cfg.alarme_min % 10;
            } else if (state.passo_setup == 2) {
                // Exibe a ativação (0 = Nao, 1 = Sim, 2 = Sim/FS) no dígito 2 (unidade de horas)
                uint8_t atv = cfg.alarme_ligado ? (cfg.alarme_fds ? 2 : 1) : 0;
                nixie_hora_dez = -1;
                nixie_hora_uni = atv;
                nixie_min_dez = -1;
                nixie_min_uni = -1;
            } else if (state.passo_setup == 3) {
                // Exibe o tipo de alarme (0 = Fm, 1 = mp3) no dígito 4 (unidade de minutos)
                nixie_hora_dez = -1;
                nixie_hora_uni = -1;
                nixie_min_dez = -1;
                nixie_min_uni = cfg.alarme_radio ? 0 : 1;
            }
            break;

        case SETUP_DATA:
            if (state.passo_setup == 0 || state.passo_setup == 1) {
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
            if (state.passo_setup == 0 || state.passo_setup == 1) {
                nixie_hora_dez = (cfg.alarme_som < 10) ? -1 : (cfg.alarme_som / 10);
                nixie_hora_uni = cfg.alarme_som % 10;
                nixie_min_dez = cfg.alarme_mp3_vol / 10;
                nixie_min_uni = cfg.alarme_mp3_vol % 10;
            } else {
                nixie_hora_dez = (cfg.chime_som < 10) ? -1 : (cfg.chime_som / 10);
                nixie_hora_uni = cfg.chime_som % 10;
                nixie_min_dez = cfg.chime_volume / 10;
                nixie_min_uni = cfg.chime_volume % 10;
            }
            break;
    }
}
