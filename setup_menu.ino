void run_setup_menu() {
    // Cor do setup (Vermelho)
    rgb_time = 500;
    rgb(20, 0, 0);

    int dir = 0;
    if (pos != newPos) {
        dir = (newPos > pos) ? 1 : -1;
        pos = newPos;
        beep(20); // Bipe de ajuste ainda mais curto
    }

    switch (ajuste) {
        case 1: // Ajuste da Hora do Relógio
            if (dir > 0) { // Girou Esquerda - Ajusta Hora
                hour = (hour >= 23) ? 0 : hour + 1;
            } else if (dir < 0) { // Girou Direita - Ajusta Minuto
                min = (min >= 59) ? 0 : min + 1;
            }
            h_d = (hour < 10) ? -1 : (hour / 10);
            h_u = hour % 10;
            m_d = min / 10;
            m_u = min % 10;
            break;

        case 2: // Ajuste do Alarme (Hora/Minuto)
            if (dir > 0) { // Girou Esquerda - Ajusta Hora
                alarme_hora = (alarme_hora >= 23) ? 0 : alarme_hora + 1;
            } else if (dir < 0) { // Girou Direita - Ajusta Minuto
                alarme_min = (alarme_min >= 59) ? 0 : alarme_min + 1;
            }
            h_d = (alarme_hora < 10) ? -1 : (alarme_hora / 10);
            h_u = alarme_hora % 10;
            m_d = alarme_min / 10;
            m_u = alarme_min % 10;
            break;

        case 3: // Opções do Alarme (Som e Status)
            if (dir > 0) { // Esquerda - Rádio ou Buzzer
                alarme_radio = !alarme_radio;
            } else if (dir < 0) { // Direita - Ligado ou Desligado
                alarme_ligado = !alarme_ligado;
            }
            // Som (Esquerda)
            h_d = -1; // Esconde zero à esquerda
            h_u = alarme_radio ? 1 : 0;
            // Status (Direita)
            m_d = -1; // Esconde zero à esquerda
            m_u = alarme_ligado ? 1 : 0;
            break;

        case 4: // Ajuste de Dia e Mês
            if (dir > 0) { // Girou Esquerda - Ajusta Dia
                mday = (mday >= 31) ? 1 : mday + 1;
            } else if (dir < 0) { // Girou Direita - Ajusta Mês
                mon = (mon >= 12) ? 1 : mon + 1;
            }
            h_d = (mday < 10) ? -1 : (mday / 10);
            h_u = mday % 10;
            m_d = (mon < 10) ? -1 : (mon / 10);
            m_u = mon % 10;
            break;

        case 5: // Ajuste de Ano
            if (dir > 0) { // Esquerda - Decrementa
                year = (year <= 2000) ? 2099 : year - 1;
            } else if (dir < 0) { // Direita - Incrementa
                year = (year >= 2099) ? 2000 : year + 1;
            }
            h_d = 2; // Século 20
            h_u = 0;
            m_d = ((year % 100) < 10) ? 0 : ((year % 100) / 10);
            m_u = (year % 100) % 10;
            break;

        case 6: // Ajuste de Alarme Fim de Semana
            if (dir > 0 || dir < 0) { // Qualquer lado alterna ligado/desligado
                alarme_fds = !alarme_fds;
            }
            h_d = -1; // Desliga dezenas de hora
            h_u = -1; // Desliga unidades de hora
            m_d = -1; // Desliga dezenas de minuto
            m_u = alarme_fds ? 1 : 0; // Exibe 1 ou 0 na extremidade direita
            break;
    }
}
