# Manual do Usuário - Rádio Relógio Nixie

Este manual detalha o funcionamento, as configurações e todos os comandos de interação do seu **Rádio Relógio Nixie**. O relógio utiliza um encoder rotativo com botão integrado como único controle físico, oferecendo uma interface limpa e intuitiva.

---

## 🕒 1. Modo Normal (Relógio / Standby)
No estado padrão, o relógio exibe a hora atual nos tubos Nixie. O display LCD e sua luz de fundo (backlight) permanecem completamente desligados para manter a estética clássica e a discrição no escuro.

### Controles no Modo Relógio:
* **Girar o Encoder:** 
  * **Sentido Horário (Direita):** Aumenta o brilho das Nixies (até o limite de 11).
  * **Sentido Anti-horário (Esquerda):** Diminui o brilho das Nixies (até o limite de 1).
  * *Cada ajuste emite um bipe curto (o brilho roda apenas em RAM, reiniciando em 10 ao ligar o relógio).*
* **Clique Curto (1 toque):** Entra no **Modo de Configuração (Setup)**.
* **Toque Longo (Pressionar por 0.7s):** Liga o Rádio FM.
* **Clique Duplo Rápido (2 toques):** Ativa/Desativa o **Salva Displays (Screen Saver)**.
  * *Quando ativo, desliga totalmente os tubos Nixie e a luz de LED base para economizar os filamentos das válvulas.*
  * *Para acordar o relógio, basta girar o encoder ou dar 1 clique curto.*

---

## 📻 2. Modo Rádio (FM)
Ao ligar o rádio, o áudio é direcionado para o amplificador interno e a frequência sintonizada é exibida nas Nixies (ex: `106.3` ou `88.5`). O display LCD continua desligado.

### Controles no Modo Rádio:
* **Clique Curto:** Alterna o controle do encoder entre **Sintonia (SCA)** e **Volume (VOL)**.
  * **Modo Sintonia (SCA):** 
    * Girar o encoder inicia uma busca automática por estações. O rádio é silenciado durante a varredura e o áudio retorna assim que uma estação com sinal forte for encontrada.
  * **Modo Volume (VOL):**
    * Girar o encoder ajusta o volume digital (de 0 a 15).
    * As Nixies exibem a força do sinal RSSI nas horas (à esquerda) e o nível de volume nos minutos (à direita).
    * No volume 0, o rádio é mutado por hardware para eliminar qualquer ruído de fundo.
* **Toque Longo (Pressionar por 0.7s):** Desliga o rádio e retorna ao **Modo Relógio**.

---

## ⚙️ 3. Modo de Configuração (Setup)
Ao entrar no Setup, o display LCD traseiro se acende. As Nixies exibem em pisca-pisca os valores que estão sendo editados.

### Controles no Setup:
* **Clique Curto:** Avança para a próxima tela de configuração. Ao passar pela última tela (Menu 8), salva tudo e sai.
* **Toque Longo:** Salva todas as alterações imediatamente e retorna ao Modo Relógio.
* **Girar o Encoder:** Ajusta o parâmetro da tela atual.

### Telas de Configuração (Menus 1 a 8):

1. **Ajustar Relógio (Ajustar Relogio):**
   * **Girar à Esquerda:** Ajusta a Hora.
   * **Girar à Direita:** Ajusta o Minuto.
2. **Hora do Alarme (Hora do Alarme):**
   * **Girar à Esquerda:** Ajusta a Hora do despertador.
   * **Girar à Direita:** Ajusta o Minuto do despertador.
3. **Modo do Alarme (Modo do Alarme):**
   * **Girar à Esquerda:** Alterna a fonte do despertador entre **RADIO** ou **MP3** (cartão Micro SD).
   * **Girar à Direita:** Alterna o status do alarme entre **ON** (Ligado) ou **OFF** (Desligado).
4. **Ajuste Data (Ajuste Data: D/M):**
   * **Girar à Esquerda:** Ajusta o Dia.
   * **Girar à Direita:** Ajusta o Mês.
5. **Ajuste Ano (Ajuste Ano):**
   * **Girar o encoder:** Ajusta o Ano (de 2000 a 2099).
6. **Alarme Fim de Semana (Alarme Fim de S.):**
   * **Girar o encoder:** Alterna entre **LIGADO** (o alarme tocará todos os dias) ou **DESLIGADO** (o alarme não tocará aos sábados e domingos).
7. **Som e Volume do Alarme (Som Alarme / Vol Alarme):**
   * **Girar à Esquerda:** Escolhe a faixa do cartão SD que tocará no alarme (emite prévia).
   * **Girar à Direita:** Ajusta o volume do alarme MP3 (escala de 0 a 30).
8. **Som e Volume da Hora Cheia (Som Hora / Vol Hora):**
   * **Girar à Esquerda:** Escolhe a faixa do cartão SD para o chime/aviso de hora cheia (emite prévia).
   * **Girar à Direita:** Ajusta o volume do chime (escala de 0 a 30).

---

## 🔔 4. Funcionamento do Alarme (Despertador)
Quando o horário do alarme chega, o relógio desperta tocando a rádio FM ou a faixa MP3 escolhida.

* **Desligar Alarme (Clique Curto):** Cancela o alarme por hoje. O som para de tocar e o relógio volta ao modo standby normal.
* **Modo Soneca / Snooze (Toque Longo):** 
  * Pausa o alarme por **5 minutos**.
  * Durante a soneca, as Nixies exibem uma contagem regressiva de minutos e segundos (ex: `04:59`).
  * Para cancelar a soneca a qualquer momento e desligar o alarme, dê um **clique curto**.
  * Para reiniciar o cronômetro da soneca de volta para 5 minutos, dê outro **toque longo**.

---

## 💎 5. Proteção e Cuidados
* **Efeito Caça-Níqueis (Anti-Cathode Poisoning):** A cada 15 minutos, no segundo zero, os dígitos giram rapidamente. O efeito dura 4 segundos no total: nos primeiros 3 segundos todos os dígitos embaralham de forma aleatória e, em seguida, os dígitos travam um a um (da esquerda para a direita) a cada 1/3 de segundo até exibir a hora correta. Esse processo ajuda a evitar o envenenamento de catodo das válvulas Nixie.
* **Salva Displays (Madrugada):** Recomenda-se ativar o *Salva Displays* (duplo clique) antes de dormir. Desligar os filamentos quando o relógio não está em uso evita o desgaste prematuro e o escurecimento do vidro dos tubos.
