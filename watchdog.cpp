#include <Arduino.h>
#include <avr/wdt.h>

// Executa antes da inicializacao das variaveis globais. Isso evita um ciclo de
// resets caso o microcontrolador reinicie por watchdog.
void desativarWatchdogNoBoot()
    __attribute__((naked)) __attribute__((section(".init3")));

void desativarWatchdogNoBoot() {
  MCUSR = 0;
  wdt_disable();
}

void iniciarWatchdog() {
  wdt_reset();
  wdt_enable(WDTO_4S);
}

void alimentarWatchdog() {
  wdt_reset();
}
