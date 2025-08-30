#ifndef SOFTWARE_UART_SERIAL_LINE_H
#define SOFTWARE_UART_SERIAL_LINE_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay_basic.h>

// Configuração de pinos
#define SOFT_UART_TX_PIN_BIT  PB2  // Arduino D10
#define SOFT_UART_RX_PIN_BIT  PB3  // Arduino D11

// Baud rate
#ifndef SOFTWARE_UART_BAUD_RATE
#define SOFTWARE_UART_BAUD_RATE 9600UL
#endif

// Cálculos de tempo
#define SOFT_UART_BIT_PERIOD_US (1000000UL / SOFTWARE_UART_BAUD_RATE)
#define SOFT_UART_CYCLES_PER_BIT (F_CPU / SOFTWARE_UART_BAUD_RATE)
#define SOFT_UART_CYCLES_HALF_BIT (SOFT_UART_CYCLES_PER_BIT / 2)

// Macros de controle de pino
#define SOFT_UART_TX_HIGH() (PORTB |= (1 << SOFT_UART_TX_PIN_BIT))
#define SOFT_UART_TX_LOW()  (PORTB &= ~(1 << SOFT_UART_TX_PIN_BIT))
#define SOFT_UART_RX_READ() (PINB & (1 << SOFT_UART_RX_PIN_BIT))

// Tamanho do buffer de recepção
#define SOFT_UART_RX_BUFFER_SIZE 64

// Protótipos
void soft_uart_init(void);
void soft_uart_write_byte(uint8_t data);
int soft_uart_write_string(const char *str);
uint8_t soft_uart_read_buffer(void);
int putcharec(int c);  // redireciona printf()

extern volatile uint8_t soft_uart_rx_overflow;

#endif
