// Implementado Recentemente por Yori Nilaje

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

/**
 * Inicializa a UART com o baud rate especificado.
 * @param baudrate Taxa de transmissão (ex: 9600, 115200).
 */
void uart_init(uint32_t baudrate);

/**
 * Envia um byte via UART.
 * @param data Byte a ser enviado.
 */
void uart_write_byte(uint8_t data);

/**
 * Envia uma string via UART.
 * @param str String terminada em '\0'.
 */
void uart_write_string(const char *str);

/**
 * Lê um byte da UART (bloqueante).
 * @return Byte recebido.
 */
uint8_t uart_read_byte(void);

/**
 * Verifica se há dados disponíveis para leitura.
 * @return 1 se há dados, 0 caso contrário.
 */
uint8_t uart_data_ready(void);

#endif /* UART_H_ */