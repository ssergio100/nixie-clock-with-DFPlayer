# Estados do Sistema - Rádio Relógio Nixie

Documentação do comportamento do hardware e do encoder rotativo em cada estado do relógio.

### 1. MODO RELÓGIO (Standby/Principal)
* **O que faz:** Estado padrão. Exibe as horas e atualiza o LCD. A multiplexação das Nixies e do LED base roda de forma assíncrona via interrupção do Timer1 a cada **250 µs** em segundo plano, garantindo estabilidade e permitindo o controle de brilho sem tremer.
* **Toque Curto:** Entra no **MODO SETUP** (Emite bipe duplo).
* **Toque Longo:** Liga o rádio e entra no **MODO RÁDIO**.
* **Giro:** Ajusta o **Brilho das Nixies** e do **LED base** de forma síncrona (escala de `1` a `10`). A segunda linha do LCD exibe temporariamente o status (ex: `Brilho: 08/10`) por 2 segundos, e o nível é salvo na EEPROM.

### 2. MODO RÁDIO (Ouvindo FM)
* **O que faz:** O módulo FM está ativo e o áudio é liberado. A multiplexação das Nixies é executada via Timer1 (interrupção rápida a cada 250 µs em máquina de estados) em segundo plano, evitando piscadas e distúrbios de barramento.
* **Toque Curto:** Alterna o foco do giro do encoder entre **Sintonia (SCA)** e **Volume (VOL)**.
* **Toque Longo:** Desliga o rádio e volta ao **MODO RELÓGIO**.
* **Giro (conforme o modo ativo):**
  * **Modo Sintonia (SCA):** Varre as frequências de 0.1 em 0.1 MHz e para na próxima estação forte (com RSSI >= SIGNAL_THRESHOLD). O display Nixie exibe a frequência sintonizada (ex: `106.3`).
  * **Modo Volume (VOL):** Ajusta o volume digital do rádio de `0` (mudo absoluto via software) a `15` (máximo). As válvulas Nixie passam a exibir o sinal de recepção RSSI nas horas (esquerda) e o nível de volume nos minutos (direita). A luz base LED muda para Roxo/Lilás como feedback visual.

### 3. MODO SETUP (Configuração)
* **O que faz:** Interrompe a contagem para edição (Menus 1 a 6). A luz base fica vermelha. A multiplexação das Nixies também roda no Timer1 de forma estável.
* **Toque Curto:** Avança para a próxima tela. Na tela 7, salva tudo na EEPROM/RTC e volta ao **MODO RELÓGIO**.
* **Toque Longo:** Salva todas as alterações imediatamente e sai do modo de configuração, voltando ao **MODO RELÓGIO** (atalho para não precisar passar por todas as telas).
* **Giro:** Incrementa/Decrementa o valor que está sendo editado (Hora, Alarme, Data, etc).

### 4. MODO ALARME (Despertando)
* **O que faz:** A hora do alarme chegou. O Buzzer toca infinito ou o Rádio liga (aplicando o volume digital salvo e usando mute caso configurado em 0).
* **Toque Curto:** Desliga o alarme (cancela por hoje) e volta ao **MODO RELÓGIO**.
* **Toque Longo:** Silencia o áudio e entra no **MODO SONECA**.
* **Giro:** *[Vazio e Bloqueado - Proposital, para evitar desconfigurações acidentais ao tentar desligar de manhã].*

### 5. MODO SONECA ("Bomba Relógio")
* **O que faz:** O alarme pausa por 5 minutos e as Nixies exibem a contagem regressiva.
* **Toque Curto:** Desiste de dormir, cancela o alarme por hoje e volta ao **MODO RELÓGIO**.
* **Toque Longo:** Reseta a soneca devolvendo o contador para os 5 minutos iniciais.
* **Giro:** *[Bloqueado - Mesmo motivo acima].*

---

### 🌟 6. MODO SCREEN SAVER (Salva Displays) -> [A SER IMPLEMENTADO]
* **O que faz:** Após um período de inatividade ou em horário programado, as Nixies desligam ou fazem rodízio de números ("Slot Machine" contínuo) para evitar o desgaste / envenenamento dos filamentos.
* **Qualquer Toque ou Giro:** Cancela a proteção instantaneamente, acorda as telas e volta ao **MODO RELÓGIO**.
