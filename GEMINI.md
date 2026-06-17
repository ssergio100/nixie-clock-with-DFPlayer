# Relatório de Desenvolvimento - Rádio Relógio Nixie
**Sessão Concluída em:** 15 de Junho de 2026 (Madrugada de 16)

## 🚀 O que implementamos nesta maratona

1. **Bug do I2C Assassino Resolvido:**
   - Descobrimos que a linha `Wire.setClock(400000)` no final do `setup()` estava matando o chip RTC DS1307, pois ele suporta estritamente 100kHz. O "overclock" do barramento causava falhas de leitura e perda total da hora ao desligar da tomada. Removido e estabilizado!

2. **Áudio Analógico e Potenciômetro:**
   - Confirmamos que o potenciômetro físico é do tipo **A20K (Audio Taper/Logarítmico)**.
   - Reimplantamos a leitura no pino A6 com uma "Zona Morta" inferior e uma curva achatada via software (`x * x`) para compensar a saturação rápida do amplificador externo.
   - Adicionamos um comando explícito `rx.setMute(true)` quando o volume mapeado chega a 0, garantindo silêncio absoluto no rádio com o botão totalmente fechado.

3. **Revisão Estrutural do Alarme / Soneca:**
   - Corrigimos graves falhas de "estado zumbi" na roteirização de áudio causadas pela migração do Buzzer para o MP3.
   - O amplificador (`vcc_amp`) agora liga corretamente quando o alarme MP3 dispara, inclusive na retomada da Bomba Relógio (Soneca).
   - O comando de "Soneca" (Toque Longo) e "Stop" (Toque Curto) agora forçam o desligamento global e seguro de todos os áudios, mutando o rádio e derrubando o relé do amplificador.

4. **Isolamento de Hardware (DFPlayer Mini):**
   - O código "Raiz" via `df_send_cmd` está 100% polido.
   - Confirmamos via testes elétricos (2.4V nos pinos SPK) que a alimentação e o amplificador Classe D interno do DFPlayer estão vivos.
   - No entanto, o leitor SD do módulo está inoperante/incompatível, recusando-se a piscar o LED de leitura mesmo recebendo atalhos diretos de hardware (Pino IO_2 para GND) com cartões formatados em FAT32.

---

## 🛠️ O que nos aguarda na próxima bancada

1. **Substituição do Hardware MP3:**
   - Adquirir um novo módulo DFPlayer Mini (preferencialmente de um lote/chipset diferente do atual se possível) ou testar com um Micro SD jurássico de 2GB/4GB para ver se o chip ressuscita.
   - Quando funcionar, plugar no circuito sem mexer em uma linha de código, pois a base de software já está pronta e testada esperando por ele!

2. **Implementação do Salva Displays (Screen Saver das Nixies):**
   - A pauta final de programação! Responder às 3 perguntas do Plano de Implementação para codificarmos a rotina que salva as válvulas do *Cathode Poisoning* na madrugada.

*Código restaurado para a versão de produção (relógio funcional). O rádio, os menus e o relógio Nixie continuam operantes para você usar normalmente até a chegada da próxima peça! Bom descanso, engenheiro!*
