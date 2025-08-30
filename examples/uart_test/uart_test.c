#include "contiki.h"
#include "uart_avr.h"
#include <stdio.h>

// Defina a frequência do CPU (Arduino Uno: 16 MHz)
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Baud rate da UART
#define UART_BAUD_RATE 9600

// Processo do Contiki
PROCESS(uart_test_process, "UART Test Process Novo - Testando");
AUTOSTART_PROCESSES(&uart_test_process);

// Implementação do processo
PROCESS_THREAD(uart_test_process, ev, data) {
    static struct etimer timer;

    PROCESS_BEGIN();

    // Inicializa a UART
    uart_init(UART_BAUD_RATE);
    printf("UART inicializada com %u baud.\n", UART_BAUD_RATE);

    // Configura um timer de 2 segundos
    etimer_set(&timer, CLOCK_SECOND * 2);

    while (1) {
        PROCESS_WAIT_EVENT();

        // Ecoa dados recebidos
        if (ev == PROCESS_EVENT_POLL && uart_data_ready()) {
            uint8_t received_byte = uart_read_byte();
            uart_write_byte(received_byte); // Ecoa o byte
            printf("Recebido: %c (0x%02X)\n", received_byte, received_byte);
        }

        // Envia mensagem periódica
        if (ev == PROCESS_EVENT_TIMER && etimer_expired(&timer)) {
            uart_write_string("Hello from Arduino UART!\r\n");
            printf("Mensagem enviada.\n");
            etimer_reset(&timer);
        }
    }

    PROCESS_END();
}