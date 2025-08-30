#include "contiki.h"
#include "dev/uart1.h" // Para hardware UART no Arduino
#include <stdio.h>

PROCESS(la66_at_process, "LA66 AT Command Process");
AUTOSTART_PROCESSES(&la66_at_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(la66_at_process, ev, data)
{
  static struct etimer timer;
  static const char *at_ver = "AT+VER\r\n";
  
  PROCESS_BEGIN();

  // Inicializa UART (pinos 10-RX, 11-TX no Arduino)
  uart1_init(BAUD2UBR(9600)); // Configura para 9600 baud (padrão LA66)

  printf("System online. Sending AT commands to LA66...\n");

  while(1) {
    // Envia comando AT+VER
    printf("Sending: %s", at_ver);
    for(const char *p = at_ver; *p != '\0'; p++) {
      uart1_writeb(*p);
    }

    // Aguarda resposta (ajuste o tempo conforme necessário)
    etimer_set(&timer, CLOCK_SECOND * 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    // Lê resposta da UART
    while(uart1_data_available()) {
      unsigned char c = uart1_readb();
      printf("Received: %c (0x%02X)\n", c, c);
    }

    etimer_set(&timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  }

  PROCESS_END();
}