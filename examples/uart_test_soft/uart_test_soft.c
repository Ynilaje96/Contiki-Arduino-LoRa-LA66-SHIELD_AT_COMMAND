#include "contiki.h"
#include "software_uart.h" // Inclui a Software UART para D2/D3
#include <stdio.h>     // Para printf (ainda usando a UART0 para debug no PC)

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// O baud rate da Soft UART é definido em soft_uart.h como SOFT_UART_BAUD_RATE

/*---------------------------------------------------------------------------*/
PROCESS(uart_test_process, "UART Test Process (D2/D3)"); // Renomeado para clareza
AUTOSTART_PROCESSES(&uart_test_process);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(uart_test_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  // 1. Inicializa a Software UART (D2/D3)
  soft_uart_init(); // Não passa mais o baud rate aqui
  printf("Software UART (D2/D3) inicializada com %u baud.\n", (unsigned int)SOFT_UART_BAUD_RATE); // Usa a macro de soft_uart.h

  etimer_set(&et, CLOCK_SECOND * 2); // Espera 2 segundos antes de começar

  while(1) {
    PROCESS_WAIT_EVENT();

    // Teste de Recepção e Echo na Software UART (D2/D3)
    if(ev == PROCESS_EVENT_POLL) {
      if (soft_uart_data_ready()) { // Verifica se há dados na Software UART
        // ATENÇÃO: soft_uart_read_byte() é BLOQUEANTE!
        // Isso pode causar problemas em aplicações Contiki mais complexas.
        uint8_t received_byte = soft_uart_read_byte();
        soft_uart_write_byte(received_byte); // Ecoa o byte de volta na Software UART
        // Imprime a informação no terminal serial do PC (via UART0)
        printf("Soft UART Recebido: %c (0x%02X), Enviando de volta.\n", received_byte, received_byte);
      }
    }

    // Teste de Transmissão Periódica na Software UART (D2/D3)
    if(ev == PROCESS_EVENT_TIMER) {
      if(etimer_expired(&et)) {
        soft_uart_write_string("Hello from Contiki Software UART (D2/D3)!\r\n");
        printf("Soft UART Enviado: \"Hello from Contiki Software UART (D2/D3)!\"\n");
        etimer_reset(&et);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/