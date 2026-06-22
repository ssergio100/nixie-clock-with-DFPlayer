# Relatório de Desenvolvimento - Rádio Relógio Nixie
**Sessão Concluída em:** 18 de Junho de 2026 (Madrugada)

## 🚀 O que implementamos nesta maratona

1. **Otimização Crítica de RAM (Código Serial Raiz):**
   - Removemos a biblioteca pesada `DFRobotDFPlayerMini` e migramos para o envio direto via Serial pura (`df_send_cmd` com 10 bytes e checksum dinâmico).
   - Aplicamos a macro `F()` em todas as strings de depuração serial.
   - Isso reduziu drasticamente o uso da memória RAM (SRAM) do Arduino Nano, blindando o relógio contra travamentos por estouro de pilha (stack overflow).

2. **Resolução do Conflito de Áudio RDA5807 vs MP3 (Alta Impedância / DHIZ):**
   - **O Problema:** Como a saída de áudio do RDA5807 e do DFPlayer compartilham a mesma entrada do amplificador sem resistores misturadores, o buffer de saída ativo do rádio (mesmo com volume zero ou mutado via `DMUTE`) mantinha baixíssima impedância, aterrando em AC o sinal analógico do MP3.
   - **A Solução:** Ativamos o modo de Alta Impedância do rádio através do bit **DHIZ (Audio Output High-Z Disable)** no Registrador `02h`, chamando `rx.setAudioOutputHighImpedance(true)`.
   - O rádio agora é eletricamente "desconectado" da linha de áudio sempre que está desligado ou quando o alarme MP3 dispara, liberando a linha para o áudio do DFPlayer tocar limpo e alto.
   - Quando o rádio é ligado (via encoder ou alarme), chamamos `rx.setAudioOutputHighImpedance(false)` para reconectar as saídas à linha de áudio.

3. **Sincronização Imediata de Volume:**
   - Ao ligar o rádio ou sair do modo soneca (alarme de rádio), o software faz a leitura física instantânea do potenciômetro no pino A6 e define o volume (`rx.setVolume()`), garantindo que o som comece no nível esperado do seletor físico.

4. **Implementação do Salva Displays (Screen Saver das Nixies):**
   - Implementamos a rotina de proteção das válvulas Nixie contra desgaste desnecessário e *Cathode Poisoning*.
   - O modo Screen Saver desliga todos os dígitos das Nixies ([relogio.ino](file:///C:/Users/sergi/OneDrive/Área de Trabalho/radio_relogio_nixie_encoder/relogio.ino#L4-L11)) e desativa os LEDs de base ([nixie.ino](file:///C:/Users/sergi/OneDrive/Área de Trabalho/radio_relogio_nixie_encoder/nixie.ino#L10)).
   - A ativação/desativação manual ocorre via toque duplo no encoder em [encoder.ino](file:///C:/Users/sergi/OneDrive/Área de Trabalho/radio_relogio_nixie_encoder/encoder.ino#L94-L103), e qualquer movimentação do encoder ou clique acorda o relógio instantaneamente.

---

## 🛠️ O que nos aguarda na próxima bancada

*Toda a pauta principal de programação foi concluída! Circuito de áudio, lógica de alarmes, multiplexação das Nixies e proteção de tela estão 100% integrados em software.*

