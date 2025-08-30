// soft_uart.h
#ifndef SOFT_UART_H_
#define SOFT_UART_H_

#include <stdint.h>
#include <avr/io.h> // Para PORTD, DDRD, PIND, etc.

// Define os pinos RX e TX para a Software UART (Arduino D2 e D3)
#define SOFT_UART_RX_PIN    PB3  // Pino digital 11 do Arduino
#define SOFT_UART_TX_PIN    PB2  // Pino digital 10 do Arduino

// Mapeamento dos pinos para os registradores PORT/DDR/PIN do AVR
// Para RX (B3): PB3 (Parte do PCINT2_vect - PCINT3)
#define SOFT_UART_RX_PORT_REG   PORTB
#define SOFT_UART_RX_DDR_REG    DDRB
#define SOFT_UART_RX_PIN_REG    PINB
#define SOFT_UART_RX_PCINT_BIT  PCINT3 // Bit correspondente ao PB3 em PCMSK0

// Para TX (B3): PB3
#define SOFT_UART_TX_PORT_REG   PORTB
#define SOFT_UART_TX_DDR_REG    DDRB
#define SOFT_UART_TX_PIN_BIT    PB2

// Define a frequência da CPU (Arduino Uno)
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Define o Baud Rate para a Software UART
#define SOFTWARE_UART_BAUD_RATE 9600UL

// Macros para controle de pino (para clareza no TX)
#define SOFT_UART_TX_HIGH() (SOFT_UART_TX_PORT_REG |= (1 << SOFT_UART_TX_PIN_BIT))
#define SOFT_UART_TX_LOW()  (SOFT_UART_TX_PORT_REG &= ~(1 << SOFT_UART_TX_PIN_BIT))
#define SOFT_UART_RX_READ() ((SOFT_UART_RX_PIN_REG & (1 << SOFT_UART_RX_PIN)) ? 1 : 0)

// Cálculos de tempo em ciclos de clock para Timer1
// F_CPU = 16MHz
// Baud = 9600
// Tempo de um bit = 1 / 9600 = 104.16 us
// Ciclos de clock por bit = F_CPU / BAUD_RATE = 16000000 / 9600 = 1666.666...
// Usaremos arredondamento para o cálculo dos ciclos.

// Número de ciclos para 1 bit completo (aproximado)
#define SOFT_UART_CYCLES_PER_BIT (F_CPU / SOFTWARE_UART_BAUD_RATE)

// Número de ciclos para 0.5 bit (para amostragem no meio do bit)
#define SOFT_UART_CYCLES_HALF_BIT (SOFT_UART_CYCLES_PER_BIT / 2)

// Contador para _delay_loop_2. Cada iteração consome 2 ou 3 ciclos. Usaremos 3 ciclos por iteração.
// Por segurança, arredondamos para cima.
#define SOFT_UART_TX_DELAY_LOOP_COUNT ((SOFT_UART_CYCLES_PER_BIT + 2) / 3)


/**
 * Inicializa a Software UART, configurando pinos e interrupções.
 * Chama-se uma vez no início do programa.
 */
void soft_uart_init(void);

/**
 * Envia um byte via Software UART (pino D3).
 * Esta função é bloqueante (bit-banging).
 * @param data Byte a ser enviado.
 */
void soft_uart_write_byte(uint8_t data);

/**
 * Envia uma string via Software UART (pino D3).
 * Esta função é bloqueante.
 * @param str String terminada em '\0'.
 */
void soft_uart_write_string(const char *str);

#endif /* SOFT_UART_H_ */