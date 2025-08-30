// soft_uart.h
#ifndef SOFT_UART_H_
#define SOFT_UART_H_

#include <stdint.h>

// Defina os pinos RX e TX para a Software UART
// Estes correspondem aos pinos D2 e D3 do Arduino
#define SOFT_UART_RX_PIN    PD2  // Pino digital 2 do Arduino
#define SOFT_UART_TX_PIN    PD3  // Pino digital 3 do Arduino

// Mapeamento dos pinos para os registradores PORT/DDR/PIN do AVR
// Para D2: PD2
#define SOFT_UART_RX_PORT   PORTD
#define SOFT_UART_RX_DDR    DDRD
#define SOFT_UART_RX_PIN_REG PIND

// Para D3: PD3
#define SOFT_UART_TX_PORT   PORTD
#define SOFT_UART_TX_DDR    DDRD
#define SOFT_UART_TX_PIN_REG PIND

// Defina o BAUD RATE AQUI, pois soft_uart.c precisará dele como constante
#ifndef F_CPU
#define F_CPU 16000000UL // Frequência da CPU (16 MHz para Arduino Uno)
#endif

// Calcule o tempo de um bit em microssegundos.
// Isso DEVE ser uma constante de compilação para _delay_us/_delay_ms.
#define SOFT_UART_BAUD_RATE 9600UL // Defina o baud rate AQUI. Use 'UL' para unsigned long.
#define SOFT_UART_BIT_TIME_US (1000000UL / SOFT_UART_BAUD_RATE)


/**
 * Inicializa a Software UART. O baud rate é definido via macro SOFT_UART_BAUD_RATE.
 */
void soft_uart_init(void);

/**
 * Envia um byte via Software UART.
 * @param data Byte a ser enviado.
 */
void soft_uart_write_byte(uint8_t data);

/**
 * Envia uma string via Software UART.
 * @param str String terminada em '\0'.
 */
void soft_uart_write_string(const char *str);

/**
 * Lê um byte da Software UART (bloqueante).
 * @return Byte recebido.
 */
uint8_t soft_uart_read_byte(void);

/**
 * Verifica se há dados disponíveis para leitura na Software UART.
 * (Implementação simples pode apenas verificar o estado do pino RX).
 * @return 1 se há dados, 0 caso contrário.
 */
uint8_t soft_uart_data_ready(void);

#endif /* SOFT_UART_H_ */