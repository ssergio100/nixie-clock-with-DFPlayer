<script setup>
import { computed, onBeforeUnmount, onMounted, ref } from 'vue'

const now = ref(new Date())
const mode = ref('clock')
const radioMode = ref('tune')
const setupStep = ref(0)
const setupField = ref(0)
const screenSaver = ref(false)
const brightness = ref(6)
const volumeRadio = ref(5)
const station = ref(106.3)
const alarmHour = ref(7)
const alarmMinute = ref(0)
const alarmEnabled = ref(true)
const alarmSource = ref('radio')
const year = ref(2026)
const month = ref(6)
const day = ref(22)
const editHour = ref(18)
const editMinute = ref(30)
const selectedTrack = ref(1)
const chimeTrack = ref(2)
const chimeVolume = ref(15)

let timer = null
let holdTimer = null
let pressStart = 0
let lastClickAt = 0

const clamp = (value, min, max) => Math.min(max, Math.max(min, value))

onMounted(() => {
  timer = setInterval(() => {
    now.value = new Date()
  }, 1000)
})

onBeforeUnmount(() => {
  if (timer) clearInterval(timer)
  if (holdTimer) clearTimeout(holdTimer)
})

const setupLabels = [
  'Ajustar Relógio',
  'Hora do Alarme',
  'Modo do Alarme',
  'Ajuste Data',
  'Ajuste Ano',
  'Alarme Fim de Semana',
  'Som do Alarme',
  'Som da Hora Cheia'
]

const currentTime = computed(() => ({
  hour: now.value.getHours(),
  minute: now.value.getMinutes(),
  second: now.value.getSeconds()
}))

const displayValues = computed(() => {
  if (mode.value === 'clock' && !screenSaver.value) {
    return {
      hour: currentTime.value.hour,
      minute: currentTime.value.minute,
      showColon: true,
      type: 'time'
    }
  }

  if (mode.value === 'radio') {
    if (radioMode.value === 'tune') {
      const stationText = station.value.toFixed(1).replace('.', '')
      return {
        hour: Number(stationText[0] || 0),
        minute: Number(stationText.slice(1) || 0),
        showColon: false,
        type: 'radio'
      }
    }

    return {
      hour: Math.floor(volumeRadio.value / 10),
      minute: volumeRadio.value % 10,
      showColon: false,
      type: 'volume'
    }
  }

  if (mode.value === 'setup') {
    if (setupStep.value === 0) {
      return {
        hour: editHour.value,
        minute: editMinute.value,
        showColon: true,
        type: 'setup'
      }
    }

    if (setupStep.value === 1) {
      return {
        hour: alarmHour.value,
        minute: alarmMinute.value,
        showColon: true,
        type: 'setup'
      }
    }

    if (setupStep.value === 3) {
      return {
        hour: day.value,
        minute: month.value,
        showColon: false,
        type: 'setup'
      }
    }

    if (setupStep.value === 4) {
      return {
        hour: Math.floor(year.value / 100),
        minute: year.value % 100,
        showColon: false,
        type: 'setup'
      }
    }

    return {
      hour: setupStep.value,
      minute: 0,
      showColon: false,
      type: 'setup'
    }
  }

  return {
    hour: 0,
    minute: 0,
    showColon: true,
    type: 'idle'
  }
})

const digitValues = computed(() => {
  const hour = String(displayValues.value.hour).padStart(2, '0')
  const minute = String(displayValues.value.minute).padStart(2, '0')
  return `${hour}${minute}`.split('').map(Number)
})

const statusText = computed(() => {
  if (mode.value === 'radio') {
    return radioMode.value === 'tune' ? 'Modo Rádio - Sintonia' : 'Modo Rádio - Volume'
  }
  if (mode.value === 'setup') {
    return `Setup - ${setupLabels[setupStep.value]}`
  }
  if (screenSaver.value) return 'Screen Saver'
  return 'Modo Relógio'
})

const helperText = computed(() => {
  if (mode.value === 'radio') {
    return radioMode.value === 'tune'
      ? 'Gire para buscar estação (SCA)'
      : 'Gire para ajustar o volume (VOL)'
  }

  if (mode.value === 'setup') {
    if (setupStep.value === 0) {
      return setupField.value === 0
        ? 'Gire à esquerda/direita para ajustar a hora'
        : 'Gire à esquerda/direita para ajustar o minuto'
    }
    if (setupStep.value === 1) {
      return setupField.value === 0
        ? 'Gire para ajustar a hora do alarme'
        : 'Gire para ajustar o minuto do alarme'
    }
    if (setupStep.value === 2) {
      return 'Gire para alternar RADIO / MP3 e ON / OFF'
    }
    if (setupStep.value === 3) {
      return setupField.value === 0
        ? 'Gire para ajustar o dia'
        : 'Gire para ajustar o mês'
    }
    if (setupStep.value === 4) return 'Gire para ajustar o ano'
    if (setupStep.value === 5) return 'Gire para alternar fim de semana'
    if (setupStep.value === 6) return 'Gire para escolher a faixa do alarme'
    return 'Gire para escolher a faixa do chime'
  }

  if (screenSaver.value) {
    return 'Gire ou clique curto para acordar o relógio'
  }

  return 'Clique curto: setup | toque longo: rádio | clique duplo: screen saver'
})

const updateClockDraft = () => {
  editHour.value = now.value.getHours()
  editMinute.value = now.value.getMinutes()
}

const rotate = (direction) => {
  const step = direction > 0 ? 1 : -1

  if (mode.value === 'clock') {
    brightness.value = clamp(brightness.value + step, 1, 11)
    return
  }

  if (mode.value === 'radio') {
    if (radioMode.value === 'tune') {
      station.value = clamp(station.value + step * 0.1, 76.1, 107.9)
    } else {
      volumeRadio.value = clamp(volumeRadio.value + step, 0, 15)
    }
    return
  }

  if (mode.value === 'setup') {
    switch (setupStep.value) {
      case 0:
        if (setupField.value === 0) {
          editHour.value = clamp(editHour.value + step, 0, 23)
        } else {
          editMinute.value = clamp(editMinute.value + step, 0, 59)
        }
        break
      case 1:
        if (setupField.value === 0) {
          alarmHour.value = clamp(alarmHour.value + step, 0, 23)
        } else {
          alarmMinute.value = clamp(alarmMinute.value + step, 0, 59)
        }
        break
      case 2:
        if (step > 0) {
          alarmSource.value = alarmSource.value === 'radio' ? 'mp3' : 'radio'
        } else {
          alarmEnabled.value = !alarmEnabled.value
        }
        break
      case 3:
        if (setupField.value === 0) {
          day.value = clamp(day.value + step, 1, 31)
        } else {
          month.value = clamp(month.value + step, 1, 12)
        }
        break
      case 4:
        year.value = clamp(year.value + step, 2000, 2099)
        break
      case 5:
        alarmEnabled.value = !alarmEnabled.value
        break
      case 6:
        selectedTrack.value = clamp(selectedTrack.value + step, 1, 99)
        break
      case 7:
        chimeTrack.value = clamp(chimeTrack.value + step, 1, 99)
        break
      default:
        break
    }
  }
}

const beginEncoderPress = () => {
  pressStart = Date.now()
  if (holdTimer) clearTimeout(holdTimer)
  holdTimer = setTimeout(() => {
    if (mode.value === 'clock') {
      mode.value = 'radio'
      radioMode.value = 'tune'
    } else if (mode.value === 'radio') {
      mode.value = 'clock'
    } else if (mode.value === 'setup') {
      mode.value = 'clock'
    }
  }, 700)
}

const endEncoderPress = () => {
  if (holdTimer) {
    clearTimeout(holdTimer)
  }

  const elapsed = Date.now() - pressStart
  if (elapsed >= 700) return

  const nowClick = Date.now()
  if (mode.value === 'clock') {
    if (nowClick - lastClickAt < 300) {
      screenSaver.value = !screenSaver.value
    } else {
      mode.value = 'setup'
      setupStep.value = 0
      setupField.value = 0
      updateClockDraft()
    }
    lastClickAt = nowClick
  } else if (mode.value === 'radio') {
    radioMode.value = radioMode.value === 'tune' ? 'volume' : 'tune'
  } else if (mode.value === 'setup') {
    if (setupStep.value < setupLabels.length - 1) {
      setupStep.value += 1
    } else {
      mode.value = 'clock'
    }
  }
}

const onWheel = (event) => {
  rotate(event.deltaY > 0 ? 1 : -1)
}
</script>

<template>
  <main class="app-shell">
    <section class="clock-panel">
      <div class="clock-top">
        <div class="status-pill">{{ statusText }}</div>
        <div class="brightness-pill">Brilho {{ brightness }}</div>
      </div>

      <div class="display-frame">
        <div class="nixie-grid">
          <div v-for="(digit, index) in digitValues" :key="index" class="nixie-tube">
            <span class="nixie-digit">{{ digit }}</span>
          </div>
          <span v-if="displayValues.showColon" class="nixie-colon">:</span>
        </div>
      </div>

      <div class="helper-text">{{ helperText }}</div>

      <div class="encoder-wrap">
        <div class="encoder-label">Encoder</div>
        <div class="encoder-body">
          <div
            class="encoder-ring"
            @wheel.prevent="onWheel"
            @pointerdown="beginEncoderPress"
            @pointerup="endEncoderPress"
          >
            <div class="encoder-notch"></div>
            <button class="encoder-center" @click.prevent="endEncoderPress">OK</button>
          </div>
        </div>
      </div>
    </section>
  </main>
</template>
