<script setup>
import { useNixieClock } from './useNixieClock'

const {
  mode,
  radioMode,
  hourVal,
  minuteVal,
  secondVal,
  alarmEnabled,
  alarmRinging,
  inSnooze,
  snoozeTimeLeft,
  brightnessLabel,
  radioFrequency,
  currentRssi,
  temperatureC,
  timeMultiplier,
  serialLogs,
  muteBeep,
  isSlotMachine,
  isScanning,
  chimeActive,
  isPressed,
  knobRotation,
  lcdLine1,
  lcdLine2,
  nixieDigits,
  showColon,
  baseLedGlow,
  displayFullyOff,
  nixieIntensity,
  audioActive,
  setupLabel,
  addLog,
  beep,
  triggerAlarm,
  triggerChime,
  triggerSlotMachine,
  setTimeMultiplier,
  toggleAlarmQuick,
  clearLogs,
  handleButtonDown,
  handleButtonUp,
  handleButtonCancel,
  handleKnobStart,
  handleWheel
} = useNixieClock()
</script>

<template>
  <main class="app-shell">
    <div class="simulator-wrapper">
      
      <!-- CABINE DO RELÓGIO (Chassi físico) -->
      <section class="clock-cabinet">
        <div class="wood-top"></div>
        <div class="cabinet-body">
          
          <!-- Leds Traseiros e Decoração de Brilho -->
          <div class="ambient-glow" :class="{ 'led-on': baseLedGlow, 'color-radio': mode === 'radio' }"></div>
          
          <div class="panel-front">
            <div class="panel-header">
              <span class="brand-logo">NIXIE CHRONOS</span>
              <div class="led-indicator" :class="{ 'active': alarmEnabled }"></div>
            </div>

            <!-- Grade das Válvulas Nixies -->
            <div class="nixie-cradle">
              <div class="nixie-glass-area" :style="{ '--nixie-intensity': nixieIntensity }">
                
                <!-- Tubo 1 -->
                <div class="nixie-tube" :class="{ 'tube-off': nixieDigits[0] === -1 }">
                  <div class="glass-reflection"></div>
                  <div class="grid-mesh"></div>
                  <div class="nixie-glow" :class="{ 'active-glow': nixieDigits[0] !== -1 && !displayFullyOff }"></div>
                  <div class="filaments-container">
                    <span v-for="n in 10" :key="n-1" class="filament-digit" 
                          :class="{ 'glowing': nixieDigits[0] === n-1, 'inactive': nixieDigits[0] !== n-1 }">
                      {{ n-1 }}
                    </span>
                  </div>
                </div>

                <!-- Tubo 2 -->
                <div class="nixie-tube" :class="{ 'tube-off': nixieDigits[1] === -1 }">
                  <div class="glass-reflection"></div>
                  <div class="grid-mesh"></div>
                  <div class="nixie-glow" :class="{ 'active-glow': nixieDigits[1] !== -1 && !displayFullyOff }"></div>
                  <div class="filaments-container">
                    <span v-for="n in 10" :key="n-1" class="filament-digit" 
                          :class="{ 'glowing': nixieDigits[1] === n-1, 'inactive': nixieDigits[1] !== n-1 }">
                      {{ n-1 }}
                    </span>
                  </div>
                </div>

                <!-- Separador Neon (Colon) -->
                <div class="nixie-separator">
                  <div class="neon-dot" :class="{ 'lit': showColon }"></div>
                  <div class="neon-dot" :class="{ 'lit': showColon }"></div>
                </div>

                <!-- Tubo 3 -->
                <div class="nixie-tube" :class="{ 'tube-off': nixieDigits[2] === -1 }">
                  <div class="glass-reflection"></div>
                  <div class="grid-mesh"></div>
                  <div class="nixie-glow" :class="{ 'active-glow': nixieDigits[2] !== -1 && !displayFullyOff }"></div>
                  <div class="filaments-container">
                    <span v-for="n in 10" :key="n-1" class="filament-digit" 
                          :class="{ 'glowing': nixieDigits[2] === n-1, 'inactive': nixieDigits[2] !== n-1 }">
                      {{ n-1 }}
                    </span>
                  </div>
                </div>

                <!-- Tubo 4 -->
                <div class="nixie-tube" :class="{ 'tube-off': nixieDigits[3] === -1 }">
                  <div class="glass-reflection"></div>
                  <div class="grid-mesh"></div>
                  <div class="nixie-glow" :class="{ 'active-glow': nixieDigits[3] !== -1 && !displayFullyOff }"></div>
                  <div class="filaments-container">
                    <span v-for="n in 10" :key="n-1" class="filament-digit" 
                          :class="{ 'glowing': nixieDigits[3] === n-1, 'inactive': nixieDigits[3] !== n-1 }">
                      {{ n-1 }}
                    </span>
                  </div>
                </div>

              </div>
              <!-- Base Metálica dos tubos -->
              <div class="cradle-base"></div>
            </div>

            <!-- Grade de Alto-Falante / Som ativo -->
            <div class="speaker-grill">
              <div class="grill-line" v-for="i in 8" :key="i" 
                   :class="{ 'bounce': audioActive }"></div>
            </div>

            <!-- Controle do Encoder Físico -->
            <div class="encoder-casing">
              <div class="dial-indicators">
                <span :class="{ 'highlight': mode === 'radio' && radioMode === 'tune' }">SCA</span>
                <span :class="{ 'highlight': mode === 'radio' && radioMode === 'volume' }">VOL</span>
                <span :class="{ 'highlight': mode === 'radio' && radioMode === 'clock' }">CLK</span>
              </div>
              <div class="encoder-body">
                <div class="encoder-ring"
                     :style="{ transform: `rotate(${knobRotation}deg)` }"
                     @wheel.prevent="handleWheel"
                     @pointerdown="handleKnobStart">
                  <div class="knob-grooves"></div>
                  <div class="encoder-notch"></div>
                  <button class="encoder-center" 
                          :class="{ 'pressed': isPressed }"
                          @pointerdown.prevent.stop="handleButtonDown"
                          @pointerup.prevent.stop="handleButtonUp"
                          @pointercancel.prevent.stop="handleButtonCancel">
                    <span>SET</span>
                  </button>
                </div>
              </div>
              <div class="encoder-hint">Gire o anel para ajustar • Pressione o centro</div>
            </div>

          </div>
        </div>
        <div class="wood-bottom"></div>
      </section>

      <!-- PAINEL TRASEIRO (Display LCD Traseiro) -->
      <section class="lcd-back-panel">
        <div class="panel-tag">PAINEL TRASEIRO</div>
        <div class="lcd-display-frame">
          <div class="lcd-screen" :class="{ 'backlight-on': mode === 'setup' }">
            <div class="lcd-text-line">{{ lcdLine1 }}</div>
            <div class="lcd-text-line">{{ lcdLine2 }}</div>
          </div>
        </div>
        <div class="lcd-note">A luz azul de fundo só se acende durante o Setup físico.</div>
      </section>

      <!-- CONSOLE SERIAL E PAINEL DE CONTROLE DO SIMULADOR -->
      <section class="simulation-dashboard">
        <h2>Painel do Desenvolvedor & Depuração Serial</h2>
        
        <div class="sim-controls-grid">
          <div class="control-card">
            <h3>Controle de Tempo</h3>
            <div class="time-adjusters">
              <button @click="setTimeMultiplier(1)" :class="{ 'active': timeMultiplier === 1 }">1x (Real)</button>
              <button @click="setTimeMultiplier(60)" :class="{ 'active': timeMultiplier === 60 }">60x (Min/s)</button>
              <button @click="setTimeMultiplier(3600)" :class="{ 'active': timeMultiplier === 3600 }">3600x (H/s)</button>
            </div>
            <div class="clock-values-view">
              Hora RTC: <span>{{ String(hourVal).padStart(2, '0') }}:{{ String(minuteVal).padStart(2, '0') }}:{{ String(secondVal).padStart(2, '0') }}</span>
            </div>
          </div>

          <div class="control-card">
            <h3>Disparos & Eventos</h3>
            <div class="event-buttons">
              <button @click="triggerAlarm(false)" class="danger-btn">Forçar Disparo Alarme</button>
              <button @click="triggerChime(true)" class="warn-btn">Testar Chime</button>
              <button @click="triggerSlotMachine" class="info-btn">Girar Caça-Níqueis</button>
              <button @click="beep(1000, 150)" class="action-btn">Manual Beep</button>
            </div>
          </div>

          <div class="control-card">
            <h3>Ajustes Rápidos</h3>
            <div class="quick-settings">
              <label>
                <input type="checkbox" :checked="alarmEnabled" @change="toggleAlarmQuick" />
                Ativar Alarme
              </label>
              <label>
                <input type="checkbox" v-model="muteBeep" />
                Buzzer em mudo
              </label>
              <label>
                Temperatura LM35
                <input class="quick-number" type="number" min="0" max="99.9" step="0.1" v-model.number="temperatureC" />
                °C
              </label>
            </div>
            <div class="state-pills">
              <span class="pill" :class="{ 'active': displayFullyOff }">{{ brightnessLabel }}</span>
              <span class="pill" :class="{ 'active': mode === 'setup' }">{{ mode === 'setup' ? setupLabel : `Modo ${mode === 'radio' ? radioMode.toUpperCase() : 'RELOGIO'}` }}</span>
              <span class="pill" :class="{ 'active': isScanning }">Busca FM</span>
              <span class="pill" :class="{ 'active': chimeActive }">Chime</span>
              <span class="pill" :class="{ 'active': alarmRinging }">Tocando</span>
              <span class="pill" :class="{ 'active': inSnooze }">Soneca</span>
            </div>
          </div>
        </div>

        <div class="serial-terminal">
          <div class="terminal-header">
            <span>Terminal Serial (9600 baud)</span>
            <button @click="clearLogs">Limpar Logs</button>
          </div>
          <div class="terminal-body">
            <div v-for="(log, idx) in serialLogs" :key="idx" class="log-line">{{ log }}</div>
            <div v-if="serialLogs.length === 0" class="log-empty">Nenhum log gerado.</div>
          </div>
        </div>

      </section>

    </div>
  </main>
</template>

<style scoped>
/* ==========================================
   Importação de fontes e Variáveis
   ========================================== */
@import url('https://fonts.googleapis.com/css2?family=Fira+Mono:wght@400;700&family=Share+Tech+Mono&display=swap');

.app-shell {
  min-height: 100vh;
  background-color: #0b0d11;
  color: #e2e8f0;
  display: flex;
  justify-content: center;
  align-items: center;
  padding: 40px 20px;
}

.simulator-wrapper {
  width: 100%;
  max-width: 900px;
  display: flex;
  flex-direction: column;
  gap: 32px;
}

/* ==========================================
   Chassi e Cabine do Relógio (Visual Físico)
   ========================================== */
.clock-cabinet {
  background: linear-gradient(180deg, #1f140e 0%, #150d09 100%);
  border-radius: 20px;
  box-shadow: 
    0 25px 60px rgba(0, 0, 0, 0.8),
    inset 0 2px 2px rgba(255, 255, 255, 0.1),
    inset 0 -2px 10px rgba(0,0,0,0.6);
  border: 1px solid #2e1c12;
  position: relative;
  overflow: hidden;
  padding: 10px;
}

.wood-top {
  height: 10px;
  background: linear-gradient(90deg, #3a251a 0%, #4c3224 50%, #3a251a 100%);
  border-radius: 12px 12px 0 0;
  border-bottom: 2px solid #150d09;
}

.wood-bottom {
  height: 12px;
  background: linear-gradient(90deg, #150d09 0%, #2e1c12 50%, #150d09 100%);
  border-radius: 0 0 12px 12px;
  border-top: 2px solid #0f0a06;
}

.cabinet-body {
  background: radial-gradient(circle at center, #1b202a 0%, #0d1017 100%);
  border: 2px solid #07090d;
  margin: 4px;
  padding: 24px;
  border-radius: 10px;
  position: relative;
}

.ambient-glow {
  position: absolute;
  top: 10px;
  left: 10%;
  right: 10%;
  height: 150px;
  background: radial-gradient(circle, rgba(255, 115, 0, 0.18) 0%, rgba(255, 115, 0, 0) 70%);
  z-index: 1;
  pointer-events: none;
  transition: background 0.6s ease;
}

.ambient-glow.led-on {
  background: radial-gradient(circle, rgba(255, 106, 0, 0.35) 0%, rgba(255, 106, 0, 0) 70%);
}

.ambient-glow.led-on.color-radio {
  background: radial-gradient(circle, rgba(0, 162, 255, 0.28) 0%, rgba(0, 162, 255, 0) 70%);
}

.panel-front {
  position: relative;
  z-index: 2;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.panel-header {
  width: 100%;
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0 10px;
  margin-bottom: 24px;
}

.brand-logo {
  font-family: 'Share Tech Mono', monospace;
  font-size: 0.8rem;
  letter-spacing: 0.4em;
  color: #5d7290;
}

.led-indicator {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background-color: #242c38;
  border: 1px solid #10151c;
  box-shadow: inset 0 1px 1px rgba(0,0,0,0.8);
  transition: all 0.3s;
}

.led-indicator.active {
  background-color: #ff3c00;
  box-shadow: 
    0 0 8px #ff3c00,
    0 0 15px #ff5500;
}

/* ==========================================
   Válvulas Nixie Tubes (3D Look)
   ========================================== */
.nixie-cradle {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.nixie-glass-area {
  display: flex;
  justify-content: center;
  align-items: flex-end;
  gap: 16px;
  padding: 10px 20px 0;
}

.nixie-tube {
  width: 90px;
  height: 180px;
  background: linear-gradient(90deg, rgba(255, 255, 255, 0.03) 0%, rgba(255, 255, 255, 0.08) 50%, rgba(0,0,0,0.5) 100%);
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-top-left-radius: 45px;
  border-top-right-radius: 45px;
  border-bottom-left-radius: 8px;
  border-bottom-right-radius: 8px;
  position: relative;
  overflow: hidden;
  box-shadow:
    0 -10px 30px rgba(0,0,0,0.8),
    inset 0 15px 30px rgba(255, 255, 255, 0.1),
    inset -5px 0 15px rgba(0,0,0,0.6);
  transition: opacity 0.5s;
}

.nixie-tube.tube-off {
  box-shadow: 
    inset 0 15px 30px rgba(255,255,255,0.02),
    inset -5px 0 15px rgba(0,0,0,0.9);
}

.glass-reflection {
  position: absolute;
  top: 5px;
  left: 12px;
  width: 14px;
  height: 90%;
  background: linear-gradient(90deg, rgba(255, 255, 255, 0.15) 0%, rgba(255, 255, 255, 0) 100%);
  border-radius: 10px;
  pointer-events: none;
}

.grid-mesh {
  position: absolute;
  top: 30px;
  bottom: 10px;
  left: 10px;
  right: 10px;
  border: 1px solid rgba(255, 255, 255, 0.04);
  background-image: 
    linear-gradient(rgba(255, 255, 255, 0.03) 1px, transparent 1px),
    linear-gradient(90deg, rgba(255, 255, 255, 0.03) 1px, transparent 1px);
  background-size: 3px 3px;
  opacity: 0.8;
  z-index: 1;
}

.nixie-glow {
  position: absolute;
  bottom: 0;
  left: 15%;
  right: 15%;
  height: 50px;
  background: radial-gradient(circle, rgba(255, 81, 0, 0.12) 0%, rgba(255, 81, 0, 0) 70%);
  opacity: 0;
  transition: opacity 0.3s;
  z-index: 2;
}

.active-glow {
  opacity: var(--nixie-intensity, 1);
}

.filaments-container {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  display: grid;
  place-items: center;
  z-index: 3;
}

.filament-digit {
  grid-area: 1 / 1 / 2 / 2;
  font-family: 'Courier New', monospace;
  font-size: 5rem;
  font-weight: 700;
  user-select: none;
  transition: all 0.1s ease;
}

.filament-digit.inactive {
  color: rgba(255, 255, 255, 0.035);
  text-shadow: none;
}

.filament-digit.glowing {
  color: #ff9142;
  text-shadow: 
    0 0 3px #ff3c00,
    0 0 8px #ff5100,
    0 0 16px #ff7c00,
    0 0 30px rgba(255, 81, 0, 0.8);
  z-index: 10;
  opacity: var(--nixie-intensity, 1);
}

/* Pontos separadores de neon */
.nixie-separator {
  display: flex;
  flex-direction: column;
  justify-content: center;
  gap: 30px;
  height: 160px;
  width: 20px;
  align-items: center;
}

.neon-dot {
  width: 8px;
  height: 8px;
  background-color: rgba(255, 255, 255, 0.03);
  border-radius: 50%;
  box-shadow: inset 0 1px 1px rgba(0,0,0,0.8);
  transition: background-color 0.1s;
}

.neon-dot.lit {
  background-color: #ff6a00;
  box-shadow: 
    0 0 5px #ff3c00,
    0 0 12px #ff6a00,
    0 0 20px #ffb300;
}

.cradle-base {
  width: 100%;
  height: 14px;
  background: linear-gradient(180deg, #10141d 0%, #1c2333 100%);
  border-top: 1px solid #334057;
  border-bottom: 2px solid #080a0e;
  border-radius: 4px;
}

/* ==========================================
   Grade do Alto-Falante (Simulador visual)
   ========================================== */
.speaker-grill {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 6px;
  height: 20px;
  margin: 18px 0;
}

.grill-line {
  width: 3px;
  height: 6px;
  background-color: #1a222e;
  border-radius: 2px;
  transition: height 0.1s ease;
}

.grill-line.bounce {
  animation: audioBounce 0.6s infinite alternate;
}

.grill-line.bounce:nth-child(2n) { animation-delay: 0.15s; }
.grill-line.bounce:nth-child(3n) { animation-delay: 0.3s; }
.grill-line.bounce:nth-child(4n) { animation-delay: 0.05s; }

@keyframes audioBounce {
  0% { height: 6px; background-color: #1a222e; }
  100% { height: 16px; background-color: #ff8c00; }
}

/* ==========================================
   Capa do Encoder e Botão Físico
   ========================================== */
.encoder-casing {
  display: flex;
  flex-direction: column;
  align-items: center;
  margin-top: 10px;
}

.dial-indicators {
  display: flex;
  justify-content: space-between;
  width: 180px;
  margin-bottom: 8px;
}

.dial-indicators span {
  font-family: 'Share Tech Mono', monospace;
  font-size: 0.72rem;
  color: #42526c;
  letter-spacing: 0.1em;
  transition: color 0.3s;
}

.dial-indicators span.highlight {
  color: #ffa600;
  text-shadow: 0 0 6px rgba(255, 166, 0, 0.4);
}

.encoder-body {
  width: 140px;
  height: 140px;
  background: radial-gradient(circle, #0e1218 0%, #06080b 100%);
  border-radius: 50%;
  border: 4px solid #1a2330;
  box-shadow: 
    0 10px 20px rgba(0,0,0,0.6),
    inset 0 2px 5px rgba(0,0,0,0.8);
  display: flex;
  justify-content: center;
  align-items: center;
}

.encoder-ring {
  width: 110px;
  height: 110px;
  border-radius: 50%;
  background: radial-gradient(circle, #2d3b4e 0%, #151e2b 70%, #0d131b 100%);
  border: 2px solid #3d5069;
  position: relative;
  cursor: grab;
  box-shadow: 
    0 4px 8px rgba(0,0,0,0.5),
    inset 0 1px 1px rgba(255,255,255,0.1);
  display: flex;
  justify-content: center;
  align-items: center;
}

.encoder-ring:active {
  cursor: grabbing;
}

.knob-grooves {
  position: absolute;
  width: 96px;
  height: 96px;
  border-radius: 50%;
  border: 2px dashed rgba(255,255,255,0.06);
  pointer-events: none;
}

.encoder-notch {
  position: absolute;
  top: 8px;
  width: 6px;
  height: 18px;
  background: linear-gradient(180deg, #ffc87c 0%, #ffa600 100%);
  border-radius: 3px;
  box-shadow: 0 0 6px rgba(255, 166, 0, 0.7);
  pointer-events: none;
}

.encoder-center {
  width: 54px;
  height: 54px;
  border-radius: 50%;
  border: 1px solid #1b2636;
  background: linear-gradient(180deg, #18283d 0%, #091320 100%);
  box-shadow: 
    0 3px 6px rgba(0,0,0,0.6),
    inset 0 1px 1px rgba(255,255,255,0.05);
  cursor: pointer;
  display: grid;
  place-items: center;
  font-family: 'Share Tech Mono', monospace;
  font-size: 0.75rem;
  font-weight: 700;
  color: #ffbe6a;
  outline: none;
  z-index: 10;
  transition: transform 0.1s, box-shadow 0.1s;
}

.encoder-center:hover {
  background: linear-gradient(180deg, #20354e 0%, #0c1c2f 100%);
}

.encoder-center.pressed {
  transform: scale(0.92);
  box-shadow: 
    0 1px 2px rgba(0,0,0,0.8),
    inset 0 2px 4px rgba(0,0,0,0.6);
  background: #060b13;
}

.encoder-hint {
  font-size: 0.75rem;
  color: #4b586e;
  margin-top: 10px;
}

/* ==========================================
   LCD Traseiro
   ========================================== */
.lcd-back-panel {
  background-color: #121820;
  border: 1px solid #232e3d;
  border-radius: 12px;
  padding: 16px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 12px;
}

.panel-tag {
  font-family: 'Share Tech Mono', monospace;
  font-size: 0.75rem;
  color: #ff9d00;
  letter-spacing: 0.2em;
}

.lcd-display-frame {
  background-color: #05080c;
  border: 3px solid #1d2530;
  border-radius: 6px;
  padding: 12px 18px;
  width: 100%;
  max-width: 320px;
  box-shadow: inset 0 2px 6px rgba(0,0,0,0.8);
}

.lcd-screen {
  background-color: #0b151f; /* Desligado */
  border-radius: 2px;
  padding: 8px;
  font-family: 'Fira Mono', monospace;
  font-size: 1.05rem;
  letter-spacing: 0.08em;
  color: rgba(255, 255, 255, 0.04);
  text-shadow: none;
  min-height: 56px;
  display: flex;
  flex-direction: column;
  justify-content: center;
  transition: all 0.4s ease;
  user-select: none;
}

.lcd-screen.backlight-on {
  background-color: #0044ff; /* Azul clássico aceso */
  color: #e2f0ff;
  text-shadow: 
    0 0 3px rgba(255,255,255,0.7),
    0 0 8px rgba(0,255,255,0.4);
  box-shadow: 
    0 0 15px rgba(0, 68, 255, 0.35),
    inset 0 0 12px rgba(255,255,255,0.25);
}

.lcd-text-line {
  white-space: pre;
  line-height: 1.25;
}

.lcd-note {
  font-size: 0.72rem;
  color: #55657d;
}

/* ==========================================
   Painel de Configuração do Simulador
   ========================================== */
.simulation-dashboard {
  background-color: #0f131a;
  border: 1px solid #202733;
  border-radius: 16px;
  padding: 24px;
}

.simulation-dashboard h2 {
  font-family: 'Share Tech Mono', monospace;
  font-size: 1.1rem;
  color: #b0c4de;
  margin-top: 0;
  margin-bottom: 20px;
  border-bottom: 1px solid #202733;
  padding-bottom: 8px;
  letter-spacing: 0.05em;
}

.sim-controls-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  gap: 20px;
  margin-bottom: 24px;
}

.control-card {
  background-color: #151b24;
  border: 1px solid #222c3b;
  border-radius: 10px;
  padding: 16px;
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.control-card h3 {
  margin: 0;
  font-size: 0.9rem;
  color: #8fa1b8;
  text-transform: uppercase;
  letter-spacing: 0.05em;
}

.time-adjusters {
  display: flex;
  gap: 8px;
}

.time-adjusters button,
.event-buttons button {
  flex: 1;
  background-color: #212936;
  border: 1px solid #334054;
  color: #cdd9e8;
  padding: 8px;
  font-size: 0.8rem;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.2s;
}

.time-adjusters button:hover,
.event-buttons button:hover {
  background-color: #2d384a;
}

.time-adjusters button.active {
  background-color: #0077ff;
  border-color: #0077ff;
  color: #ffffff;
}

.clock-values-view {
  font-family: 'Share Tech Mono', monospace;
  font-size: 0.95rem;
  color: #8fa1b8;
  margin-top: 6px;
}

.clock-values-view span {
  color: #ff9900;
  font-weight: 700;
}

.event-buttons {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 8px;
}

.event-buttons .danger-btn {
  background-color: #4a1515;
  border-color: #722020;
  color: #ffb8b8;
}
.event-buttons .danger-btn:hover { background-color: #611e1e; }

.event-buttons .warn-btn {
  background-color: #4a3415;
  border-color: #725120;
  color: #ffe6b8;
}
.event-buttons .warn-btn:hover { background-color: #61441e; }

.event-buttons .info-btn {
  background-color: #153c4a;
  border-color: #205c72;
  color: #b8ebff;
}
.event-buttons .info-btn:hover { background-color: #1e5161; }

.quick-settings {
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.quick-settings label {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 0.88rem;
  color: #c3d1e2;
  cursor: pointer;
}

.quick-number {
  width: 68px;
  margin-left: auto;
  border: 1px solid #334054;
  border-radius: 4px;
  background: #0f141c;
  color: #ffb45c;
  padding: 4px 6px;
}

.state-pills {
  display: flex;
  gap: 6px;
  flex-wrap: wrap;
  margin-top: 4px;
}

.pill {
  font-size: 0.72rem;
  padding: 4px 8px;
  background-color: #1c2430;
  border-radius: 4px;
  color: #55657d;
  transition: all 0.2s;
}

.pill.active {
  background-color: #ff550020;
  border: 1px solid #ff550040;
  color: #ff7733;
}

/* ==========================================
   Terminal de logs Serial
   ========================================== */
.serial-terminal {
  background-color: #090c10;
  border: 1px solid #1c222e;
  border-radius: 8px;
  overflow: hidden;
}

.terminal-header {
  background-color: #161c26;
  padding: 8px 16px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  font-size: 0.8rem;
}

.terminal-header span {
  font-family: 'Share Tech Mono', monospace;
  color: #8fa1b8;
}

.terminal-header button {
  background: none;
  border: none;
  color: #ff6a00;
  cursor: pointer;
  font-size: 0.75rem;
}

.terminal-body {
  padding: 12px;
  height: 150px;
  overflow-y: auto;
  font-family: 'Fira Mono', monospace;
  font-size: 0.78rem;
  color: #a3b8cc;
  display: flex;
  flex-direction: column;
}

.log-line {
  line-height: 1.5;
  margin-bottom: 3px;
  white-space: pre-wrap;
}

.log-empty {
  color: #4b586e;
  text-align: center;
  margin: auto;
}

/* ==========================================
   Responsividade
   ========================================== */
@media (max-width: 600px) {
  .nixie-tube {
    width: 60px;
    height: 130px;
  }
  .filament-digit {
    font-size: 3.3rem;
  }
  .nixie-glass-area {
    gap: 8px;
    padding: 10px 5px 0;
  }
}
</style>
