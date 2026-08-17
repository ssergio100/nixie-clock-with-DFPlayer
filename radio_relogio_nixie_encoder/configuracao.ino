// Persistencia versionada das configuracoes na EEPROM.

const uint16_t ASSINATURA_CONFIG = 0x4E58; // "NX"
const uint8_t VERSAO_CONFIG = 1;

uint16_t calcularCrcConfig(const uint8_t *dados, size_t tamanho) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < tamanho; i++) {
    crc ^= (uint16_t)dados[i] << 8;
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
  }
  return crc;
}

uint16_t calcularCrcRegistro(const RegistroConfig &registro) {
  return calcularCrcConfig((const uint8_t *)&registro,
                           offsetof(RegistroConfig, crc));
}

bool valoresDadosPersistidosValidos(const DadosConfigPersistidos &valor) {
  return valor.alarme_hora <= 23 && valor.alarme_min <= 59 &&
         valor.alarme_ligado <= 1 && valor.alarme_radio <= 1 &&
         valor.alarme_fds <= 1 &&
         valor.alarme_som >= 1 && valor.alarme_som <= 99 &&
         valor.alarme_mp3_vol <= 30 && valor.chime_som >= 1 &&
         valor.chime_som <= 99 && valor.chime_volume <= 30 &&
         valor.radio_frequencia >= 7610 &&
         valor.radio_frequencia <= 10790 &&
         (valor.radio_frequencia % 20 == 10);
}

bool valoresConfigValidos(const Config &valor) {
  return valor.alarme_hora <= 23 && valor.alarme_min <= 59 &&
         valor.alarme_som >= 1 && valor.alarme_som <= 99 &&
         valor.alarme_mp3_vol <= 30 && valor.chime_som >= 1 &&
         valor.chime_som <= 99 && valor.chime_volume <= 30 &&
         valor.radio_frequencia >= 7610 &&
         valor.radio_frequencia <= 10790 &&
         (valor.radio_frequencia % 20 == 10);
}

void copiarConfigParaDados(DadosConfigPersistidos &dados) {
  dados.alarme_hora = cfg.alarme_hora;
  dados.alarme_min = cfg.alarme_min;
  dados.alarme_ligado = cfg.alarme_ligado ? 1 : 0;
  dados.alarme_radio = cfg.alarme_radio ? 1 : 0;
  dados.alarme_fds = cfg.alarme_fds ? 1 : 0;
  dados.alarme_som = cfg.alarme_som;
  dados.alarme_mp3_vol = cfg.alarme_mp3_vol;
  dados.chime_som = cfg.chime_som;
  dados.chime_volume = cfg.chime_volume;
  dados.radio_frequencia = cfg.radio_frequencia;
}

void copiarDadosParaConfig(const DadosConfigPersistidos &dados) {
  cfg.alarme_hora = dados.alarme_hora;
  cfg.alarme_min = dados.alarme_min;
  cfg.alarme_ligado = dados.alarme_ligado != 0;
  cfg.alarme_radio = dados.alarme_radio != 0;
  cfg.alarme_fds = dados.alarme_fds != 0;
  cfg.alarme_som = dados.alarme_som;
  cfg.alarme_mp3_vol = dados.alarme_mp3_vol;
  cfg.chime_som = dados.chime_som;
  cfg.chime_volume = dados.chime_volume;
  cfg.radio_frequencia = dados.radio_frequencia;
}

void salvarConfiguracao() {
  RegistroConfig registro = {};
  registro.assinatura = ASSINATURA_CONFIG;
  registro.versao = VERSAO_CONFIG;
  copiarConfigParaDados(registro.dados);
  registro.crc = calcularCrcRegistro(registro);

  // EEPROM.put atualiza somente os bytes que realmente mudaram.
  EEPROM.put(0, registro);
}

void carregarConfiguracao() {
  RegistroConfig registro;
  EEPROM.get(0, registro);

  bool registro_valido =
      registro.assinatura == ASSINATURA_CONFIG &&
      registro.versao == VERSAO_CONFIG &&
      registro.crc == calcularCrcRegistro(registro) &&
      valoresDadosPersistidosValidos(registro.dados);

  if (registro_valido) {
    copiarDadosParaConfig(registro.dados);
    return;
  }

  // Migra automaticamente o formato antigo, que gravava Config diretamente
  // no endereco zero e nao possuia assinatura nem CRC.
  Config formato_antigo;
  EEPROM.get(0, formato_antigo);
  if (valoresConfigValidos(formato_antigo)) {
    cfg = formato_antigo;
    salvarConfiguracao();
    Serial.println(F("Configuracao antiga migrada"));
    return;
  }

  cfg = Config();
  salvarConfiguracao();
  Serial.println(F("Configuracao padrao gravada"));
}
