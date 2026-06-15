# Estados do Sistema - Rádio Relógio Nixie

Documentação do comportamento do hardware e do encoder rotativo em cada estado do relógio.

### 1. MODO RELÓGIO (Standby/Principal)
* **O que faz:** Estado padrão. Exibe as horas e atualiza o LCD.
* **Toque Curto:** Entra no **MODO SETUP** (Emite bipe duplo).
* **Toque Longo:** Liga o rádio e entra no **MODO RÁDIO**.
* **Giro:** *[Vazio - Atualmente não faz nada. Sugestão: Ajustar o brilho geral das válvulas ou trocar a cor do LED RGB base?]*

### 2. MODO RÁDIO (Ouvindo FM)
* **O que faz:** O módulo FM está ativo e o áudio é liberado.
* **Toque Curto:** Alterna o "sub-modo" do giro (`f_step`): *Navegar nas Memórias -> Busca Fina de Estação -> Exibir Hora*.
* **Toque Longo:** Desliga o rádio e volta ao **MODO RELÓGIO**.
* **Giro:** Muda de estação ou de memória (conforme o sub-modo acima).

### 3. MODO SETUP (Configuração)
* **O que faz:** Interrompe a contagem para edição (Menus 1 a 6). A luz base fica vermelha.
* **Toque Curto:** Avança para a próxima tela. Na tela 7, salva tudo na EEPROM/RTC e volta ao **MODO RELÓGIO**.
* **Toque Longo:** Salva todas as alterações imediatamente e sai do modo de configuração, voltando ao **MODO RELÓGIO** (atalho para não precisar passar por todas as telas).
* **Giro:** Incrementa/Decrementa o valor que está sendo editado (Hora, Alarme, Data, etc).

### 4. MODO ALARME (Despertando)
* **O que faz:** A hora do alarme chegou. O Buzzer toca infinito ou o Rádio liga.
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
