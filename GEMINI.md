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

---

## 🛠️ O que nos aguarda na próxima bancada

1. **Implementação do Salva Displays (Screen Saver das Nixies):**
   - A última pauta de programação pendente! Vamos criar a rotina de proteção das válvulas Nixie contra o *Cathode Poisoning* (envenenamento de catodo) durante a madrugada, rotacionando ou desligando os dígitos em horários pré-definidos.

*Circuito de áudio e lógica de alarmes 100% integrados em software! O MP3 e a rádio FM agora dividem amigavelmente o mesmo amplificador sem interferências mútuas.*
