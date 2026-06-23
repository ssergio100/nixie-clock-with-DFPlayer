# Relatório de Desenvolvimento - Rádio Relógio Nixie
**Sessão Concluída em:** 22 de Junho de 2026

## 🚀 O que implementamos nesta maratona

1. **Reestruturação Completa do Setup (Consolidado em 4 Telas):**
   - Agrupamos as configurações do relógio em apenas 4 telas lógicas de setup:
     1. **Horário:** Ajuste de `HH` (passo 0) e `MM` (passo 1).
     2. **Alarme:** Ajuste de Hora (passo 0), Minuto (passo 1), Ativação `Sim`/`Sim/FS`/`Nao` (passo 2) e Tipo de alarme `Fm`/`mp3` (passo 3).
     3. **Data/Ano:** Ajuste de Dia (passo 0), Mês (passo 1) e Ano (passo 2).
     4. **Sons e Volume:** Ajuste de Faixa (`S01`) e Volume do Alarme, seguido de Faixa (`S02`) e Volume do Chime.

2. **Ajuste Bidirecional com Incremento/Decremento:**
   - O encoder rotativo agora suporta incremento (ao girar para a direita) e decremento (ao girar para a esquerda) de forma consistente e unificada em todos os parâmetros, incluindo brilho físico, sintonia de rádio, volume analógico e parâmetros de setup.

3. **Piscamento Seletivo e Sincronizado (LCD & Nixies):**
   - **LCD 16x2:** O parâmetro ativo sob ajuste pisca de forma limpa (apagando os caracteres na fase `OFF` do ciclo). Alinhamos o horário perfeitamente embaixo do indicador `HH:MM` e substituímos o prefixo `F` por `S` (Som) na tela de áudio.
   - **Válvulas Nixie:** O piscamento é segmentado: apenas os dígitos correspondentes ao campo que está sendo editado (horas ou minutos) piscam na fase `OFF` da interrupção do Timer1, enquanto os demais dígitos permanecem estáveis no brilho configurado. Para os ajustes de um único dígito (como ativação ou tipo de alarme), exibe e pisca apenas a válvula correspondente, apagando as outras.

4. **Remoção de Endereços Mágicos da EEPROM:**
   - Criamos constantes nomeadas descritivas no arquivo principal (ex: `EEPROM_ADDR_ALARME_HORA`) para mapear os índices da memória EEPROM, removendo os números numéricos fixos (`0` a `8`) espalhados nas rotinas de gravação e leitura.

---

## 🛠️ O que nos aguarda na próxima bancada

* Toda a interface de usuário por menus físicos e feedbacks visuais está atualizada e otimizada! Realize os testes na bancada com o hardware para validação do comportamento.
