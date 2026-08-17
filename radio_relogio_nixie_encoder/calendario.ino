bool anoBissexto(uint16_t ano) {
  return (ano % 4 == 0) && ((ano % 100 != 0) || (ano % 400 == 0));
}

uint8_t obterDiasNoMes(uint8_t mes, uint16_t ano) {
  switch (mes) {
  case 2:
    return anoBissexto(ano) ? 29 : 28;
  case 4:
  case 6:
  case 9:
  case 11:
    return 30;
  case 1:
  case 3:
  case 5:
  case 7:
  case 8:
  case 10:
  case 12:
    return 31;
  default:
    return 0;
  }
}

bool dataValida(uint8_t dia, uint8_t mes, uint16_t ano) {
  if (ano < 2000 || ano > 2099) {
    return false;
  }

  uint8_t dias_no_mes = obterDiasNoMes(mes, ano);
  return dias_no_mes != 0 && dia >= 1 && dia <= dias_no_mes;
}

uint8_t limitarDiaAoMes(uint8_t dia, uint8_t mes, uint16_t ano) {
  uint8_t dias_no_mes = obterDiasNoMes(mes, ano);
  if (dias_no_mes == 0) {
    return 1;
  }
  if (dia < 1) {
    return 1;
  }
  return (dia > dias_no_mes) ? dias_no_mes : dia;
}

uint8_t calcularDiaSemana(uint8_t dia, uint8_t mes, uint16_t ano) {
  if (ano < 100) {
    ano += 2000;
  }
  if (!dataValida(dia, mes, ano)) {
    return 1; // Retorno seguro: 1 = domingo.
  }

  static const uint8_t ajuste_mes[] = {0, 3, 2, 5, 0, 3,
                                       5, 1, 4, 6, 2, 4};
  if (mes < 3) {
    ano--;
  }
  return ((ano + ano / 4 - ano / 100 + ano / 400 +
           ajuste_mes[mes - 1] + dia) %
          7) +
         1; // 1 = domingo, 7 = sabado.
}

const char *obterNomeDiaSemana(uint8_t dia_semana) {
  switch (dia_semana) {
  case 1:
    return "Domingo";
  case 2:
    return "Segunda";
  case 3:
    return "Terca";
  case 4:
    return "Quarta";
  case 5:
    return "Quinta";
  case 6:
    return "Sexta";
  case 7:
    return "Sabado";
  default:
    return "Erro";
  }
}
