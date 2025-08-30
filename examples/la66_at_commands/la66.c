/*
 * LA66 LoRaWAN Shield Driver for Contiki
 */

#include "contiki.h"
#include "la66.h"
#include "at-master.h"
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Definições de comandos AT específicos do LA66 */
#define LA66_AT_RESET         "ATZ\r\n"
#define LA66_AT_JOIN          "AT+JOIN\r\n"
#define LA66_AT_SEND          "AT+SEND="
#define LA66_AT_GET_STATUS    "AT+NJS=?\r\n"
#define LA66_AT_CFG           "AT+CFG\r\n"

/* Estrutura para armazenar respostas */
struct la66_response {
  uint16_t len;
  char data[128];
};

/* Eventos do processo */
process_event_t PROCESS_EVENT_LA66_RESPONSE; // <--- ADICIONE ESTA LINHA AQUI!
PROCESS_NAME(la66_process);

/* Comandos AT */
static struct at_cmd at_cmd_la66;
static struct ctimer la66_timer;

/* Comandos estruturas de confirmacao de resposta */
static struct at_cmd at_cmd_ok_response;
static struct at_cmd at_cmd_error_response;
static struct at_cmd at_cmd_njs_response;

static void handle_ok(struct at_cmd *cmd, uint8_t len, char *data);
static void handle_error(struct at_cmd *cmd, uint8_t len, char *data);
static void handle_njs(struct at_cmd *cmd, uint8_t len, char *data);
static void join_timeout(void *ptr); // Ou a assinatura correta do seu timer callback






/*---------------------------------------------------------------------------*/
/* Funções de callback */
static void
la66_callback(struct at_cmd *cmd, uint8_t len, char *data)
{
  static struct la66_response response;
  
  response.len = len;
  if(len > 0 && data != NULL) {
    memcpy(response.data, data, len < sizeof(response.data) ? len : sizeof(response.data)-1);
    response.data[len] = '\0'; // Garante terminação nula
  }
  
  process_post(&la66_process, PROCESS_EVENT_LA66_RESPONSE, &response);
}
/*---------------------------------------------------------------------------*/
/* Função de inicialização */
static void
init(void)
{
  /* Inicializa interface AT (UART) */
  //at_init(1); // Usando UART1
  
  /* Configura callback padrão */
  at_register(&at_cmd_la66, PROCESS_CURRENT(), "", 0, 128, la66_callback);
  
  /* Reset do módulo */
  at_send(LA66_AT_RESET, strlen(LA66_AT_RESET));
  
  printf("LA66 Driver Initialized\n");

  /*Mensagens de Erro e Confirmacao*/
  at_register(&at_cmd_ok_response, &la66_process, "OK", 2, 64, handle_ok);
  at_register(&at_cmd_error_response, &la66_process, "ERROR", 5, 64, handle_error);
  at_register(&at_cmd_njs_response, &la66_process, "+NJS:", 5, 64, handle_njs); // Exemplo para +NJS:
  
  /*Inicio da Rotina*/
  at_send(LA66_AT_RESET, strlen(LA66_AT_RESET));
  printf("LA66: Driver inicializado e comando ATZ enviado.\n");
  ctimer_set(&la66_timer, CLOCK_SECOND * 2, NULL, NULL);


}
/*---------------------------------------------------------------------------*/
/* Função para join na rede LoRaWAN */
static int
join_network(uint8_t mode) // 0=ABP, 1=OTAA
{
  char cmd[32];
  
  /* Configura modo de join */
  snprintf(cmd, sizeof(cmd), "AT+NJM=%d\r\n", mode);
  at_send(cmd, strlen(cmd));
  
  /* Envia comando JOIN */
  at_send(LA66_AT_JOIN, strlen(LA66_AT_JOIN));
  
  /* Configura timeout para join */
  ctimer_set(&la66_timer, CLOCK_SECOND * 30, join_timeout, NULL);
  
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Função para enviar dados */
static int
send_data(uint8_t port, uint8_t confirm, const char *data)
{
  char cmd[64];
  int len;
  
  /* Monta comando AT+SEND */
  len = snprintf(cmd, sizeof(cmd), "AT+SEND=%d,%d,%d,%s\r\n", 
                 port, confirm, strlen(data), data);
  
  if(len > 0 && len < sizeof(cmd)) {
    at_send(cmd, len);
    return 0;
  }
  
  return -1;
}
/*---------------------------------------------------------------------------*/
/* Função para verificar status de join */
static int
get_join_status(void)
{
  at_send(LA66_AT_GET_STATUS, strlen(LA66_AT_GET_STATUS));
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Função para obter configurações */
static int
get_config(void)
{
  at_send(LA66_AT_CFG, strlen(LA66_AT_CFG));
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Estrutura do driver */
const struct la66_driver LA66_DRIVER = {
  init,
  join_network,
  send_data,
  get_join_status,
  get_config,
};
/*---------------------------------------------------------------------------*/
/*Confirmacao de erros ou de sucessos*/
static void handle_ok(struct at_cmd *cmd, uint8_t len, char *data) {
    printf("LA66: OK received.\n");
    // Sinalizar sucesso para o processo principal, talvez via process_post
}
static void handle_error(struct at_cmd *cmd, uint8_t len, char *data) {
    printf("LA66: ERROR received.\n");
    // Sinalizar falha
}
static void handle_njs(struct at_cmd *cmd, uint8_t len, char *data) {
    printf("LA66: NJS response: %.*s\n", len, data);
    // Analisar o status de join e atualizar o estado interno do driver
}

// Em la66.c, perto das outras callbacks ou funções auxiliares
static void
join_timeout(void *ptr)
{
  printf("LA66: Timeout ao tentar join na rede.\n");
  // Implemente aqui a lógica para lidar com o timeout,
  // como tentar novamente ou sinalizar um erro.
}



/*---------------------------------------------------------------------------*/
/* Processo LA66 */
PROCESS(la66_process, "LA66 LoRaWAN Driver");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(la66_process, ev, data)
{
  PROCESS_BEGIN();
  PROCESS_EVENT_LA66_RESPONSE = process_alloc_event();
  
  while(1) {
    PROCESS_WAIT_EVENT();
    
    if(ev == PROCESS_EVENT_LA66_RESPONSE) {
      struct la66_response *resp = (struct la66_response *)data;
      
      if(resp != NULL && resp->len > 0) {
        printf("LA66 Response: %.*s\n", resp->len, resp->data);
        
        // Aqui você pode adicionar tratamento específico para respostas
        if(strstr(resp->data, "JOINED")) {
          printf("Network joined successfully!\n");
        }
      }
    }
  }
  
  PROCESS_END();
}