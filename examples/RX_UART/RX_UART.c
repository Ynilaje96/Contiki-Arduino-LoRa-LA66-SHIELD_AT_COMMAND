#include "contiki.h"
#include "serial-line.h"
#include "software_uart_serial_line.h"
#include <stdio.h>
#include <string.h>

#define DEBUG_PRINTF(...) do { \
  printf("[%lu] ", clock_time()); \
  printf(__VA_ARGS__); \
} while(0)

PROCESS(receiver_process, "UART Receiver");
AUTOSTART_PROCESSES(&receiver_process);

PROCESS_THREAD(receiver_process, ev, data)
{
  static int msg_count = 0;

  PROCESS_BEGIN();

  DEBUG_PRINTF("Receiver iniciando...\n");

  soft_uart_init();
  DEBUG_PRINTF("UART inicializada (RX no pino 10)\n");

  soft_uart_write_string("RECEIVER_START\n");
  DEBUG_PRINTF("Sinal de startup enviado\n");

  serial_line_init();
  DEBUG_PRINTF("Serial-line pronto\n");

  while(1) {
    PROCESS_WAIT_EVENT();

    DEBUG_PRINTF("Evento recebido: %d\n", ev);

    if(ev == serial_line_event_message) {
      char *msg = (char *)data;
      msg_count++;

      DEBUG_PRINTF("Msg #%d: %s\n", msg_count, msg);
      DEBUG_PRINTF("Tamanho: %d bytes\n", strlen(msg));

      char ack[50];
      snprintf(ack, sizeof(ack), "RECEBIDO_%d:%s\n", msg_count, msg);
      soft_uart_write_string(ack);

      DEBUG_PRINTF("ACK enviado: %s\n", ack);
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
