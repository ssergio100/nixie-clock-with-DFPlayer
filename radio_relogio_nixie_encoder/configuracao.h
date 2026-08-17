#pragma once

#include <Arduino.h>

struct DadosConfigPersistidos {
  uint8_t alarme_hora;
  uint8_t alarme_min;
  uint8_t alarme_ligado;
  uint8_t alarme_radio;
  uint8_t alarme_fds;
  uint8_t alarme_som;
  uint8_t alarme_mp3_vol;
  uint8_t chime_som;
  uint8_t chime_volume;
  uint16_t radio_frequencia;
} __attribute__((packed));

struct RegistroConfig {
  uint16_t assinatura;
  uint8_t versao;
  DadosConfigPersistidos dados;
  uint16_t crc;
} __attribute__((packed));

static_assert(sizeof(DadosConfigPersistidos) == 11,
              "Formato persistido de Config foi alterado");
static_assert(sizeof(RegistroConfig) == 16,
              "Layout do registro de EEPROM foi alterado");
