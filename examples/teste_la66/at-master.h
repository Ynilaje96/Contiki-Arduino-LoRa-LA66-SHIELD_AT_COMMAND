#ifndef AT_MASTER_H_
#define AT_MASTER_H_

#include "contiki.h"

/*---------------------------------------------------------------------------*/
// Macros para respostas AT padrão
#define AT_DEFAULT_RESPONSE_OK    "\r\nOK\r\n"
#define AT_DEFAULT_RESPONSE_ERROR "\r\nERROR\r\n"

/*---------------------------------------------------------------------------*/
// Macro de conveniência para enviar uma string via at_send
// assume que at_send está definida em at-master.c
#define AT_RESPONSE(x) at_send((x), (strlen(x)))

/*---------------------------------------------------------------------------*/
// Declaração do evento global para notificação de comandos AT recebidos
extern process_event_t at_cmd_received_event;

/*---------------------------------------------------------------------------*/
// Forward declaration da estrutura at_cmd (necessária antes da definição completa)
struct at_cmd;

/*---------------------------------------------------------------------------*/
// Definição dos códigos de status para funções AT
typedef enum {
  AT_STATUS_OK,
  AT_STATUS_ERROR,
  AT_STATUS_INVALID_ARGS_ERROR,
} at_status_t;

/*---------------------------------------------------------------------------*/
// Typedef para a função de callback de evento AT
// Esta função será chamada quando uma resposta AT for detectada
typedef void (*at_event_callback_t)(struct at_cmd *cmd,
                                    uint8_t len,
                                    char *data);

/*---------------------------------------------------------------------------*/
/**
 * \brief Definição da estrutura para registrar um comando/resposta AT
 * na lista de monitoramento do AT Master.
 */
struct at_cmd {
  struct at_cmd *next; // Ponteiro para o próximo item na lista (para contiki-lib/list)
  const char *cmd_header; // String do cabeçalho da resposta a ser buscada (ex: "OK", "+EVT:JOINED")
  uint8_t cmd_hdr_len;    // Comprimento do cmd_header
  uint8_t cmd_max_len;    // Comprimento máximo esperado para a linha completa da resposta
  at_event_callback_t event_callback; // Função de callback a ser chamada quando a resposta é detectada
  struct process *app_process; // Processo Contiki que registrou este comando (para postar eventos de volta)
};

/*---------------------------------------------------------------------------*/
/**
 * \brief AT initialization
 * \param uart  selects which UART to use (unused in current soft_uart implementation)
 *
 * This function initializes the AT master process and the underlying
 * Software UART and serial-line input handler. It should be called once at startup.
 */
void at_init(uint8_t uart);

/*---------------------------------------------------------------------------*/
/**
 * \brief Sends a string of data over the configured UART.
 * \param s   Pointer to the string to send.
 * \param len Length of the string to send.
 * \return    Number of bytes sent.
 *
 * Note: This function does not automatically append CR+LF. The caller
 * is responsible for adding necessary terminators for AT commands.
 */
uint8_t at_send(char *s, uint8_t len);

/*---------------------------------------------------------------------------*/
/**
 * \brief                 Registers a callback to handle a specific AT command response.
 * \param cmd             A pointer to an allocated struct at_cmd placeholder.
 * \param app_process     The Contiki process registering this command (to receive events).
 * \param cmd_hdr         String to compare when an AT command response is received (e.g., "OK", "ERROR").
 * \param cmd_hdr_len     Length of cmd_hdr.
 * \param cmd_max_len     Maximum expected length of the response line.
 * \param event_callback  Callback function to handle the detected AT response.
 * \return                AT_STATUS_OK on success, or an error code.
 *
 * Registers the parameters to search for when a valid AT frame has been received,
 * and sets up the callback and target process for handling it.
 */
at_status_t at_register(struct at_cmd *cmd,
                        struct process *app_process,
                        const char *cmd_hdr,
                        const uint8_t cmd_hdr_len,
                        const uint8_t cmd_max_len,
                        at_event_callback_t event_callback);

#endif /* AT_MASTER_H_ */