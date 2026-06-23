<script setup>
import { computed, onBeforeUnmount, onMounted, ref, watch } from 'vue'

// ==========================================
// 1. Estados e Configurações Principais (Espelho da EEPROM / Arduino)
// ==========================================
const now = ref(new Date())
const mode = ref('clock') // 'clock', 'radio', 'setup'
const radioMode = ref('tune') // 'tune' (SCA), 'volume' (VOL)
const setupStep = ref(0) // Menus 0 a 7 (Setup Menus 1 a 8)

// Variáveis editáveis no Setup
const hourVal = ref(12)
const minuteVal = ref(0)
const secondVal = ref(0)
const dayVal = ref(22)
const monthVal = ref(6)
const yearVal = ref(2026)

const alarmHour = ref(6)
const alarmMinute = ref(0)
const alarmEnabled = ref(false)
const alarmSource = ref('mp3') // 'radio' ou 'mp3'
const alarmFds = ref(false) // true = toca fim de semana, false = dias úteis apenas
const alarmTrack = ref(1)
const alarmMp3Vol = ref(25)

const chimeTrack = ref(2)
const chimeVolume = ref(15)

// Estados Temporários / Hardware
const screenSaver = ref(false)
const brightness = ref(6)
const volumeRadio = ref(5)
const radioStationIndex = ref(12) // Inicia no índice 12 (92.5 Kiss FM)
const currentRssi = ref(38)
const baseLedGlow = ref(true)

// Lista de Frequências Reais de São Paulo (Extraído do freqs.cpp)
const stations = [
  76.7,  // Classic Pan
  88.1,  // Gazeta
  89.1,  // 89 FM A Rádio Rock
  89.7,  // Novabrasil
  90.5,  // CBN
  90.9,  // Bandeirantes
  91.3,  // Disney
  92.1,  // Play
  92.5,  // Kiss FM
  92.9,  // Massa FM
  93.3,  // Estilo
  93.7,  // Rádio USP
  94.7,  // Antena 1
  95.3,  // Nativa
  96.1,  // Band
  96.9,  // BandNews
  97.7,  // Energia
  98.5,  // Metropolitana
  100.1, // Transamérica
  100.9, // Jovem Pan
  101.7, // Alfa FM
  102.5, // Imprensa
  103.3, // Cultura
  104.3, // Dumont
  104.7, // Transcontinental
  105.1, // Rádio 105
  105.7, // Rádio Musical
  106.3, // Mix FM
  107.3  // Eldorado
]

// Estados do Simulador / Controle
const timeMultiplier = ref(1) // Velocidade do tempo (1x, 60x, 3600x)
const serialLogs = ref([])
const audioCtx = ref(null)
const isMuted = ref(false)
const muteBeep = ref(false)

// Estados de Alarme / Soneca
const alarmRinging = ref(false)
const inSnooze = ref(false)
const snoozeTimeLeft = ref(300) // 5 minutos em segundos

// Efeito Caça-Níqueis (Anti-Poisoning)
const isSlotMachine = ref(false)
const slotDigits = ref([-1, -1, -1, -1])

// Variáveis de Cronômetros e Interrupções
let rtcTimer = null
let snoozeTimer = null
let blinkInterval = null
const blinkState = ref(true)

// Visualização do LCD
const lcdLine1 = ref("AL: Desligado   ")
const lcdLine2 = ref("Data: 22/06/2026")

// ==========================================
// 2. Lógica do Buzzer (Web Audio API)
// ==========================================
const beep = (freq, durationMs) => {
  if (muteBeep.value || isMuted.value) return
  try {
    if (!audioCtx.value) {
      audioCtx.value = new (window.AudioContext || window.webkitAudioContext)()
    }
    if (audioCtx.value.state === 'suspended') {
      audioCtx.value.resume()
    }
    const osc = audioCtx.value.createOscillator()
    const gainNode = audioCtx.value.createGain()

    osc.type = 'sine'
    osc.frequency.value = freq

    gainNode.gain.setValueAtTime(0.08, audioCtx.value.currentTime)
    gainNode.gain.exponentialRampToValueAtTime(0.001, audioCtx.value.currentTime + (durationMs / 1000))

    osc.connect(gainNode)
    gainNode.connect(audioCtx.value.destination)

    osc.start()
    osc.stop(audioCtx.value.currentTime + (durationMs / 1000))
  } catch (e) {
    console.warn("Buzzer Web Audio não inicializado ou bloqueado pelo navegador.", e)
  }
}

const addLog = (text) => {
  const timeStr = new Date().toLocaleTimeString('pt-BR', { hour12: false })
  serialLogs.value.unshift(`[${timeStr}] ${text}`)
  if (serialLogs.value.length > 50) {
    serialLogs.value.pop()
  }
}

// ==========================================
// 3. Simulação de Relógio e RTC
// ==========================================
const updateTime = () => {
  if (timeMultiplier.value === 1) {
    now.value = new Date()
    // Sincroniza apenas se não estivermos editando no Setup
    if (mode.value !== 'setup') {
      hourVal.value = now.value.getHours()
      minuteVal.value = now.value.getMinutes()
      secondVal.value = now.value.getSeconds()
      dayVal.value = now.value.getDate()
      monthVal.value = now.value.getMonth() + 1
      yearVal.value = now.value.getFullYear()
    }
  } else {
    // Multiplicador ativo: adiciona segundos virtualmente
    let totalSeconds = hourVal.value * 3600 + minuteVal.value * 60 + secondVal.value
    totalSeconds += timeMultiplier.value

    secondVal.value = totalSeconds % 60
    let totalMinutes = Math.floor(totalSeconds / 60)
    minuteVal.value = totalMinutes % 60
    let totalHours = Math.floor(totalMinutes / 60)
    hourVal.value = totalHours % 24

    // Se mudou o dia
    if (totalHours >= 24) {
      dayVal.value += 1
      if (dayVal.value > 30) { // Simples wrap-around de mês
        dayVal.value = 1
        monthVal.value += 1
        if (monthVal.value > 12) {
          monthVal.value = 1
          yearVal.value += 1
        }
      }
    }
  }

  // Verifica Caça-Níqueis (a cada 15 minutos, no segundo 0)
  if (minuteVal.value % 15 === 0 && secondVal.value === 0 && mode.value === 'clock' && !alarmRinging.value) {
    triggerSlotMachine()
  }

  // Verifica Chime da Hora Cheia (Minuto 0, Segundo 0)
  if (minuteVal.value === 0 && secondVal.value === 0 && mode.value === 'clock') {
    triggerChime()
  }

  // Verifica Disparo de Alarme
  checkAlarmTrigger()
}

onMounted(() => {
  addLog("Nano Online. Lendo EEPROM...")
  addLog("DFPlayer Mini Online (Modo Nativo).")
  addLog("RDA5807 FM Radio Inicializado.")
  
  rtcTimer = setInterval(updateTime, 1000)
  
  blinkInterval = setInterval(() => {
    blinkState.value = !blinkState.value
    // LED de base pisca a cada 2s no modo standby normal
    if (mode.value === 'clock' && !radioStationIndex.value && !screenSaver.value) {
      if (secondVal.value % 2 === 0) {
        baseLedGlow.value = true
      } else {
        baseLedGlow.value = false
      }
    }
  }, 300)
})

onBeforeUnmount(() => {
  clearInterval(rtcTimer)
  clearInterval(blinkInterval)
  if (snoozeTimer) clearInterval(snoozeTimer)
})

// ==========================================
// 4. Lógica de Alarmes e Soneca
// ==========================================
const checkAlarmTrigger = () => {
  if (alarmEnabled.value && hourVal.value === alarmHour.value && minuteVal.value === alarmMinute.value && secondVal.value === 0 && !inSnooze.value && !alarmRinging.value) {
    // DS1307 weekday: 1=Dom, 7=Sab.
    // Usando Date JS: 0=Dom, 6=Sab.
    const tempDate = new Date(yearVal.value, monthVal.value - 1, dayVal.value)
    const wday = tempDate.getDay()
    const isWeekend = (wday === 0 || wday === 6)
    
    if (!alarmFds.value && isWeekend) {
      addLog("Alarme bloqueado: Configurado apenas para dias úteis (FDS OFF).")
      return
    }

    triggerAlarm()
  }
}

const triggerAlarm = () => {
  alarmRinging.value = true
  screenSaver.value = false
  addLog(`>>> DESPERTADOR DISPAROU! Modo: ${alarmSource.value.toUpperCase()} <<<`)
  
  if (alarmSource.value === 'radio') {
    mode.value = 'radio'
    radioMode.value = 'tune'
    addLog(`Rádio FM Ligado na sintonia: ${stations[radioStationIndex.value].toFixed(1)} MHz`)
  } else {
    addLog(`DFPlayer tocando faixa de Alarme: Track ${alarmTrack.value} (Volume: ${alarmMp3Vol.value})`)
  }
  // Bipe intermitente se for buzzer
  beep(1000, 500)
}

const cancelAlarm = () => {
  if (alarmRinging.value || inSnooze.value) {
    alarmRinging.value = false
    inSnooze.value = false
    addLog(">>> ALARME CANCELADO PELO USUÁRIO <<<")
    beep(500, 300) // Bipe longo de confirmação
    if (mode.value === 'radio') {
      mode.value = 'clock'
    }
  }
}

const activateSnooze = () => {
  if (alarmRinging.value || inSnooze.value) {
    alarmRinging.value = false
    inSnooze.value = true
    snoozeTimeLeft.value = 300 // 5 minutos
    addLog(">>> SONECA ATIVADA (5 MINUTOS) <<<")
    
    // Bipes duplos de confirmação
    beep(1000, 100)
    setTimeout(() => beep(1000, 100), 160)

    if (mode.value === 'radio') {
      mode.value = 'clock'
    }

    if (snoozeTimer) clearInterval(snoozeTimer)
    snoozeTimer = setInterval(() => {
      snoozeTimeLeft.value--
      if (snoozeTimeLeft.value <= 0) {
        clearInterval(snoozeTimer)
        inSnooze.value = false
        triggerAlarm()
      }
    }, 1000)
  }
}

const triggerChime = () => {
  addLog(`[CHIME] Hora cheia! DFPlayer tocando chime track ${chimeTrack.value} (Vol: ${chimeVolume.value})`)
  beep(880, 150)
  setTimeout(() => beep(880, 150), 200)
}

// ==========================================
// 5. Efeito Caça-Níqueis (Anti-Poisoning)
// ==========================================
const triggerSlotMachine = () => {
  if (isSlotMachine.value) return
  isSlotMachine.value = true
  addLog("Iniciando rotina de proteção de cátodo (Caça-Níqueis)...")
  
  const startTime = Date.now()
  const interval = setInterval(() => {
    const elapsed = Date.now() - startTime
    
    if (elapsed >= 4000) {
      clearInterval(interval)
      isSlotMachine.value = false
    } else {
      // Dígito 1 (Hora dezena) - embaralha até 3s
      if (elapsed < 3000) {
        slotDigits.value[0] = Math.floor(Math.random() * 10)
      } else {
        slotDigits.value[0] = hourVal.value < 10 ? -1 : Math.floor(hourVal.value / 10)
      }

      // Dígito 2 (Hora unidade) - embaralha até 3.3s
      if (elapsed < 3333) {
        slotDigits.value[1] = Math.floor(Math.random() * 10)
      } else {
        slotDigits.value[1] = hourVal.value % 10
      }

      // Dígito 3 (Minuto dezena) - embaralha até 3.6s
      if (elapsed < 3666) {
        slotDigits.value[2] = Math.floor(Math.random() * 10)
      } else {
        slotDigits.value[2] = Math.floor(minuteVal.value / 10)
      }

      // Dígito 4 (Minuto unidade) - embaralha até 4.0s
      if (elapsed < 4000) {
        slotDigits.value[3] = Math.floor(Math.random() * 10)
      } else {
        slotDigits.value[3] = minuteVal.value % 10
      }
    }
  }, 50)
}

// ==========================================
// 6. Lógica de Sintonia do Rádio e RSSI (Não-Bloqueante)
// ==========================================
const isScanning = ref(false)
const scanDirection = ref(1)

const startRadioScan = (dir) => {
  if (isScanning.value) return
  isScanning.value = true
  scanDirection.value = dir
  addLog(`Iniciando varredura FM (Seek ${dir > 0 ? 'CW' : 'CCW'})... Mute ativo.`)
  
  let steps = 0
  const interval = setInterval(() => {
    steps++
    
    // Atualiza índice de estação circularmente
    let newIndex = radioStationIndex.value + dir
    if (newIndex >= stations.length) newIndex = 0
    if (newIndex < 0) newIndex = stations.length - 1
    radioStationIndex.value = newIndex
    
    // Simula RSSI instável durante a busca
    currentRssi.value = Math.floor(Math.random() * 12) + 12 // Baixo sinal

    // Simula bip de áudio analógico rápido
    beep(400, 10)

    // Condição de parada: Sinal forte (simulado em estações reais)
    // Damos ao menos 3 passos para não travar na mesma estação
    const rssiStop = Math.floor(Math.random() * 18) + 32
    if (steps >= 3 && rssiStop >= 38) {
      clearInterval(interval)
      currentRssi.value = rssiStop
      isScanning.value = false
      addLog(`Estação encontrada: ${stations[radioStationIndex.value].toFixed(1)} MHz (Sinal: ${currentRssi.value} RSSI). Mute desligado.`)
      beep(1000, 40)
    } else if (steps >= 100) { // Limite de varredura
      clearInterval(interval)
      isScanning.value = false
      currentRssi.value = 25
      addLog(`Seek finalizado por timeout.`)
    }
  }, 100)
}

// ==========================================
// 7. Controle do Encoder Rotativo (Rotação)
// ==========================================
const rotate = (direction) => {
  if (screenSaver.value) {
    // Movimentação do encoder acorda o relógio
    screenSaver.value = false
    beep(50, 40)
    addLog("Relógio acordado por giro do encoder.")
    return
  }

  // Bipe de rotação rápido (15-20ms)
  beep(1000, 15)

  if (mode.value === 'clock') {
    // Ajusta brilho das Nixies (1 a 11)
    if (direction < 0) { // CCW
      brightness.value = Math.max(1, brightness.value - 1)
    } else { // CW
      brightness.value = Math.min(11, brightness.value + 1)
    }
    addLog(`Brilho Nixie: ${brightness.value}`)
    return
  }

  if (mode.value === 'radio') {
    if (radioMode.value === 'volume') {
      // Ajusta volume do rádio (0 a 15)
      if (direction < 0) { // CCW
        volumeRadio.value = Math.max(0, volumeRadio.value - 1)
      } else { // CW
        volumeRadio.value = Math.min(15, volumeRadio.value + 1)
      }
      addLog(`Volume digital Rádio: ${volumeRadio.value}`)
    } else {
      // Modo Sintonia (SCA): Inicia busca automática não-bloqueante
      startRadioScan(direction)
    }
    return
  }

  if (mode.value === 'setup') {
    // Configuração sintonizada nas direções do encoder descritas no manual.md
    switch (setupStep.value) {
      case 0: // Ajustar Relógio
        if (direction < 0) { // Esquerda: Ajusta Hora
          hourVal.value = (hourVal.value + 1) % 24
        } else { // Direita: Ajusta Minuto
          minuteVal.value = (minuteVal.value + 1) % 60
        }
        break
      case 1: // Hora do Alarme
        if (direction < 0) { // Esquerda: Ajusta Hora
          alarmHour.value = (alarmHour.value + 1) % 24
        } else { // Direita: Ajusta Minuto
          alarmMinute.value = (alarmMinute.value + 1) % 60
        }
        break
      case 2: // Modo do Alarme
        if (direction < 0) { // Esquerda: Alterna MP3 / RADIO
          alarmSource.value = alarmSource.value === 'radio' ? 'mp3' : 'radio'
        } else { // Direita: Alterna ON / OFF
          alarmEnabled.value = !alarmEnabled.value
        }
        break
      case 3: // Ajuste Data
        if (direction < 0) { // Esquerda: Ajusta Dia
          dayVal.value = dayVal.value >= 31 ? 1 : dayVal.value + 1
        } else { // Direita: Ajusta Mês
          monthVal.value = monthVal.value >= 12 ? 1 : monthVal.value + 1
        }
        break
      case 4: // Ajuste Ano
        if (direction < 0) { // Esquerda: Decrementa
          yearVal.value = Math.max(2000, yearVal.value - 1)
        } else { // Direita: Incrementa
          yearVal.value = Math.min(2099, yearVal.value + 1)
        }
        break
      case 5: // Alarme Fim de Semana
        alarmFds.value = !alarmFds.value
        break
      case 6: // Som/Volume Alarme
        if (direction < 0) { // Esquerda: Som (Track)
          alarmTrack.value = alarmTrack.value >= 99 ? 1 : alarmTrack.value + 1
        } else { // Direita: Volume MP3 (0 a 30)
          alarmMp3Vol.value = Math.min(30, Math.max(0, alarmMp3Vol.value + 1))
        }
        break
      case 7: // Som/Volume Hora Cheia
        if (direction < 0) { // Esquerda: Som (Track)
          chimeTrack.value = chimeTrack.value >= 99 ? 1 : chimeTrack.value + 1
        } else { // Direita: Volume Chime (0 a 30)
          chimeVolume.value = Math.min(30, Math.max(0, chimeVolume.value + 1))
        }
        break
    }
  }
}

// ==========================================
// 8. Cliques do Encoder (Curto, Duplo e Longo)
// ==========================================
const isPressed = ref(false)
let clickCount = 0
let clickTimeout = null
let longPressTimeout = null

const handleButtonDown = () => {
  isPressed.value = true
  
  // Detecção de Toque Longo (0.7 segundos)
  longPressTimeout = setTimeout(() => {
    triggerLongPress()
    longPressTimeout = null
    clickCount = 0 // Cancela bipes normais
  }, 700)
}

const handleButtonUp = () => {
  isPressed.value = false
  if (longPressTimeout) {
    clearTimeout(longPressTimeout)
    longPressTimeout = null
    
    // Detecção de Cliques curtos / Duplos
    clickCount++
    if (clickCount === 1) {
      clickTimeout = setTimeout(() => {
        if (clickCount === 1) {
          triggerShortClick()
        } else if (clickCount === 2) {
          triggerDoubleClick()
        }
        clickCount = 0
      }, 250) // Janela de clique duplo
    }
  }
}

const triggerShortClick = () => {
  if (alarmRinging.value || inSnooze.value) {
    cancelAlarm()
    return
  }

  if (screenSaver.value) {
    screenSaver.value = false
    beep(50, 50)
    addLog("Relógio acordado por clique no encoder.")
    return
  }

  beep(30, 40) // Bipe curto de clique

  if (mode.value === 'clock') {
    // Entra no Setup
    mode.value = 'setup'
    setupStep.value = 0
    // Bipe duplo de entrada
    beep(1000, 60)
    setTimeout(() => beep(1000, 60), 120)
    addLog("Menu Setup Iniciado. LCD Traseiro Ligado.")
  } else if (mode.value === 'radio') {
    // Alterna VOL e SCA
    radioMode.value = radioMode.value === 'tune' ? 'volume' : 'tune'
    addLog(`Modo Rádio alterado para: ${radioMode.value.toUpperCase()}`)
  } else if (mode.value === 'setup') {
    // Avança páginas do Setup
    if (setupStep.value < 7) {
      setupStep.value++
      addLog(`Menu Avançado: Página ${setupStep.value + 1}/8`)
    } else {
      // Salva e sai
      salvarESair()
    }
  }
}

const triggerDoubleClick = () => {
  if (mode.value === 'clock' && !alarmRinging.value && !inSnooze.value) {
    screenSaver.value = !screenSaver.value
    // Bipe duplo de transição
    beep(1000, 50)
    setTimeout(() => beep(1000, 50), 80)
    addLog(`Screen Saver Manual: ${screenSaver.value ? 'ATIVADO (Tubo Off)' : 'DESATIVADO (Tubo On)'}`)
  }
}

const triggerLongPress = () => {
  if (alarmRinging.value || inSnooze.value) {
    activateSnooze()
    return
  }

  if (mode.value === 'clock') {
    // Liga o Rádio
    mode.value = 'radio'
    radioMode.value = 'tune'
    beep(1000, 100)
    addLog(`Rádio ligado via toque longo. Freq: ${stations[radioStationIndex.value].toFixed(1)} MHz`)
  } else if (mode.value === 'radio') {
    // Desliga o Rádio
    mode.value = 'clock'
    beep(1000, 100)
    addLog("Rádio desligado por toque longo. Retornando ao relógio.")
  } else if (mode.value === 'setup') {
    // Salva e sai
    salvarESair()
  }
}

const salvarESair = () => {
  mode.value = 'clock'
  setupStep.value = 0
  // Bipe duplo de saída longo
  beep(1000, 120)
  setTimeout(() => beep(1000, 120), 200)
  addLog("Ajustes gravados na EEPROM com sucesso! LCD Desligado.")
}

// ==========================================
// 9. Lógica de Rotação por Arraste de Mouse/Touch
// ==========================================
const knobRotation = ref(0)
let startAngle = 0
let isDragging = false

const getAngle = (x, y, element) => {
  const rect = element.getBoundingClientRect()
  const centerX = rect.left + rect.width / 2
  const centerY = rect.top + rect.height / 2
  return Math.atan2(y - centerY, x - centerX) * (180 / Math.PI)
}

const handleKnobStart = (event) => {
  if (event.target.classList.contains('encoder-center')) return
  isDragging = true
  const clientX = event.clientX || event.touches?.[0]?.clientX
  const clientY = event.clientY || event.touches?.[0]?.clientY
  startAngle = getAngle(clientX, clientY, event.currentTarget)
  
  window.addEventListener('pointermove', handleKnobMove)
  window.addEventListener('pointerup', handleKnobEnd)
}

const handleKnobMove = (event) => {
  if (!isDragging) return
  const clientX = event.clientX || event.touches?.[0]?.clientX
  const clientY = event.clientY || event.touches?.[0]?.clientY
  const knobElement = document.querySelector('.encoder-ring')
  if (!knobElement) return
  
  const currentAngle = getAngle(clientX, clientY, knobElement)
  let diff = currentAngle - startAngle
  
  if (diff > 180) diff -= 360
  if (diff < -180) diff += 360
  
  // 15 graus por clique/tick do encoder
  const stepAngle = 15
  if (Math.abs(diff) >= stepAngle) {
    const ticks = Math.floor(diff / stepAngle)
    rotate(ticks)
    startAngle = currentAngle
    knobRotation.value = (knobRotation.value + (ticks * stepAngle)) % 360
  }
}

const handleKnobEnd = () => {
  isDragging = false
  window.removeEventListener('pointermove', handleKnobMove)
  window.removeEventListener('pointerup', handleKnobEnd)
}

const handleWheel = (e) => {
  rotate(e.deltaY > 0 ? 1 : -1)
}

// ==========================================
// 10. Atualização Reativa do LCD 16x2
// ==========================================
const getDayName = (d, m, y) => {
  const date = new Date(y, m - 1, d)
  const weekdays = ['Domingo', 'Segunda', 'Terca', 'Quarta', 'Quinta', 'Sexta', 'Sabado']
  return weekdays[date.getDay()] || "Erro"
}

watch([now, mode, radioMode, setupStep, hourVal, minuteVal, dayVal, monthVal, yearVal, alarmHour, alarmMinute, alarmEnabled, alarmSource, alarmTrack, alarmMp3Vol, chimeTrack, chimeVolume, alarmFds, currentRssi, volumeRadio, alarmRinging, inSnooze, snoozeTimeLeft], () => {
  if (alarmRinging.value) {
    if (alarmSource.value === 'radio') {
      lcdLine1.value = "Em alarme: Radio"
      lcdLine2.value = `FM ${stations[radioStationIndex.value].toFixed(1)} MHz`
    } else {
      lcdLine1.value = "Em alarme: MP3  "
      lcdLine2.value = "Som: Alarme     "
    }
  } else if (inSnooze.value) {
    lcdLine1.value = "Em soneca       "
    const m = Math.floor(snoozeTimeLeft.value / 60)
    const s = snoozeTimeLeft.value % 60
    lcdLine2.value = `Tempo: ${String(m).padStart(2, '0')}:${String(s).padStart(2, '0')}`
  } else if (mode.value === 'radio') {
    const freqStr = stations[radioStationIndex.value].toFixed(1)
    if (radioMode.value === 'volume') {
      lcdLine1.value = `FM ${freqStr}MHz  VOL`
      lcdLine2.value = `Volume: ${String(volumeRadio.value).padStart(2, '0')}/15   `
    } else {
      lcdLine1.value = `FM ${freqStr}MHz  SCA`
      lcdLine2.value = `Sig:${String(currentRssi.value).padStart(2, '0')}  Stereo  `
    }
  } else if (mode.value === 'setup') {
    switch (setupStep.value) {
      case 0:
        lcdLine1.value = "Ajustar Relogio "
        lcdLine2.value = `${String(hourVal.value).padStart(2, '0')}:${String(minuteVal.value).padStart(2, '0')}`
        break
      case 1:
        lcdLine1.value = "Hora do Alarme  "
        lcdLine2.value = `${String(alarmHour.value).padStart(2, '0')}:${String(alarmMinute.value).padStart(2, '0')}`
        break
      case 2:
        lcdLine1.value = "Modo do Alarme  "
        lcdLine2.value = `${alarmSource.value.toUpperCase()} (${alarmEnabled.value ? 'ON ' : 'OFF'})`
        break
      case 3:
        lcdLine1.value = "Ajuste Data: D/M"
        lcdLine2.value = `${String(dayVal.value).padStart(2, '0')}/${String(monthVal.value).padStart(2, '0')} ${getDayName(dayVal.value, monthVal.value, yearVal.value)}`
        break
      case 4:
        lcdLine1.value = `Ajuste Ano: ${yearVal.value}`
        lcdLine2.value = `Dia: ${getDayName(dayVal.value, monthVal.value, yearVal.value)}`
        break
      case 5:
        lcdLine1.value = "Alarme Fim de S."
        lcdLine2.value = alarmFds.value ? "     LIGADO     " : "   DESLIGADO    "
        break
      case 6:
        lcdLine1.value = `Som Alarme: ${String(alarmTrack.value).padStart(2, '0')}  `
        lcdLine2.value = `Vol Alarme: ${String(alarmMp3Vol.value).padStart(2, '0')}/30`
        break
      case 7:
        lcdLine1.value = `Som Hora: ${String(chimeTrack.value).padStart(2, '0')}    `
        lcdLine2.value = `Vol Hora: ${String(chimeVolume.value).padStart(2, '0')}/30  `
        break
    }
  } else {
    // Modo Normal
    if (alarmEnabled.value) {
      const type = alarmSource.value === 'radio' ? 'Rad' : 'MP3'
      const fdsStr = alarmFds.value ? 'FDS' : '   '
      lcdLine1.value = `AL:${String(alarmHour.value).padStart(2, '0')}:${String(alarmMinute.value).padStart(2, '0')} ${type} ${fdsStr}`
    } else {
      lcdLine1.value = "AL: Desligado   "
    }
    lcdLine2.value = `Data: ${String(dayVal.value).padStart(2, '0')}/${String(monthVal.value).padStart(2, '0')}/${yearVal.value}`
  }
})

// ==========================================
// 11. Renderização das Válvulas Nixies
// ==========================================
const nixieDigits = computed(() => {
  // Se caça-níqueis ativo, retorna valor embaralhado
  if (isSlotMachine.value) {
    return slotDigits.value
  }

  // Se o screen saver estiver ativo, apaga totalmente o display (-1 significa apagado)
  if (screenSaver.value) {
    return [-1, -1, -1, -1]
  }

  if (inSnooze.value) {
    // Soneca exibe minutos e segundos regressivos
    const m = Math.floor(snoozeTimeLeft.value / 60)
    const s = snoozeTimeLeft.value % 60
    return [
      Math.floor(m / 10),
      m % 10,
      Math.floor(s / 10),
      s % 10
    ]
  }

  if (mode.value === 'clock') {
    return [
      hourVal.value < 10 ? -1 : Math.floor(hourVal.value / 10),
      hourVal.value % 10,
      Math.floor(minuteVal.value / 10),
      minuteVal.value % 10
    ]
  }

  if (mode.value === 'radio') {
    if (radioMode.value === 'tune') {
      const freq = stations[radioStationIndex.value]
      const rounded = Math.round(freq * 10)
      const digits = String(rounded).padStart(4, ' ')
      return digits.split('').map(c => c === ' ' ? -1 : Number(c))
    } else {
      // Modo Volume: Sinal RSSI nas horas (à esquerda), volume nos minutos (à direita)
      const rssi = currentRssi.value
      const vol = volumeRadio.value
      return [
        rssi < 10 ? -1 : Math.floor(rssi / 10),
        rssi % 10,
        Math.floor(vol / 10),
        vol % 10
      ]
    }
  }

  if (mode.value === 'setup') {
    // Se estiver piscando (setup), alternamos com vazio baseado no clock de 300ms
    if (!blinkState.value) {
      return [-1, -1, -1, -1]
    }

    switch (setupStep.value) {
      case 0: // Relógio
        return [
          hourVal.value < 10 ? -1 : Math.floor(hourVal.value / 10),
          hourVal.value % 10,
          Math.floor(minuteVal.value / 10),
          minuteVal.value % 10
        ]
      case 1: // Alarme Hora/Min
        return [
          alarmHour.value < 10 ? -1 : Math.floor(alarmHour.value / 10),
          alarmHour.value % 10,
          Math.floor(alarmMinute.value / 10),
          alarmMinute.value % 10
        ]
      case 2: // Modo status alarme
        return [
          -1,
          alarmSource.value === 'radio' ? 1 : 0, // 1 = Rádio, 0 = MP3
          -1,
          alarmEnabled.value ? 1 : 0 // 1 = On, 0 = Off
        ]
      case 3: // Data Dia/Mês
        return [
          dayVal.value < 10 ? -1 : Math.floor(dayVal.value / 10),
          dayVal.value % 10,
          monthVal.value < 10 ? -1 : Math.floor(monthVal.value / 10),
          monthVal.value % 10
        ]
      case 4: // Ano
        return [
          2,
          0,
          Math.floor((yearVal.value % 100) / 10),
          (yearVal.value % 100) % 10
        ]
      case 5: // Fim de Semana
        return [
          -1,
          -1,
          -1,
          alarmFds.value ? 1 : 0
        ]
      case 6: // Faixa e Volume do Alarme
        return [
          alarmTrack.value < 10 ? -1 : Math.floor(alarmTrack.value / 10),
          alarmTrack.value % 10,
          Math.floor(alarmMp3Vol.value / 10),
          alarmMp3Vol.value % 10
        ]
      case 7: // Faixa e Volume do Chime
        return [
          chimeTrack.value < 10 ? -1 : Math.floor(chimeTrack.value / 10),
          chimeTrack.value % 10,
          Math.floor(chimeVolume.value / 10),
          chimeVolume.value % 10
        ]
    }
  }

  return [-1, -1, -1, -1]
})

const showColon = computed(() => {
  if (screenSaver.value) return false
  if (mode.value === 'clock' || inSnooze.value) {
    // Colon pisca a cada segundo no modo relógio
    return secondVal.value % 2 === 0
  }
  if (mode.value === 'setup' && setupStep.value <= 1) {
    return blinkState.value
  }
  return false
})
</script>

<template>
  <main class="app-shell">
    <div class="simulator-wrapper">
      
      <!-- CABINE DO RELÓGIO (Chassi físico) -->
      <section class="clock-cabinet">
        <div class="wood-top"></div>
        <div class="cabinet-body">
          
          <!-- Leds Traseiros e Decoração de Brilho -->
          <div class="ambient-glow" :class="{ 'led-on': baseLedGlow && !screenSaver, 'color-radio': mode === 'radio' }"></div>
          
          <div class="panel-front">
            <div class="panel-header">
              <span class="brand-logo">NIXIE CHRONOS</span>
              <div class="led-indicator" :class="{ 'active': alarmEnabled }"></div>
            </div>

            <!-- Grade das Válvulas Nixies -->
            <div class="nixie-cradle">
              <div class="nixie-glass-area">
                
                <!-- Tubo 1 -->
                <div class="nixie-tube" :class="{ 'tube-off': nixieDigits[0] === -1 }">
                  <div class="glass-reflection"></div>
                  <div class="grid-mesh"></div>
                  <div class="nixie-glow" :class="{ 'active-glow': !screenSaver }"></div>
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
                  <div class="nixie-glow" :class="{ 'active-glow': !screenSaver }"></div>
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
                  <div class="nixie-glow" :class="{ 'active-glow': !screenSaver }"></div>
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
                  <div class="nixie-glow" :class="{ 'active-glow': !screenSaver }"></div>
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
                   :class="{ 'bounce': mode === 'radio' || alarmRinging }"></div>
            </div>

            <!-- Controle do Encoder Físico -->
            <div class="encoder-casing">
              <div class="dial-indicators">
                <span class="lbl-left" :class="{ 'highlight': mode==='radio' && radioMode==='tune' }">SCA</span>
                <span class="lbl-right" :class="{ 'highlight': mode==='radio' && radioMode==='volume' }">VOL</span>
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
                          @pointerup.prevent.stop="handleButtonUp">
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
              <button @click="timeMultiplier = 1; addLog('Tempo ajustado para 1x (Tempo Real)')" :class="{ 'active': timeMultiplier === 1 }">1x (Real)</button>
              <button @click="timeMultiplier = 60; addLog('Tempo acelerado: 1 min/seg')" :class="{ 'active': timeMultiplier === 60 }">60x (Min/s)</button>
              <button @click="timeMultiplier = 3600; addLog('Tempo acelerado: 1 hora/seg')" :class="{ 'active': timeMultiplier === 3600 }">3600x (H/s)</button>
            </div>
            <div class="clock-values-view">
              Hora RTC: <span>{{ String(hourVal).padStart(2, '0') }}:{{ String(minuteVal).padStart(2, '0') }}:{{ String(secondVal).padStart(2, '0') }}</span>
            </div>
          </div>

          <div class="control-card">
            <h3>Disparos & Eventos</h3>
            <div class="event-buttons">
              <button @click="triggerAlarm" class="danger-btn">Forçar Disparo Alarme</button>
              <button @click="triggerChime" class="warn-btn">Testar Chime</button>
              <button @click="triggerSlotMachine" class="info-btn">Girar Caça-Níqueis</button>
              <button @click="beep(1000, 150)" class="action-btn">Manual Beep</button>
            </div>
          </div>

          <div class="control-card">
            <h3>Ajustes Rápidos</h3>
            <div class="quick-settings">
              <label>
                <input type="checkbox" v-model="alarmEnabled" @change="addLog(`Alarme via hardware: ${alarmEnabled ? 'LIGADO' : 'DESLIGADO'}`)" />
                Ativar Alarme
              </label>
              <label>
                <input type="checkbox" v-model="muteBeep" />
                Mudar Buzzer (Mudo)
              </label>
            </div>
            <div class="state-pills">
              <span class="pill" :class="{ 'active': screenSaver }">Screen Saver</span>
              <span class="pill" :class="{ 'active': alarmRinging }">Tocando</span>
              <span class="pill" :class="{ 'active': inSnooze }">Soneca</span>
            </div>
          </div>
        </div>

        <div class="serial-terminal">
          <div class="terminal-header">
            <span>Terminal Serial (9600 baud)</span>
            <button @click="serialLogs = []">Limpar Logs</button>
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
  opacity: 1;
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
  width: 130px;
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
