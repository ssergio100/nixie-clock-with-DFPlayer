void run_setup_menu() {
    // Cor do setup (LED pisca em 2s/2s — comportamento fixo)
    rgb(20, 0, 0);

    int dir = 0;
    if (encoder_pos_anterior != encoder_pos_atual) {
        dir = (encoder_pos_atual > encoder_pos_anterior) ? 1 : -1;
        encoder_pos_anterior = encoder_pos_atual;
        beep(20); // Bipe de estado_menu ainda mais curto
    }

    switch (estado_menu) {
        case 1: // Ajuste da Hora do Relógio
            if (dir > 0) { // Girou Esquerda - Ajusta Hora
                hour = (hour >= 23) ? 0 : hour + 1;
            } else if (dir < 0) { // Girou Direita - Ajusta Minuto
                min = (min >= 59) ? 0 : min + 1;
            }
            nixie_hora_dez = (hour < 10) ? -1 : (hour / 10);
            nixie_hora_uni = hour % 10;
            nixie_min_dez = min / 10;
            nixie_min_uni = min % 10;
            break;

        case 2: // Ajuste do Alarme (Hora/Minuto)
            if (dir > 0) { // Girou Esquerda - Ajusta Hora
                alarme_hora = (alarme_hora >= 23) ? 0 : alarme_hora + 1;
            } else if (dir < 0) { // Girou Direita - Ajusta Minuto
                alarme_min = (alarme_min >= 59) ? 0 : alarme_min + 1;
            }
            nixie_hora_dez = (alarme_hora < 10) ? -1 : (alarme_hora / 10);
            nixie_hora_uni = alarme_hora % 10;
            nixie_min_dez = alarme_min / 10;
            nixie_min_uni = alarme_min % 10;
            break;

        case 3: // Opções do Alarme (Som e Status)
            if (dir > 0) { // Esquerda - Rádio ou MP3
                alarme_radio = !alarme_radio;
            } else if (dir < 0) { // Direita - Ligado ou Desligado
                alarme_ligado = !alarme_ligado;
            }
            // Som (Esquerda)
            nixie_hora_dez = -1; // Esconde zero à esquerda
            nixie_hora_uni = alarme_radio ? 1 : 0;
            // Status (Direita)
            nixie_min_dez = -1; // Esconde zero à esquerda
            nixie_min_uni = alarme_ligado ? 1 : 0;
            break;

        case 4: // Ajuste de Dia e Mês
            if (dir > 0) { // Girou Esquerda - Ajusta Dia
                mday = (mday >= 31) ? 1 : mday + 1;
            } else if (dir < 0) { // Girou Direita - Ajusta Mês
                mon = (mon >= 12) ? 1 : mon + 1;
            }
            nixie_hora_dez = (mday < 10) ? -1 : (mday / 10);
            nixie_hora_uni = mday % 10;
            nixie_min_dez = (mon < 10) ? -1 : (mon / 10);
            nixie_min_uni = mon % 10;
            break;

        case 5: // Ajuste de Ano
            if (dir > 0) { // Esquerda - Decrementa
                year = (year <= 2000) ? 2099 : year - 1;
            } else if (dir < 0) { // Direita - Incrementa
                year = (year >= 2099) ? 2000 : year + 1;
            }
            nixie_hora_dez = 2; // Século 20
            nixie_hora_uni = 0;
            nixie_min_dez = ((year % 100) < 10) ? 0 : ((year % 100) / 10);
            nixie_min_uni = (year % 100) % 10;
            break;

        case 6: // Ajuste de Alarme Fim de Semana
            if (dir > 0 || dir < 0) { // Qualquer lado alterna ligado/desligado
                alarme_fds = !alarme_fds;
            }
            nixie_hora_dez = -1; // Desliga dezenas de hora
            nixie_hora_uni = -1; // Desliga unidades de hora
            nixie_min_dez = -1; // Desliga dezenas de minuto
            nixie_min_uni = alarme_fds ? 1 : 0; // Exibe 1 ou 0 na extremidade direita
            break;
    }
}
