# Estados do Sistema - Rádio Relógio Nixie

Documentação do comportamento do hardware e do encoder rotativo em cada estado do relógio.

### 1. MODO RELÓGIO (Standby/Principal)
* **O que faz:** Estado padrão. Exibe as horas nas Nixies. O display LCD de 16x2 e seu backlight permanecem desligados para economizar energia e manter a discrição do relógio. A multiplexação das Nixies e do LED base roda de forma assíncrona via interrupção do Timer1 a cada **250 µs** em segundo plano.
* **Toque Curto:** Entra no **MODO SETUP** (Emite bipe duplo).
* **Toque Longo:** Liga o rádio e entra no **MODO RÁDIO**.
* **Giro:** Ajusta o brilho das Nixies de 1 a 11. Girar para a direita (sentido horário) aumenta o brilho, girar para a esquerda (sentido anti-horário) diminui o brilho. Cada alteração emite um bipe curto de feedback (o brilho roda em RAM e reinicia em 10 ao ligar o relógio).

### 2. MODO RÁDIO (Ouvindo FM)
* **O que faz:** O módulo FM está ativo e o áudio é liberado. O display LCD de 16x2 permanece completamente desligado (sem texto e sem backlight). A multiplexação das Nixies é executada via Timer1 em segundo plano.
* **Toque Curto:** Alterna o foco do giro do encoder entre **Sintonia (SCA)** e **Volume (VOL)**.
* **Toque Longo:** Desliga o rádio e volta ao **MODO RELÓGIO**.
* **Giro (conforme o modo ativo):**
  * **Modo Sintonia (SCA):** Varre as frequências de 0.1 em 0.1 MHz e para na próxima estação forte. O display Nixie exibe a frequência sintonizada (ex: `106.3`).
  * **Modo Volume (VOL):** Ajusta o volume digital do rádio de `0` a `15`. As válvulas Nixie exibem o sinal de recepção RSSI nas horas (esquerda) e o nível de volume nos minutos (direita).

### 3. MODO SETUP (Configuração)
* **O que faz:** Liga o display LCD e seu backlight para exibir e configurar os parâmetros do relógio (Menus de 1 a 8). A contagem das Nixies é interrompida para edição. A multiplexação das Nixies roda no Timer1 de forma estável.
* **Toque Curto:** Avança para a próxima tela. Na tela 8, salva tudo na EEPROM/RTC e volta ao **MODO RELÓGIO**.
* **Toque Longo:** Salva todas as alterações imediatamente e sai do modo de configuração, voltando ao **MODO RELÓGIO** (desligando o LCD).
* **Giro:** Incrementa/Decrementa o valor que está sendo editado (Hora, Alarme, Data, etc).

### 4. MODO ALARME (Despertando)
* **O que faz:** A hora do alarme chegou. O LCD continua desligado. O Buzzer toca ou o Rádio liga.
* **Toque Curto:** Desliga o alarme (cancela por hoje) e volta ao **MODO RELÓGIO**.
* **Toque Longo:** Silencia o áudio e entra no **MODO SONECA**.
* **Giro:** *[Vazio e Bloqueado].*

### 5. MODO SONECA ("Bomba Relógio")
* **O que faz:** O alarme pausa por 5 minutos e as Nixies exibem a contagem regressiva. O LCD continua desligado.
* **Toque Curto:** Desiste de dormir, cancela o alarme por hoje e volta ao **MODO RELÓGIO**.
* **Toque Longo:** Reseta a soneca devolvendo o contador para os 5 minutos iniciais.
* **Giro:** *[Bloqueado].*

---

### 🌟 6. MODO SCREEN SAVER (Salva Displays)
* **O que faz:** Ativado manualmente via **duplo clique rápido (dois toques)** no botão do encoder. Quando ativo, desliga completamente as válvulas Nixie e o LED de base. O LCD já se encontra desligado.
* **Como acordar:** Qualquer interação posterior, seja **girar o encoder** um dente ou dar um **único clique** no botão, acorda o relógio instantaneamente (ligando Nixies e LED base). O primeiro clique de despertar é consumido (bloqueio temporário de 600ms) para evitar comandos acidentais.
* **Como voltar a dormir:** Efetue outro duplo clique rápido no botão do encoder.
