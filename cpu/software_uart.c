// soft_uart.c
#include "software_uart.h"
#include <avr/io.h>
#include <util/delay.h> // Para _delay_us

// O F_CPU e SOFT_UART_BIT_TIME_US agora vêm de soft_uart.h

void soft_uart_init(void) { // Função agora não recebe baudrate
    // Configura os pinos como entrada/saída
    SOFT_UART_RX_DDR &= ~(1 << SOFT_UART_RX_PIN); // RX como entrada
    SOFT_UART_RX_PORT |= (1 << SOFT_UART_RX_PIN); // Pull-up no RX (opcional, mas bom para evitar floating)

    SOFT_UART_TX_DDR |= (1 << SOFT_UART_TX_PIN);  // TX como saída
    SOFT_UART_TX_PORT |= (1 << SOFT_UART_TX_PIN); // TX em HIGH (idle state)

    // bit_time_us não é mais uma variável, é uma macro
}

void soft_uart_write_byte(uint8_t data) {
    uint8_t i;

    // Start bit (LOW)
    SOFT_UART_TX_PORT &= ~(1 << SOFT_UART_TX_PIN);
    _delay_us(SOFT_UART_BIT_TIME_US); // Usando a macro

    // Data bits (8 bits, LSB first)
    for (i = 0; i < 8; i++) {
        if (data & 0x01) {
            SOFT_UART_TX_PORT |= (1 << SOFT_UART_TX_PIN); // HIGH
        } else {
            SOFT_UART_TX_PORT &= ~(1 << SOFT_UART_TX_PIN); // LOW
        }
        _delay_us(SOFT_UART_BIT_TIME_US); // Usando a macro
        data >>= 1;
    }

    // Stop bit (HIGH)
    SOFT_UART_TX_PORT |= (1 << SOFT_UART_TX_PIN);
    _delay_us(SOFT_UART_BIT_TIME_US); // Usando a macro
}

void soft_uart_write_string(const char *str) {
    while (*str) {
        soft_uart_write_byte(*str++);
    }
}

uint8_t soft_uart_read_byte(void) {
    uint8_t data = 0;
    uint8_t i;

    // Espera pelo start bit (LOW)
    // ATENÇÃO: Se não houver start bit, esta linha BLOQUEIA PARA SEMPRE.
    while (SOFT_UART_RX_PIN_REG & (1 << SOFT_UART_RX_PIN));

    // Ajusta para o meio do primeiro bit de dados
    _delay_us(SOFT_UART_BIT_TIME_US / 2); // Usando a macro

    // Lê os 8 bits de dados
    for (i = 0; i < 8; i++) {
        _delay_us(SOFT_UART_BIT_TIME_US); // Usando a macro
        if (SOFT_UART_RX_PIN_REG & (1 << SOFT_UART_RX_PIN)) {
            data |= (1 << i); // LSB first
        }
    }

    // Espera pelo stop bit (para garantir que a linha retorne ao estado HIGH)
    _delay_us(SOFT_UART_BIT_TIME_US); // Usando a macro

    return data;
}

uint8_t soft_uart_data_ready(void) {
    // Para esta implementação simples, "data ready" significa que o pino RX está LOW (start bit)
    return !(SOFT_UART_RX_PIN_REG & (1 << SOFT_UART_RX_PIN));
}