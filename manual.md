# Manual do Usuário - Rádio Relógio Nixie

Este manual detalha o funcionamento, as configurações e todos os comandos de interação do seu **Rádio Relógio Nixie**. O relógio utiliza um encoder rotativo com botão integrado como único controle físico, oferecendo uma interface limpa, intuitiva e de rápida resposta.

---

## 🕒 1. Modo Normal (Relógio / Standby)
No estado padrão, o relógio exibe a hora atual nos tubos Nixie. O display LCD traseiro e sua luz de fundo (backlight) permanecem completamente desligados para manter a estética clássica e a discrição no escuro.

### Ajuste de Brilho e Modos de Desligamento:
Girar o encoder rotativo no sentido horário (direita) ou anti-horário (esquerda) ajusta o nível de brilho do display. O nível atual varia de `-2` a `10`:
* **Nível `1` a `10`:** Brilho normal ativo das Nixies (nível 10 é o máximo). O LED de segundos pisca normalmente.
* **Nível `0` (Mínimo):** Mantém as Nixies no brilho mínimo, com o LED de segundos piscando normalmente.
* **Nível `-1` (Mínimo sem LED):** Mantém as Nixies no brilho mínimo, mas desliga totalmente o LED de segundos (`PINO_LED_BASE`).
* **Nível `-2` (Totalmente Desligado):** Apaga completamente tanto as Nixies quanto o LED de segundos.
  * *Observação:* O estado totalmente desligado (`-2`) não acorda por cliques ou movimentos para a esquerda no encoder. Para ligar novamente o display, basta girar o encoder para a **direita** (sentido horário).

### Exibição de Temperatura:
* **Leitura Automática:** A cada 10 segundos (nos segundos finalizados em 8 e 9), o relógio exibe a temperatura ambiente medida pelo sensor LM35 nos três dígitos da direita das Nixies por 2 segundos (ex: ` 175` para 17.5 °C). O primeiro tubo permanece apagado.
* *Observação:* A temperatura não será exibida se o display estiver no modo totalmente desligado (nível `-2`), de forma a preservar o repouso completo das válvulas e do LED.

### Outros Controles no Modo Relógio:
* **Clique Curto (1 toque):** Entra no **Modo de Configuração (Setup)**. A resposta do clique é instantânea.
* **Toque Longo (Pressionar por 0.7s):** Liga o Rádio FM.

---

## 📻 2. Modo Rádio (FM)
Ao ligar o rádio, o áudio é direcionado para o amplificador interno e a frequência sintonizada é exibida nas Nixies (ex: `94.7` ou `106.3`). O display LCD continua desligado.

### Controles no Modo Rádio:
* **Clique Curto:** Alterna ciclicamente o modo de sintonia do rádio:
  * **Sintonia (SCA):** Girar o encoder inicia uma busca automática por estações. O rádio silencia durante a busca e o áudio retorna ao sintonizar uma estação.
  * **Volume (VOL):** Girar o encoder ajusta o volume digital (0 a 15). O display Nixie exibe a força do sinal RSSI nas horas (esquerda) e o nível de volume nos minutos (direita). No volume 0, o rádio entra em mute analógico.
  * **Relógio (CLK):** Exibe a hora normal nas Nixies enquanto a rádio continua tocando. Girar o encoder ajusta o brilho das Nixies (escala de `-2` a `10`).
* **Toque Longo (Pressionar por 0.7s):** Desliga o rádio e retorna ao **Modo Relógio**.

---

## ⚙️ 3. Modo de Configuração (Setup)
Ao entrar no Setup, o display LCD traseiro se acende e exibe as opções. As Nixies piscam de forma selecionada e sincronizada nos campos sob edição.
* *Caso o display estivesse configurado em nível de brilho desligado/mínimo (`0`, `-1` ou `-2`), o Setup força temporariamente um brilho de nível `6` para legibilidade, retornando ao brilho original após sair.*

### Controles no Setup:
* **Clique Curto:** Avança o campo/passo de ajuste da tela atual. Ao passar pelo último campo, avança para a próxima tela de configuração.
* **Toque Longo:** Salva todas as alterações imediatamente (grava na EEPROM), sincroniza com o RTC e retorna ao Modo Relógio.
* **Girar o Encoder:** Ajusta o valor do parâmetro que está piscando no momento (suporta incremento e decremento).

### Telas de Configuração (Consolidadas em 4 Telas):

1. **Horário:**
   * **Passo 0 (`HH`):** Ajusta as Horas.
   * **Passo 1 (`MM`):** Ajusta os Minutos.
2. **Alarme:**
   * **Passo 0:** Ajusta a Hora do alarme.
   * **Passo 1:** Ajusta o Minuto do alarme.
   * **Passo 2:** Configura ativação do alarme: `Sim` (diário), `Sim/FS` (somente fim de semana) ou `Nao` (desativado).
   * **Passo 3:** Configura o tipo do alarme: `Fm` (rádio) ou `mp3` (músicas no cartão SD).
3. **Data/Ano:**
   * **Passo 0:** Ajusta o Dia.
   * **Passo 1:** Ajusta o Mês.
   * **Passo 2:** Ajusta o Ano.
4. **Sons e Volume:**
   * **Passo 0:** Escolhe a faixa do alarme (`S01`) (emite prévia).
   * **Passo 1:** Ajusta o volume do alarme.
   * **Passo 2:** Escolhe a faixa do chime/hora cheia (`S02`) (emite prévia).
   * **Passo 3:** Ajusta o volume do chime.

---

## 🔔 4. Funcionamento do Alarme (Despertador)
Quando o horário configurado chega, o alarme toca usando a rádio FM ou a faixa MP3 escolhida.
* *Se o display estiver desligado (`-2` ou `-1`), ele acorda automaticamente no nível `6` de brilho com o LED de segundos ativo para permitir visualização rápida das horas.*

* **Desligar Alarme (Clique Curto):** Encerra o alarme por completo. O som para de tocar, o amplificador é desligado e o display retorna ao nível de brilho original programado (se estava em `-2`, ele volta a se apagar completamente).
* **Modo Soneca / Snooze (Toque Longo):**
   * Pausa o alarme por **5 minutos**.
   * Durante a soneca, as Nixies exibem a contagem regressiva de minutos e segundos (ex: `04:59`).
   * Para cancelar a soneca a qualquer momento, dê um **clique curto**.
   * Para reiniciar o cronômetro da soneca para 5 minutos, dê um **toque longo**.

---

## 💎 5. Proteção e Cuidados
* **Efeito Caça-Níqueis (Anti-Cathode Poisoning):** A cada 15 minutos, no segundo zero, os dígitos giram rapidamente. O efeito dura 2 segundos no total: os dígitos embaralham de forma rápida e travam um a um (da esquerda para a direita) para evitar o envenenamento de cátodo e prolongar a vida útil das válvulas Nixie.
