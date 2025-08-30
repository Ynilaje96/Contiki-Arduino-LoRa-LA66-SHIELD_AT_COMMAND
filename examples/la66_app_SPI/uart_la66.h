/**
 * \file
 * LA66 UART Driver for ATmega328P - Header file
 *
 * \author
 * Yori Nilaje (based on Contiki examples)
 *
 * \brief
 * Provides functions to initialize and interact with the USART0 on ATmega328P
 * for communication with a virtual LA66 module.
 */

#ifndef UART_LA66_H_
#define UART_LA66_H_

#include <stdint.h> // Para tipos como uint8_t
#include <avr/io.h>   // Para as definições dos registradores do AVR (UCSR0x, UDR0, UBRR0x)

/* --- Configuration Defines (Customize these based on your ATmega328P and LA66) --- */

// Habilita/Desabilita o driver LA66 UART
#define UART_LA66_ENABLE 1

// Frequência do clock do CPU (para cálculo do baud rate)
// No Arduino UNO, geralmente é 16MHz.
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Velocidade da UART para o LA66
// Consulte o datasheet/documentação do LA66 para o baud rate padrão.
// Ex: para 9600 bps ou 115200 bps
#define UART_LA66_BAUD_RATE 9600UL

// Calcule o valor para o registrador UBRR (baud rate divisor) para USART0
// Fórmula para taxa de baud normal (sem U2X0 = 1)
#define UART_LA66_UBRR_VALUE ((F_CPU / 16 / UART_LA66_BAUD_RATE) - 1)

/* --- Public Function Prototypes --- */

/**
 * @brief Initializes the USART0 (UART) on ATmega328P for LA66 communication.
 * This function configures the necessary registers.
 */
void uart_la66_init(void);

/**
 * @brief Sends a single byte over the USART0 (UART) to the LA66.
 * This function blocks until the byte is transmitted.
 * @param byte The byte to be transmitted.
 */
void uart_la66_writeb(uint8_t byte);

/**
 * @brief Sets a callback function for received bytes on the LA66 UART.
 * @param rx_callback A pointer to the function to be called on byte reception.
 */
void uart_la66_set_input(int (*rx_callback)(unsigned char c));

#endif /* UART_LA66_H_ */