// Recentemente Implementado por Yori Nilaje

#include "uart_avr.h"
#include <avr/io.h>

void uart_init(uint32_t baudrate) {
    // Configura baud rate (UBRR = F_CPU / (16 * baudrate) - 1)
    uint16_t ubrr = (F_CPU / 16 / baudrate) - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    // Habilita TX e RX
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Configura formato: 8 bits, sem paridade, 1 stop bit (padrão)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_write_byte(uint8_t data) {
    // Espera até o buffer de transmissão estar vazio
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data; // Envia o byte
}

void uart_write_string(const char *str) {
    while (*str) {
        uart_write_byte(*str++);
    }
}

uint8_t uart_read_byte(void) {
    // Espera até um byte ser recebido
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

uint8_t uart_data_ready(void) {
    // Retorna 1 se há dados no buffer de recepção
    return (UCSR0A & (1 << RXC0)) ? 1 : 0;
}