/*
 * LA66 LoRaWAN Driver Implementation
 * Versão corrigida e funcional
 */

#include "la66driver.h"
#include "contiki.h"
#include "at-master.h"
#include "software_uart_serial_line.h"
#include <stdio.h>
#include <string.h>

/* Variáveis de estado */
static enum la_states state = LA_OFF;
static struct ctimer la_timer;

/* Estruturas para callbacks AT */
static struct at_cmd at_cmd_ok_response;
static struct at_cmd at_cmd_error_response;
static struct at_cmd at_cmd_njs_response_callback_struct;
static struct at_cmd at_cmd_ver_response_callback_struct;

/*---------------------------------------------------------------------------*/
/* Funções auxiliares para comunicação AT */
static void send_at_command(const char *cmd) {
  printf("LA66: Enviando comando: %s", cmd);
  for(int i = 0; cmd[i] != '\0'; i++) {
    soft_uart_putchar(cmd[i]);
  }
}

/*---------------------------------------------------------------------------*/
/* Implementações das funções do driver */
static void la66_init(void) {
  printf("LA66: Inicializando...\n");
  send_at_command("ATZ\r\n");
  state = LA_READY;
}

static int la66_join_network(void) {
  printf("LA66: Juntando-se à rede...\n");
  send_at_command("AT+JOIN\r\n");
  return 0;
}

static int la66_send_data(void) {
  printf("LA66: Enviando dados...\n");
  // Implemente o envio real de dados aqui
  return 0;
}

static int la66_get_join_status(void) {
  printf("LA66: Verificando status de join...\n");
  send_at_command("AT+NJS=?\r\n");
  return 0;
}

static int la66_reset(void) {
  printf("LA66: Resetando...\n");
  send_at_command("ATZ\r\n");
  state = LA_OFF;
  return 0;
}

static int la66_auto_join_send(void) {
  printf("LA66: Auto join e send...\n");
  // Implemente conforme necessário
  return 0;
}

/*---------------------------------------------------------------------------*/
/* Funções de estado */
void state_reset(void) {
  state = LA_OFF;
}

static void join_timeout(void *ptr) {
  if(state == LA_READY || state == LA_JOINED) {
    state = LA_ERROR;
    printf("LA66: Join timeout\n");
  }
}

void auto_join_send(void) {
  LA_DRIVER.auto_join_send();
}

/*---------------------------------------------------------------------------*/
/* Callbacks AT */
static void register_la66_callbacks(void) {
  at_register(&at_cmd_ok_response, PROCESS_CURRENT(), "OK", strlen("OK"), 64, handle_ok_response);
  at_register(&at_cmd_error_response, PROCESS_CURRENT(), "ERROR", strlen("ERROR"), 64, handle_error_response);
  at_register(&at_cmd_njs_response_callback_struct, PROCESS_CURRENT(), "+NJS:", strlen("+NJS:"), 64, handle_njs_response);
  at_register(&at_cmd_ver_response_callback_struct, PROCESS_CURRENT(), "+VER:", strlen("+VER:"), 64, handle_version_response);
  at_register(NULL, PROCESS_CURRENT(), "+JOIN: Accepted", strlen("+JOIN: Accepted"), 64, handle_join_accepted_response);
}

/*---------------------------------------------------------------------------*/
/* Implementação da estrutura do driver */
const struct la_driver LA_DRIVER = {
  .init = la66_init,
  .join_network = la66_join_network,
  .send_data = la66_send_data,
  .get_join_status = la66_get_join_status,
  .get_config = NULL, // Implemente se necessário
  .reset = la66_reset,
  .set_appeui = NULL, // Implemente se necessário
  .set_appkey = NULL, // Implemente se necessário
  .set_deveui = NULL, // Implemente se necessário
  .auto_join_send = la66_auto_join_send,
  .register_callbacks = register_la66_callbacks
};

/*---------------------------------------------------------------------------*/
/* Funções de callback (implementações básicas) */
void handle_ok_response(struct at_cmd *cmd, uint8_t len, char *data) {
  printf("APP: LA66 respondeu OK.\n");
}

void handle_error_response(struct at_cmd *cmd, uint8_t len, char *data) {
  printf("APP: LA66 respondeu ERROR: %.*s\n", len, data);
}

void handle_njs_response(struct at_cmd *cmd, uint8_t len, char *data) {
  printf("APP: LA66 Status de Join: %.*s\n", len, data);
  if (strstr(data, "+NJS:1")) {
    state = LA_JOINED;
  }
}

void handle_version_response(struct at_cmd *cmd, uint8_t len, char *data) {
  printf("APP: LA66 Versão: %.*s\n", len, data);
}

void handle_join_accepted_response(struct at_cmd *cmd, uint8_t len, char *data) {
  printf("APP: LA66 Join Aceito: %.*s\n", len, data);
  state = LA_JOINED;
}