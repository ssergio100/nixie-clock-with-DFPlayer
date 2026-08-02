import { computed, onBeforeUnmount, onMounted, ref } from 'vue'

export const BRILHO_SEM_PISCA = -1
export const BRILHO_DESLIGADO = -2
export const BRILHO_MAXIMO = 10

const INTERVALO_TEMPERATURA = 20
const DURACAO_TEMPERATURA = 2
const DURACAO_CHIME_MS = 2000
const DURACAO_SONECA_MS = 5 * 60 * 1000
const DURACAO_MAXIMA_ALARME_MS = 30 * 60 * 1000
const FREQUENCIA_MINIMA = 76.1
const FREQUENCIA_MAXIMA = 107.9
const PASSO_FREQUENCIA = 0.2
const MAX_PASSOS_BUSCA = 170
const LIMIAR_RSSI = 30
const CONFIG_KEY = 'nixie-clock-config-v1'

const SETUP_HORARIO = 0
const SETUP_ALARME = 1
const SETUP_DATA = 2
const SETUP_SONS = 3
const PASSOS_SETUP = [2, 4, 3, 4]

const ESTACOES = [
  { frequency: 76.7, name: 'Classic Pan', rssi: 42 },
  { frequency: 88.1, name: 'Gazeta', rssi: 35 },
  { frequency: 89.1, name: '89 FM', rssi: 47 },
  { frequency: 89.7, name: 'Novabrasil', rssi: 39 },
  { frequency: 90.5, name: 'CBN', rssi: 44 },
  { frequency: 90.9, name: 'Bandeirantes', rssi: 40 },
  { frequency: 91.3, name: 'Disney', rssi: 43 },
  { frequency: 92.1, name: 'Play', rssi: 34 },
  { frequency: 92.5, name: 'Kiss FM', rssi: 49 },
  { frequency: 92.9, name: 'Massa FM', rssi: 37 },
  { frequency: 93.3, name: 'Estilo', rssi: 32 },
  { frequency: 93.7, name: 'Radio USP', rssi: 36 },
  { frequency: 94.7, name: 'Antena 1', rssi: 48 },
  { frequency: 95.3, name: 'Nativa', rssi: 45 },
  { frequency: 96.1, name: 'Band', rssi: 41 },
  { frequency: 96.9, name: 'BandNews', rssi: 46 },
  { frequency: 97.7, name: 'Energia', rssi: 43 },
  { frequency: 98.5, name: 'Metropolitana', rssi: 50 },
  { frequency: 100.1, name: 'Transamerica', rssi: 44 },
  { frequency: 100.9, name: 'Jovem Pan', rssi: 48 },
  { frequency: 101.7, name: 'Alfa FM', rssi: 41 },
  { frequency: 102.5, name: 'Imprensa', rssi: 33 },
  { frequency: 103.3, name: 'Cultura', rssi: 38 },
  { frequency: 104.3, name: 'Dumont', rssi: 32 },
  { frequency: 104.7, name: 'Transcontinental', rssi: 37 },
  { frequency: 105.1, name: 'Radio 105', rssi: 39 },
  { frequency: 105.7, name: 'Radio Musical', rssi: 35 },
  { frequency: 106.3, name: 'Mix FM', rssi: 46 },
  { frequency: 107.3, name: 'Eldorado', rssi: 40 }
]

export function anoBissexto(ano) {
  return ano % 4 === 0 && (ano % 100 !== 0 || ano % 400 === 0)
}

export function diasNoMes(mes, ano) {
  if (mes === 2) return anoBissexto(ano) ? 29 : 28
  if ([4, 6, 9, 11].includes(mes)) return 30
  if (mes >= 1 && mes <= 12) return 31
  return 0
}

export function limitarDiaAoMes(dia, mes, ano) {
  const ultimoDia = diasNoMes(mes, ano)
  if (ultimoDia === 0 || dia < 1) return 1
  return Math.min(dia, ultimoDia)
}

export function avancarFrequencia(frequencia, direcao) {
  const minimo = Math.round(FREQUENCIA_MINIMA * 10)
  const maximo = Math.round(FREQUENCIA_MAXIMA * 10)
  const passo = Math.round(PASSO_FREQUENCIA * 10)
  let valor = Math.round(frequencia * 10) + (direcao > 0 ? passo : -passo)
  if (valor > maximo) valor = minimo
  if (valor < minimo) valor = maximo
  return valor / 10
}

const pad2 = (value) => String(value).padStart(2, '0')
const pad16 = (value) => String(value).padEnd(16, ' ').slice(0, 16)
const cyclic = (value, min, max, direction) => {
  if (direction > 0) return value >= max ? min : value + 1
  return value <= min ? max : value - 1
}

const dataKey = (year, month, day) =>
  `${year}-${pad2(month)}-${pad2(day)}`

export function useNixieClock() {
  const initialDate = new Date()

  const mode = ref('clock')
  const radioMode = ref('tune')
  const setupScreen = ref(SETUP_HORARIO)
  const setupField = ref(0)

  const hourVal = ref(initialDate.getHours())
  const minuteVal = ref(initialDate.getMinutes())
  const secondVal = ref(initialDate.getSeconds())
  const dayVal = ref(initialDate.getDate())
  const monthVal = ref(initialDate.getMonth() + 1)
  const yearVal = ref(initialDate.getFullYear())

  const alarmHour = ref(6)
  const alarmMinute = ref(0)
  const alarmEnabled = ref(false)
  const alarmSource = ref('mp3')
  const alarmFds = ref(false)
  const alarmTrack = ref(1)
  const alarmMp3Vol = ref(25)
  const chimeTrack = ref(2)
  const chimeVolume = ref(15)

  const brightness = ref(5)
  const volumeRadio = ref(1)
  const radioFrequency = ref(94.7)
  const currentRssi = ref(48)
  const temperatureTenths = ref(245)

  const timeMultiplier = ref(1)
  const serialLogs = ref([])
  const audioCtx = ref(null)
  const muteBeep = ref(false)

  const alarmRinging = ref(false)
  const inSnooze = ref(false)
  const snoozeTimeLeft = ref(300)
  const isSlotMachine = ref(false)
  const slotDigits = ref([-1, -1, -1, -1])
  const isScanning = ref(false)
  const chimeActive = ref(false)
  const audioState = ref('free')
  const blinkState = ref(true)
  const secondsLedState = ref(true)
  const isPressed = ref(false)
  const knobRotation = ref(0)

  let clockTimer = null
  let blinkTimer = null
  let secondsLedTimer = null
  let snoozeTimer = null
  let scanTimer = null
  let slotTimer = null
  let chimeTimer = null
  let alarmStartedAt = 0
  let snoozeEndsAt = 0
  let scanSteps = 0
  let lastAlarmKey = ''
  let lastChimeKey = ''
  let lastSlotKey = ''
  let radioWasActiveBeforeChime = false
  let longPressTimer = null
  let longPressHandled = false
  let startAngle = 0
  let isDragging = false

  const addLog = (text) => {
    const timeStr = new Date().toLocaleTimeString('pt-BR', { hour12: false })
    serialLogs.value.unshift(`[${timeStr}] ${text}`)
    if (serialLogs.value.length > 50) serialLogs.value.pop()
  }

  const beep = (frequency = 1000, durationMs = 50) => {
    if (muteBeep.value) return
    try {
      if (!audioCtx.value) {
        audioCtx.value = new (window.AudioContext || window.webkitAudioContext)()
      }
      if (audioCtx.value.state === 'suspended') audioCtx.value.resume()

      const oscillator = audioCtx.value.createOscillator()
      const gain = audioCtx.value.createGain()
      oscillator.type = 'sine'
      oscillator.frequency.value = frequency
      gain.gain.setValueAtTime(0.08, audioCtx.value.currentTime)
      gain.gain.exponentialRampToValueAtTime(
        0.001,
        audioCtx.value.currentTime + durationMs / 1000
      )
      oscillator.connect(gain)
      gain.connect(audioCtx.value.destination)
      oscillator.start()
      oscillator.stop(audioCtx.value.currentTime + durationMs / 1000)
    } catch (error) {
      console.warn('Web Audio indisponivel.', error)
    }
  }

  const beepDouble = () => {
    beep(1000, 100)
    window.setTimeout(() => beep(1000, 100), 160)
  }

  const validStoredConfig = (config) => {
    const frequencyTenths = Math.round(Number(config.radioFrequency) * 10)
    return config &&
      Number.isInteger(config.alarmHour) && config.alarmHour >= 0 && config.alarmHour <= 23 &&
      Number.isInteger(config.alarmMinute) && config.alarmMinute >= 0 && config.alarmMinute <= 59 &&
      typeof config.alarmEnabled === 'boolean' &&
      ['mp3', 'radio'].includes(config.alarmSource) &&
      typeof config.alarmFds === 'boolean' &&
      Number.isInteger(config.alarmTrack) && config.alarmTrack >= 1 && config.alarmTrack <= 99 &&
      Number.isInteger(config.alarmMp3Vol) && config.alarmMp3Vol >= 0 && config.alarmMp3Vol <= 30 &&
      Number.isInteger(config.chimeTrack) && config.chimeTrack >= 1 && config.chimeTrack <= 99 &&
      Number.isInteger(config.chimeVolume) && config.chimeVolume >= 0 && config.chimeVolume <= 30 &&
      frequencyTenths >= 761 && frequencyTenths <= 1079 &&
      (frequencyTenths - 761) % 2 === 0
  }

  const saveConfiguration = () => {
    const record = {
      version: 1,
      config: {
        alarmHour: alarmHour.value,
        alarmMinute: alarmMinute.value,
        alarmEnabled: alarmEnabled.value,
        alarmSource: alarmSource.value,
        alarmFds: alarmFds.value,
        alarmTrack: alarmTrack.value,
        alarmMp3Vol: alarmMp3Vol.value,
        chimeTrack: chimeTrack.value,
        chimeVolume: chimeVolume.value,
        radioFrequency: radioFrequency.value
      }
    }
    window.localStorage.setItem(CONFIG_KEY, JSON.stringify(record))
  }

  const loadConfiguration = () => {
    try {
      const stored = window.localStorage.getItem(CONFIG_KEY)
      if (!stored) {
        saveConfiguration()
        addLog('Configuracao virtual padrao gravada.')
        return
      }

      const record = JSON.parse(stored)
      if (record.version !== 1 || !validStoredConfig(record.config)) {
        throw new Error('registro invalido')
      }

      const config = record.config
      alarmHour.value = config.alarmHour
      alarmMinute.value = config.alarmMinute
      alarmEnabled.value = config.alarmEnabled
      alarmSource.value = config.alarmSource
      alarmFds.value = config.alarmFds
      alarmTrack.value = config.alarmTrack
      alarmMp3Vol.value = config.alarmMp3Vol
      chimeTrack.value = config.chimeTrack
      chimeVolume.value = config.chimeVolume
      radioFrequency.value = config.radioFrequency
      addLog('Configuracao virtual carregada e validada.')
    } catch {
      window.localStorage.removeItem(CONFIG_KEY)
      saveConfiguration()
      addLog('Configuracao virtual invalida; padroes restaurados.')
    }
  }

  const getStation = (frequency) =>
    ESTACOES.find((station) => Math.abs(station.frequency - frequency) < 0.05)

  const simulatedRssi = (frequency) => {
    const station = getStation(frequency)
    if (station) return station.rssi
    return 12 + ((Math.round(frequency * 10) * 17) % 13)
  }

  const updateRssi = () => {
    currentRssi.value = simulatedRssi(radioFrequency.value)
  }

  const currentDate = () =>
    new Date(
      yearVal.value,
      monthVal.value - 1,
      dayVal.value,
      hourVal.value,
      minuteVal.value,
      secondVal.value
    )

  const applyDate = (date) => {
    yearVal.value = date.getFullYear()
    monthVal.value = date.getMonth() + 1
    dayVal.value = date.getDate()
    hourVal.value = date.getHours()
    minuteVal.value = date.getMinutes()
    secondVal.value = date.getSeconds()
  }

  const advanceClock = (seconds) => {
    const date = currentDate()
    date.setSeconds(date.getSeconds() + seconds)
    applyDate(date)
  }

  const isWeekend = () => {
    const dayOfWeek = currentDate().getDay()
    return dayOfWeek === 0 || dayOfWeek === 6
  }

  const cancelRadioScan = () => {
    if (scanTimer) window.clearInterval(scanTimer)
    scanTimer = null
    isScanning.value = false
  }

  const scanStep = (direction) => {
    radioFrequency.value = avancarFrequencia(radioFrequency.value, direction)
    updateRssi()
    scanSteps++

    if (currentRssi.value >= LIMIAR_RSSI || scanSteps >= MAX_PASSOS_BUSCA) {
      cancelRadioScan()
      const station = getStation(radioFrequency.value)
      addLog(
        station
          ? `Estacao encontrada: ${radioFrequency.value.toFixed(1)} MHz (${station.name}, RSSI ${currentRssi.value}).`
          : `Busca encerrada apos ${scanSteps} passos, sem estacao forte.`
      )
    }
  }

  const startRadioScan = (direction) => {
    if (alarmRinging.value || mode.value !== 'radio' || radioMode.value !== 'tune') return
    cancelRadioScan()
    scanSteps = 0
    isScanning.value = true
    addLog(`Busca FM iniciada em passos de 0,2 MHz (${direction > 0 ? 'subindo' : 'descendo'}).`)
    scanStep(direction)
    if (isScanning.value) {
      scanTimer = window.setInterval(() => scanStep(direction), 100)
    }
  }

  const finishChime = () => {
    if (!chimeActive.value) return
    chimeActive.value = false
    audioState.value = radioWasActiveBeforeChime ? 'radio' : 'free'
    addLog('Chime finalizado apos 2 segundos.')
  }

  const cancelChime = () => {
    if (chimeTimer) window.clearTimeout(chimeTimer)
    chimeTimer = null
    if (chimeActive.value) finishChime()
  }

  const triggerChime = (forced = false) => {
    if (!forced && mode.value === 'setup') return false
    if (alarmRinging.value || inSnooze.value || chimeActive.value ||
        ['alarm-mp3', 'alarm-radio'].includes(audioState.value)) {
      return false
    }

    cancelRadioScan()
    radioWasActiveBeforeChime = mode.value === 'radio'
    chimeActive.value = true
    audioState.value = 'chime'
    addLog(`Chime: faixa ${chimeTrack.value}, volume ${chimeVolume.value}, duracao 2 s.`)
    chimeTimer = window.setTimeout(finishChime, DURACAO_CHIME_MS)
    return true
  }

  const stopAlarmAudio = () => {
    audioState.value = 'free'
    if (mode.value === 'radio') mode.value = 'clock'
  }

  const triggerAlarm = (fromSnooze = false) => {
    cancelRadioScan()
    cancelChime()
    if (snoozeTimer) window.clearInterval(snoozeTimer)
    snoozeTimer = null
    inSnooze.value = false
    alarmRinging.value = true
    alarmStartedAt = Date.now()

    if (alarmSource.value === 'radio') {
      mode.value = 'radio'
      radioMode.value = 'tune'
      volumeRadio.value = 5
      audioState.value = 'alarm-radio'
      updateRssi()
    } else {
      mode.value = 'clock'
      audioState.value = 'alarm-mp3'
    }

    addLog(
      `${fromSnooze ? 'Fim da soneca' : 'Despertador disparou'}: ` +
      `${alarmSource.value === 'radio' ? `FM ${radioFrequency.value.toFixed(1)} MHz` : `MP3 faixa ${alarmTrack.value}`}.`
    )
  }

  const cancelAlarm = (automatic = false) => {
    if (!alarmRinging.value && !inSnooze.value) return
    alarmRinging.value = false
    inSnooze.value = false
    if (snoozeTimer) window.clearInterval(snoozeTimer)
    snoozeTimer = null
    stopAlarmAudio()
    saveConfiguration()
    if (!automatic) beep(1000, 500)
    addLog(automatic ? 'Alarme encerrado automaticamente apos 30 minutos.' : 'Alarme cancelado pelo usuario.')
  }

  const updateSnooze = () => {
    const remainingMs = snoozeEndsAt - Date.now()
    snoozeTimeLeft.value = Math.max(0, Math.ceil(remainingMs / 1000))
    if (remainingMs <= 0) {
      window.clearInterval(snoozeTimer)
      snoozeTimer = null
      triggerAlarm(true)
    }
  }

  const activateSnooze = () => {
    if (!alarmRinging.value && !inSnooze.value) return
    alarmRinging.value = false
    inSnooze.value = true
    stopAlarmAudio()
    snoozeEndsAt = Date.now() + DURACAO_SONECA_MS
    snoozeTimeLeft.value = 300
    if (snoozeTimer) window.clearInterval(snoozeTimer)
    snoozeTimer = window.setInterval(updateSnooze, 250)
    beepDouble()
    addLog('Soneca ativada ou reiniciada por 5 minutos.')
  }

  const checkAlarmTrigger = () => {
    if (!alarmEnabled.value || alarmRinging.value || inSnooze.value ||
        hourVal.value !== alarmHour.value || minuteVal.value !== alarmMinute.value) {
      return
    }

    const key = `${dataKey(yearVal.value, monthVal.value, dayVal.value)}-${pad2(hourVal.value)}:${pad2(minuteVal.value)}`
    if (key === lastAlarmKey) return
    lastAlarmKey = key

    if (!alarmFds.value && isWeekend()) {
      addLog('Alarme ignorado: configurado somente para dias uteis.')
      return
    }
    triggerAlarm(false)
  }

  const checkChime = () => {
    if (minuteVal.value !== 0 || secondVal.value !== 0 || mode.value === 'setup') return
    const key = `${dataKey(yearVal.value, monthVal.value, dayVal.value)}-${hourVal.value}`
    if (key === lastChimeKey) return
    lastChimeKey = key
    triggerChime(false)
  }

  const finishSlotMachine = () => {
    if (slotTimer) window.clearInterval(slotTimer)
    slotTimer = null
    isSlotMachine.value = false
  }

  const triggerSlotMachine = () => {
    if (isSlotMachine.value) return
    isSlotMachine.value = true
    const startedAt = performance.now()
    addLog('Protecao de catodo iniciada por 2 segundos.')

    slotTimer = window.setInterval(() => {
      const elapsed = performance.now() - startedAt
      if (elapsed >= 2000) {
        finishSlotMachine()
        return
      }

      slotDigits.value = [
        elapsed < 1000 ? Math.floor(elapsed / 50) % 10 : (hourVal.value < 10 ? -1 : Math.floor(hourVal.value / 10)),
        elapsed < 1333 ? (Math.floor(elapsed / 50) + 3) % 10 : hourVal.value % 10,
        elapsed < 1666 ? (Math.floor(elapsed / 50) + 6) % 10 : Math.floor(minuteVal.value / 10),
        (Math.floor(elapsed / 50) + 9) % 10
      ]
    }, 50)
  }

  const checkSlotMachine = () => {
    if (mode.value !== 'clock' || alarmRinging.value || inSnooze.value ||
        minuteVal.value % 15 !== 0 || secondVal.value !== 0) {
      return
    }
    const key = `${dataKey(yearVal.value, monthVal.value, dayVal.value)}-${hourVal.value}:${minuteVal.value}`
    if (key === lastSlotKey) return
    lastSlotKey = key
    triggerSlotMachine()
  }

  const tickClock = () => {
    if (mode.value !== 'setup') {
      advanceClock(timeMultiplier.value)
      checkAlarmTrigger()
      checkChime()
      checkSlotMachine()
    }

    if (alarmRinging.value && Date.now() - alarmStartedAt >= DURACAO_MAXIMA_ALARME_MS) {
      cancelAlarm(true)
    }
  }

  const previewAlarm = () => {
    audioState.value = 'preview'
    addLog(`Previa MP3 do alarme: faixa ${alarmTrack.value}, volume ${alarmMp3Vol.value}.`)
  }

  const previewChime = () => {
    audioState.value = 'preview'
    addLog(`Previa MP3 do chime: faixa ${chimeTrack.value}, volume ${chimeVolume.value}.`)
  }

  const stopPreview = () => {
    if (audioState.value === 'preview') audioState.value = 'free'
  }

  const adjustBrightness = (direction) => {
    const before = brightness.value
    brightness.value = Math.max(
      BRILHO_DESLIGADO,
      Math.min(BRILHO_MAXIMO, brightness.value + (direction > 0 ? 1 : -1))
    )
    if (brightness.value !== before) {
      beep(1000, 15)
      addLog(`Brilho Nixie ajustado para ${brightness.value}.`)
    }
  }

  const adjustSetup = (direction) => {
    beep(1000, 20)

    if (setupScreen.value === SETUP_HORARIO) {
      if (setupField.value === 0) hourVal.value = cyclic(hourVal.value, 0, 23, direction)
      else minuteVal.value = cyclic(minuteVal.value, 0, 59, direction)
      return
    }

    if (setupScreen.value === SETUP_ALARME) {
      if (setupField.value === 0) alarmHour.value = cyclic(alarmHour.value, 0, 23, direction)
      else if (setupField.value === 1) alarmMinute.value = cyclic(alarmMinute.value, 0, 59, direction)
      else if (setupField.value === 2) {
        let activation = alarmEnabled.value ? (alarmFds.value ? 2 : 1) : 0
        activation = cyclic(activation, 0, 2, direction)
        alarmEnabled.value = activation !== 0
        alarmFds.value = activation === 2
      } else {
        alarmSource.value = alarmSource.value === 'radio' ? 'mp3' : 'radio'
      }
      return
    }

    if (setupScreen.value === SETUP_DATA) {
      if (setupField.value === 0) {
        dayVal.value = cyclic(dayVal.value, 1, diasNoMes(monthVal.value, yearVal.value), direction)
      } else if (setupField.value === 1) {
        monthVal.value = cyclic(monthVal.value, 1, 12, direction)
        dayVal.value = limitarDiaAoMes(dayVal.value, monthVal.value, yearVal.value)
      } else {
        yearVal.value = cyclic(yearVal.value, 2000, 2099, direction)
        dayVal.value = limitarDiaAoMes(dayVal.value, monthVal.value, yearVal.value)
      }
      return
    }

    if (setupField.value === 0) {
      alarmTrack.value = cyclic(alarmTrack.value, 1, 99, direction)
      previewAlarm()
    } else if (setupField.value === 1) {
      alarmMp3Vol.value = cyclic(alarmMp3Vol.value, 0, 30, direction)
      previewAlarm()
    } else if (setupField.value === 2) {
      chimeTrack.value = cyclic(chimeTrack.value, 1, 99, direction)
      previewChime()
    } else {
      chimeVolume.value = cyclic(chimeVolume.value, 0, 30, direction)
      previewChime()
    }
  }

  const rotate = (direction) => {
    const normalized = direction > 0 ? 1 : -1
    if (mode.value === 'setup') {
      adjustSetup(normalized)
    } else if (mode.value === 'clock') {
      adjustBrightness(normalized)
    } else if (radioMode.value === 'clock') {
      adjustBrightness(normalized)
    } else if (radioMode.value === 'volume') {
      volumeRadio.value = Math.max(0, Math.min(15, volumeRadio.value + normalized))
      addLog(`Volume do radio ajustado para ${volumeRadio.value}.`)
    } else {
      startRadioScan(normalized)
    }
  }

  const enterSetup = () => {
    cancelChime()
    cancelRadioScan()
    mode.value = 'setup'
    setupScreen.value = SETUP_HORARIO
    setupField.value = 0
    beepDouble()
    addLog('Setup iniciado: tela Horario, campo HH.')
  }

  const saveAndExit = () => {
    stopPreview()
    dayVal.value = limitarDiaAoMes(dayVal.value, monthVal.value, yearVal.value)
    secondVal.value = 0
    setupScreen.value = SETUP_HORARIO
    setupField.value = 0
    mode.value = 'clock'
    saveConfiguration()
    beepDouble()
    addLog('Horario, data e configuracoes gravados; LCD desligado.')
  }

  const advanceSetup = () => {
    const lastField = PASSOS_SETUP[setupScreen.value] - 1
    if (setupField.value < lastField) {
      setupField.value++
      beep(1000, 30)
      if (setupScreen.value === SETUP_SONS && setupField.value === 2) previewChime()
      return
    }

    if (setupScreen.value === SETUP_SONS) {
      saveAndExit()
      return
    }

    stopPreview()
    setupScreen.value++
    setupField.value = 0
    beep(1000, 30)
    if (setupScreen.value === SETUP_SONS) previewAlarm()
  }

  const triggerShortClick = () => {
    if (alarmRinging.value || inSnooze.value) {
      cancelAlarm(false)
      return
    }

    if (mode.value === 'clock') {
      enterSetup()
    } else if (mode.value === 'radio') {
      cancelRadioScan()
      beep(1000, 70)
      radioMode.value = radioMode.value === 'tune'
        ? 'volume'
        : radioMode.value === 'volume'
          ? 'clock'
          : 'tune'
      addLog(`Modo do radio alterado para ${radioMode.value.toUpperCase()}.`)
    } else {
      advanceSetup()
    }
  }

  const triggerLongPress = () => {
    if (alarmRinging.value || inSnooze.value) {
      activateSnooze()
      return
    }

    if (mode.value === 'setup') {
      saveAndExit()
    } else if (mode.value === 'clock') {
      cancelChime()
      mode.value = 'radio'
      radioMode.value = 'tune'
      updateRssi()
      audioState.value = 'radio'
      addLog(`Radio ligado em ${radioFrequency.value.toFixed(1)} MHz, modo SCA.`)
    } else {
      cancelChime()
      cancelRadioScan()
      mode.value = 'clock'
      audioState.value = 'free'
      saveConfiguration()
      beep(1000, 70)
      addLog('Radio desligado; frequencia gravada.')
    }
  }

  const handleButtonDown = () => {
    if (isPressed.value) return
    isPressed.value = true
    longPressHandled = false
    longPressTimer = window.setTimeout(() => {
      longPressHandled = true
      triggerLongPress()
    }, 700)
  }

  const handleButtonUp = () => {
    if (!isPressed.value) return
    isPressed.value = false
    if (longPressTimer) window.clearTimeout(longPressTimer)
    longPressTimer = null
    if (!longPressHandled) triggerShortClick()
  }

  const handleButtonCancel = () => {
    if (!isPressed.value) return
    isPressed.value = false
    if (longPressTimer) window.clearTimeout(longPressTimer)
    longPressTimer = null
    longPressHandled = false
  }

  const getAngle = (x, y, element) => {
    const rect = element.getBoundingClientRect()
    return Math.atan2(
      y - (rect.top + rect.height / 2),
      x - (rect.left + rect.width / 2)
    ) * (180 / Math.PI)
  }

  const handleKnobStart = (event) => {
    if (event.target.classList.contains('encoder-center')) return
    isDragging = true
    startAngle = getAngle(event.clientX, event.clientY, event.currentTarget)
    window.addEventListener('pointermove', handleKnobMove)
    window.addEventListener('pointerup', handleKnobEnd)
  }

  const handleKnobMove = (event) => {
    if (!isDragging) return
    const knob = document.querySelector('.encoder-ring')
    if (!knob) return
    const currentAngle = getAngle(event.clientX, event.clientY, knob)
    let difference = currentAngle - startAngle
    if (difference > 180) difference -= 360
    if (difference < -180) difference += 360

    const ticks = Math.trunc(difference / 15)
    if (ticks === 0) return
    const direction = ticks > 0 ? 1 : -1
    for (let index = 0; index < Math.abs(ticks); index++) rotate(direction)
    startAngle = currentAngle
    knobRotation.value = (knobRotation.value + ticks * 15) % 360
  }

  const handleKnobEnd = () => {
    isDragging = false
    window.removeEventListener('pointermove', handleKnobMove)
    window.removeEventListener('pointerup', handleKnobEnd)
  }

  const handleWheel = (event) => {
    rotate(event.deltaY > 0 ? 1 : -1)
  }

  const setTimeMultiplier = (multiplier) => {
    timeMultiplier.value = multiplier
    addLog(`Velocidade do RTC virtual: ${multiplier}x.`)
  }

  const toggleAlarmQuick = (event) => {
    alarmEnabled.value = event.target.checked
    saveConfiguration()
    addLog(`Alarme ${alarmEnabled.value ? 'ativado' : 'desativado'} pelo painel de teste.`)
  }

  const clearLogs = () => {
    serialLogs.value = []
  }

  const temperatureC = computed({
    get: () => temperatureTenths.value / 10,
    set: (value) => {
      const normalized = Math.max(0, Math.min(99.9, Number(value) || 0))
      temperatureTenths.value = Math.round(normalized * 10)
    }
  })

  const showTemperature = computed(() =>
    mode.value === 'clock' &&
    brightness.value !== BRILHO_DESLIGADO &&
    !alarmRinging.value &&
    !inSnooze.value &&
    !isSlotMachine.value &&
    secondVal.value % INTERVALO_TEMPERATURA >= INTERVALO_TEMPERATURA - DURACAO_TEMPERATURA
  )

  const displayForcedOn = computed(() =>
    mode.value === 'setup' || alarmRinging.value || inSnooze.value
  )

  const displayFullyOff = computed(() =>
    brightness.value === BRILHO_DESLIGADO && !displayForcedOn.value
  )

  const nixieIntensity = computed(() => {
    if (displayFullyOff.value) return 0
    let effective = brightness.value
    if (displayForcedOn.value && effective < 0) effective = 6
    if (effective < 0) effective = 0
    return Math.max(0.12, Math.min(1, (effective + 1) / 11))
  })

  const baseLedGlow = computed(() => {
    if (displayFullyOff.value) return false
    if (mode.value === 'setup') return true
    if (mode.value === 'radio') {
      if (radioMode.value === 'volume') return true
      if (radioMode.value !== 'clock') return false
    }
    if (showTemperature.value) return false
    if (brightness.value >= 0 || alarmRinging.value || inSnooze.value) {
      return secondsLedState.value
    }
    return false
  })

  const setupActivation = computed(() =>
    alarmEnabled.value ? (alarmFds.value ? 2 : 1) : 0
  )

  const normalTimeDigits = () => [
    hourVal.value < 10 ? -1 : Math.floor(hourVal.value / 10),
    hourVal.value % 10,
    Math.floor(minuteVal.value / 10),
    minuteVal.value % 10
  ]

  const setupDigits = () => {
    let digits
    if (setupScreen.value === SETUP_HORARIO) {
      digits = normalTimeDigits()
    } else if (setupScreen.value === SETUP_ALARME) {
      if (setupField.value <= 1) {
        digits = [
          alarmHour.value < 10 ? -1 : Math.floor(alarmHour.value / 10),
          alarmHour.value % 10,
          Math.floor(alarmMinute.value / 10),
          alarmMinute.value % 10
        ]
      } else if (setupField.value === 2) {
        digits = [-1, setupActivation.value, -1, -1]
      } else {
        digits = [-1, -1, -1, alarmSource.value === 'radio' ? 0 : 1]
      }
    } else if (setupScreen.value === SETUP_DATA) {
      if (setupField.value <= 1) {
        digits = [
          dayVal.value < 10 ? -1 : Math.floor(dayVal.value / 10),
          dayVal.value % 10,
          monthVal.value < 10 ? -1 : Math.floor(monthVal.value / 10),
          monthVal.value % 10
        ]
      } else {
        digits = [2, 0, Math.floor((yearVal.value % 100) / 10), yearVal.value % 10]
      }
    } else {
      const track = setupField.value <= 1 ? alarmTrack.value : chimeTrack.value
      const volume = setupField.value <= 1 ? alarmMp3Vol.value : chimeVolume.value
      digits = [
        track < 10 ? -1 : Math.floor(track / 10),
        track % 10,
        Math.floor(volume / 10),
        volume % 10
      ]
    }

    if (blinkState.value) return digits
    const blank = [...digits]
    if (setupScreen.value === SETUP_HORARIO || setupScreen.value === SETUP_ALARME) {
      if (setupField.value === 0) blank.splice(0, 2, -1, -1)
      else if (setupField.value === 1) blank.splice(2, 2, -1, -1)
      else if (setupField.value === 2) blank[1] = -1
      else blank[3] = -1
    } else if (setupScreen.value === SETUP_DATA) {
      if (setupField.value === 0) blank.splice(0, 2, -1, -1)
      else blank.splice(2, 2, -1, -1)
    } else if (setupField.value === 0 || setupField.value === 2) {
      blank.splice(0, 2, -1, -1)
    } else {
      blank.splice(2, 2, -1, -1)
    }
    return blank
  }

  const nixieDigits = computed(() => {
    if (displayFullyOff.value) return [-1, -1, -1, -1]
    if (isSlotMachine.value) return slotDigits.value
    if (inSnooze.value) {
      const minutes = Math.floor(snoozeTimeLeft.value / 60)
      const seconds = snoozeTimeLeft.value % 60
      return [Math.floor(minutes / 10), minutes % 10, Math.floor(seconds / 10), seconds % 10]
    }
    if (showTemperature.value) {
      const temperature = temperatureTenths.value
      return [-1, temperature >= 100 ? Math.floor(temperature / 100) : -1, Math.floor(temperature / 10) % 10, temperature % 10]
    }
    if (mode.value === 'clock') return normalTimeDigits()
    if (mode.value === 'setup') return setupDigits()
    if (radioMode.value === 'clock') {
      return [Math.floor(hourVal.value / 10), hourVal.value % 10, Math.floor(minuteVal.value / 10), minuteVal.value % 10]
    }
    if (radioMode.value === 'volume') {
      return [
        currentRssi.value < 10 ? -1 : Math.floor(currentRssi.value / 10),
        currentRssi.value % 10,
        volumeRadio.value < 10 ? 0 : Math.floor(volumeRadio.value / 10),
        volumeRadio.value % 10
      ]
    }
    const frequency = Math.round(radioFrequency.value * 10)
    return [
      frequency >= 1000 ? 1 : -1,
      Math.floor(frequency / 100) % 10,
      Math.floor(frequency / 10) % 10,
      frequency % 10
    ]
  })

  const getDayName = () => {
    const weekdays = ['Domingo', 'Segunda', 'Terca', 'Quarta', 'Quinta', 'Sexta', 'Sabado']
    return weekdays[currentDate().getDay()]
  }

  const blinkLcdField = (line) => {
    if (blinkState.value) return pad16(line)
    const chars = pad16(line).split('')
    let start = -1
    let length = 0
    if (setupScreen.value === SETUP_HORARIO) {
      start = setupField.value === 0 ? 10 : 13
      length = 2
    } else if (setupScreen.value === SETUP_ALARME) {
      const ranges = [[0, 2], [3, 2], [6, 6], [13, 3]]
      ;[start, length] = ranges[setupField.value]
    } else if (setupScreen.value === SETUP_DATA) {
      const ranges = [[0, 2], [3, 2], [6, 2]]
      ;[start, length] = ranges[setupField.value]
    } else {
      const ranges = [[1, 2], [5, 2], [10, 2], [14, 2]]
      ;[start, length] = ranges[setupField.value]
    }
    for (let index = 0; index < length; index++) chars[start + index] = ' '
    return chars.join('')
  }

  const lcdLine1 = computed(() => {
    if (mode.value !== 'setup') return pad16('LCD desligado')
    return [
      'Horario : HH:MM',
      'Alarme Atv    Tp',
      'Data/ano/semana',
      'Alarme e Chime'
    ][setupScreen.value]
  })

  const lcdLine2 = computed(() => {
    if (mode.value !== 'setup') return pad16('')
    let line
    if (setupScreen.value === SETUP_HORARIO) {
      line = `          ${pad2(hourVal.value)}:${pad2(minuteVal.value)}`
    } else if (setupScreen.value === SETUP_ALARME) {
      const activation = alarmEnabled.value ? (alarmFds.value ? 'Sim/FS' : ' Sim  ') : ' Nao  '
      line = `${pad2(alarmHour.value)}:${pad2(alarmMinute.value)} ${activation} ${alarmSource.value === 'radio' ? 'Fm' : 'mp3'}`
    } else if (setupScreen.value === SETUP_DATA) {
      line = `${pad2(dayVal.value)}/${pad2(monthVal.value)}/${pad2(yearVal.value % 100)} ${getDayName()}`
    } else {
      line = `S${pad2(alarmTrack.value)} v${pad2(alarmMp3Vol.value)}  S${pad2(chimeTrack.value)} v${pad2(chimeVolume.value)}`
    }
    return blinkLcdField(line)
  })

  const showColon = computed(() => baseLedGlow.value)
  const audioActive = computed(() => mode.value === 'radio' || audioState.value !== 'free')
  const brightnessLabel = computed(() => {
    if (brightness.value === BRILHO_DESLIGADO) return 'Nixies desligadas'
    if (brightness.value === BRILHO_SEM_PISCA) return 'Minimo sem LED'
    return `Brilho ${brightness.value}/10`
  })
  const setupLabel = computed(() => {
    const names = ['Horario', 'Alarme', 'Data', 'Sons']
    return `${names[setupScreen.value]} ${setupField.value + 1}/${PASSOS_SETUP[setupScreen.value]}`
  })

  onMounted(() => {
    loadConfiguration()
    updateRssi()
    addLog('Nano virtual online. RTC, DFPlayer e RDA5807 inicializados.')
    clockTimer = window.setInterval(tickClock, 1000)
    blinkTimer = window.setInterval(() => { blinkState.value = !blinkState.value }, 300)
    secondsLedTimer = window.setInterval(() => { secondsLedState.value = !secondsLedState.value }, 500)
  })

  onBeforeUnmount(() => {
    for (const timer of [clockTimer, blinkTimer, secondsLedTimer, snoozeTimer, scanTimer, slotTimer]) {
      if (timer) window.clearInterval(timer)
    }
    if (chimeTimer) window.clearTimeout(chimeTimer)
    if (longPressTimer) window.clearTimeout(longPressTimer)
    handleKnobEnd()
  })

  return {
    mode,
    radioMode,
    setupScreen,
    setupField,
    hourVal,
    minuteVal,
    secondVal,
    alarmEnabled,
    alarmRinging,
    inSnooze,
    snoozeTimeLeft,
    brightness,
    brightnessLabel,
    volumeRadio,
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
  }
}
