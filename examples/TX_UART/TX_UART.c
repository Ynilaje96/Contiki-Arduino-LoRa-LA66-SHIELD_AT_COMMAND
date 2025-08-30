#include "contiki.h"
#include "software_uart_serial_line.h"
#include "serial-line.h"
#include <stdio.h>
#include <string.h>
#include "sys/rtimer.h"

#define SOFT_UART_TX_PIN  11  // TX no D11 (PB3)
#define SOFT_UART_RX_PIN  10  // RX no D10 (PB2) - opcional

PROCESS(uart_sender_process, "UART Sender Debug");
AUTOSTART_PROCESSES(&uart_sender_process);

PROCESS_THREAD(uart_sender_process, ev, data) {
  static struct etimer timer;
  static int counter = 0;
  static char message[64];

  PROCESS_BEGIN();

  printf("DBG_SENDER| Iniciando... (TX: pino %d, RX: pino %d)\n", 
         SOFT_UART_TX_PIN, SOFT_UART_RX_PIN);

  soft_uart_init(); // ADICIONADO: inicializa UART software
  serial_line_init();

  printf("DBG_SENDER| UART + Serial-line prontos | Time: %u\n", RTIMER_NOW());

  etimer_set(&timer, CLOCK_SECOND * 3);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER && etimer_expired(&timer)) {
      counter++;
      snprintf(message, sizeof(message), "S_%d", counter);

      printf("DBG_SENDER| Enviando: '%s' | Time: %u\n", message, RTIMER_NOW());

      if(soft_uart_write_string(message) == 0) {
        printf("DBG_SENDER| Envio OK | Time: %u\n", RTIMER_NOW());
      } else {
        printf("DBG_SENDER| Falha no envio | Time: %u\n", RTIMER_NOW());
      }

      etimer_reset(&timer);
    }
  }

  PROCESS_END();
}

// Redireciona printf para UART software
/*
int putcharec(int c) {
  soft_uart_write_byte((uint8_t)c);
  return c;
}
*/