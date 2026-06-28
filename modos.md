# Estados do Sistema e Detalhes de Hardware - Rádio Relógio Nixie

Este documento serve como referência técnica detalhando o funcionamento de cada estado do sistema, a especificação dos componentes e o mapeamento dos pinos do microcontrolador Arduino.

---

## 🕒 Parte 1: Estados do Sistema (Modos de Operação)

### 1. MODO RELÓGIO (Standby/Principal)
* **Comportamento:** Estado padrão do sistema. Exibe as horas reais nas válvulas Nixie (desativando o zero à esquerda da dezena de hora para conservação). O LCD 16x2 traseiro e seu backlight permanecem apagados. A cada 10 segundos (nos segundos finalizados em 8 e 9), o relógio exibe a temperatura ambiente medida pelo sensor LM35 por 2 segundos no formato de 3 dígitos (ex: ` 175` para 17.5 °C), desligando o primeiro tubo. A multiplexação das Nixies e controle do LED base rodam via interrupção assíncrona do Timer1 a cada **250 µs**.
* **Giro do Encoder:** Controla o brilho das Nixies (chamando `ajustarBrilhoNixie(dir)`).
  * Sentido Horário (Direita) aumenta o brilho até `10`.
  * Sentido Anti-horário (Esquerda) reduz o brilho. Em `0` apaga o LED de segundos mas mantém as Nixies no mínimo. Em `-1` apaga o LED de segundos e mantém Nixies no mínimo. Em `-2` desliga totalmente (Nixies e LED base).
  * Se o display estiver em `-2`, ele não acorda com cliques curtos ou giros para a esquerda: acorda somente ao girar para a **direita**.
* **Toque Curto:** Entra no **MODO SETUP** (resposta instantânea, sem delay de detecção de clique duplo).
* **Toque Longo:** Liga o Rádio FM e entra no **MODO RÁDIO**.

### 2. MODO RÁDIO (Ouvindo FM)
* **Comportamento:** O chip FM está ativo e o áudio é direcionado para o alto-falante. O display LCD permanece apagado.
* **Toque Curto:** Alterna ciclicamente o modo de operação do encoder:
  * **Sintonia (SCA):** O encoder ajusta a frequência em passos de 200 kHz (busca automática por RSSI). As Nixies exibem a frequência (ex: `94.7`).
  * **Volume (VOL):** O encoder altera o volume digital (0 a 15). As Nixies mostram a força do sinal (RSSI) na esquerda e o volume na direita.
  * **Relógio (CLK):** Exibe a hora normal. O encoder ajusta o brilho das Nixies (`-2` a `10`).
* **Toque Longo:** Desliga o rádio, desativa o amplificador e retorna ao **MODO RELÓGIO**.

### 3. MODO SETUP (Configuração)
* **Comportamento:** Liga o backlight e o display LCD 16x2 para navegação. A multiplexação das Nixies continua ativa, mas os dígitos correspondentes ao campo em ajuste piscam sincronizadamente.
  * *Observação:* Se o brilho das Nixies estivesse desligado (`0`, `-1` ou `-2`), o Setup força temporariamente um brilho nível `6` para legibilidade.
* **Consolidado em 4 Telas Lógicas:**
  1. **Horário:** Ajuste de Horas (passo 0) e Minutos (passo 1).
  2. **Alarme:** Ajuste de Hora (passo 0), Minuto (passo 1), Ativação (`Sim`, `Sim/FS`, `Nao`) (passo 2) e Tipo (`Fm`, `mp3`) (passo 3).
  3. **Data/Ano:** Ajuste de Dia (passo 0), Mês (passo 1) e Ano (passo 2).
  4. **Sons e Volume:** Ajuste de Faixa (`S01`) e Volume do Alarme, seguido de Faixa (`S02`) e Volume do Chime.
* **Toque Curto:** Avança o campo/passo sob ajuste. Ao passar do último, salva na EEPROM/RTC e sai.
* **Toque Longo:** Grava as configurações imediatamente e retorna ao **MODO RELÓGIO**.
* **Giro do Encoder:** Incrementa ou decrementa o valor ativo.

### 4. MODO ALARME (Despertando)
* **Comportamento:** Dispara no horário configurado tocando FM ou MP3.
  * *Observação:* Se o display estava desligado, ele acorda automaticamente em brilho `6` com LED base de segundos ativo para legibilidade.
* **Toque Curto:** Cancela o alarme por hoje, cessa o som, desliga o amplificador e retorna ao **MODO RELÓGIO** (desligando o display se este estava apagado).
* **Toque Longo:** Silencia o áudio e entra no **MODO SONECA**.

### 5. MODO SONECA ("Snooze")
* **Comportamento:** Pausa o alarme por **5 minutos**. As Nixies exibem a contagem regressiva (Minutos : Segundos) em brilho intermediário `6`.
* **Toque Curto:** Cancela definitivamente o alarme e desliga as Nixies/LED (se estavam desligados).
* **Toque Longo:** Reinicia a contagem da soneca para 5 minutos.

---

## 🔌 Parte 2: Detalhes Técnicos e Pinagem (Hardware)

O projeto é baseado no microcontrolador **ATmega328P** (em placa Arduino Nano ou Uno). A tabela a seguir especifica as conexões e os barramentos utilizados:

### 1. Pinagem do Arduino

| Pino Arduino | Tipo | Recurso / Componente | Função Detalhada |
| :--- | :--- | :--- | :--- |
| **D2** | Entrada (Int0) | Encoder Rotativo - Canal A | Leitura de quadratura do giro (Interrupção externa) |
| **D3** | Entrada (Int1) | Encoder Rotativo - Canal B | Leitura de quadratura do giro (Interrupção externa) |
| **D4** | Entrada Digital | Botão do Encoder | Detecção de clique curto e longo (com resistor pull-up) |
| **D5** | Saída Digital | Decodificador BCD (K155ID1) - A | Bit menos significativo (Bit 0) do valor BCD |
| **D6** | Saída Digital | Decodificador BCD (K155ID1) - B | Bit 1 do valor BCD |
| **D7** | Saída Digital | Decodificador BCD (K155ID1) - C | Bit 2 do valor BCD |
| **D8** | Saída Digital | Decodificador BCD (K155ID1) - D | Bit mais significativo (Bit 3) do valor BCD |
| **D9** | Saída (PWM/Dig) | LED Base (`PINO_LED_BASE`) | Iluminação base e pisca dos segundos (1 Hz) |
| **D10** | Saída Serial | DFPlayer Mini - TX (via SW) | Arduino envia comandos seriais para o módulo MP3 |
| **D11** | Entrada Serial | DFPlayer Mini - RX (via SW) | Arduino recebe dados do módulo MP3 (pino 11 Rx) |
| **D12** | Saída Digital | Ânodo Nixie - Minuto Unidade | Controle de comutação do ânodo do 4º tubo |
| **D13** | Saída Digital | Buzzer Ativo | Saída para aviso de alarmes sonoros |
| **A0 / D14** | Saída Digital | Ânodo Nixie - Hora Dezena | Controle de comutação do ânodo do 1º tubo |
| **A1 / D15** | Saída Digital | Ânodo Nixie - Hora Unidade | Controle de comutação do ânodo do 2º tubo |
| **A2 / D16** | Saída Digital | VCC Amplificador | Chaveamento VCC (HIGH = amplificador OFF, LOW = ON) |
| **A3 / D17** | Saída Digital | Ânodo Nixie - Minuto Dezena | Controle de comutação do ânodo do 3º tubo |
| **A4 / SDA** | Barramento I2C | RTC (DS1307), LCD 16x2, Rádio RDA5807 | Linha de Dados Serial I2C |
| **A5 / SCL** | Barramento I2C | RTC (DS1307), LCD 16x2, Rádio RDA5807 | Linha de Clock Serial I2C |
| **A6** | Entrada Analógica | Sensor de Temperatura (LM35) | Leitura de temperatura ambiente (filtro de média) |

---

### 2. Arquitetura dos Módulos e Funcionamento do Hardware

* **Controle das Válvulas Nixie (Multiplexação Dinâmica):**
  * Para economizar pinos e drivers, os quatro tubos compartilham um único decodificador BCD **K155ID1** (que converte 4 bits binários para 1 de 10 pinos de catodo aterrado, suportando alta tensão).
  * A multiplexação liga apenas um tubo por vez ativando o seu respectivo pino de ânodo correspondente (A0, A1, A3, D12) a cada **3 ms** no total. O ciclo de interrupção do Timer1 executa a cada **250 µs** dividindo o tempo de exibição do tubo em 12 sub-estados para controle fino do brilho por largura de pulso (PWM via software).
* **Barramento I2C (Compartilhado):**
  * **Relógio em Tempo Real (RTC DS1307):** Mantém a hora e data precisas mesmo se o aparelho for desligado da tomada.
  * **Display LCD 16x2 (Endereço I2C 0x27):** Exibe informações de setup no painel traseiro.
  * **Módulo de Rádio RDA5807:** Receptor FM programado via barramento I2C, conectado à linha do amplificador de áudio.
* **Módulo de Áudio (DFPlayer Mini & Amplificador):**
  * A comunicação com o DFPlayer Mini ocorre de forma serial assíncrona nativa a 9600 bps.
  * O amplificador de áudio é chaveado fisicamente pelo pino **A2**. Isso garante que o amplificador seja completamente energizado apenas quando necessário (rádio ativo, alarmes ou prévia de sons no setup), eliminando estalidos ou ruídos estáticos de fundo na tomada durante o repouso.
