// uart_test_serial_line.c
#include "contiki.h"
#include "software_uart_serial_line.h"        // Nossa Software UART para D2/D3
#include "serial-line.h"  // O processo serial-line do Contiki
#include <stdio.h>            // Para printf

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/*---------------------------------------------------------------------------*/
PROCESS(soft_uart_serial_line_test_process, "Software UART Serial Line Test Process");
AUTOSTART_PROCESSES(&soft_uart_serial_line_test_process);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(soft_uart_serial_line_test_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  // 1. Inicializa a Software UART (D10/D11)
  soft_uart_init();
  printf("Software UART (D10/D11) inicializada com %u baud.\n", (unsigned int)SOFTWARE_UART_BAUD_RATE);

  // 2. Inicializa o processo serial-line do Contiki
  // serial_line_init() eh chamado pelo Contiki se SERIAL_LINE_INPUT_HANDLER eh definido,
  // ou por uma plataforma especifica. Chamamos aqui para garantir que esteja pronto.
  serial_line_init();
  printf("Contiki serial-line process iniciado.\n");

  etimer_set(&et, CLOCK_SECOND * 3); // Espera 3 segundos antes de começar a enviar

  while(1) {
    PROCESS_WAIT_EVENT();

    // Evento de linha serial recebida via serial-line.c
    if(ev == serial_line_event_message) {
      char *line = (char *)data;
      printf("Serial-line (D10/D11) RECEBIDO: '%s'\n", line);
      // Ecoa a linha de volta na Software UART
      soft_uart_write_string("Echo: ");
      soft_uart_write_string(line);
      soft_uart_write_string("\r\n"); // Adiciona nova linha para facilitar a leitura no terminal externo
    }

    // Transmissão periódica via Software UART (D2/D3)
    if(ev == PROCESS_EVENT_TIMER) {
      if(etimer_expired(&et)) {
        soft_uart_write_string("Hello from Contiki Software UART (D10/D11)!\r\n");
        printf("Soft UART Enviado: \"Hello from Contiki Software UART (D10/D11)!\"\n");
        etimer_reset(&et);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/