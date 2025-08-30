/*
 * LA66 LoRaWAN Shield Driver for Contiki
 */

#include "contiki.h"
#include <stdio.h>
#include <string.h>
#include "at-master.h"
#include "la66.h"

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
process_event_t PROCESS_EVENT_LA66_RESPONSE; 
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

/* Callbacks globais */
static la66_response_callback_t response_cb = NULL;
static la66_response_callback_t join_cb = NULL;
static la66_response_callback_t error_cb = NULL;


/* Funções para configurar callbacks */
static void set_response_callback(la66_response_callback_t cb) {
  response_cb = cb;
}

static void set_join_callback(la66_response_callback_t cb) {
  join_cb = cb;
}

static void set_error_callback(la66_response_callback_t cb) {
  error_cb = cb;
}

/*---------------------------------------------------------------------------*/
/* Funções de callback */
// Lembre-se: As variáveis globais response_cb, join_cb, error_cb
// devem estar declaradas (static la66_response_callback_t ...)
// e o typedef la66_response_callback_t deve estar em la66.h

/*---------------------------------------------------------------------------*/
/* Funções de callback */
static void la66_callback(struct at_cmd *cmd, uint8_t len, char *data) {
  // Verifica se os dados recebidos são válidos
  if(len > 0 && data != NULL) {
    // Opcional: Imprime a resposta completa para fins de depuração.
    // Isso é útil para ver todas as comunicações vindas do LA66.
    printf("LA66_Callback: Recebido '%s' (len: %u)\n", data, len);

    // 1. Chamar o callback genérico de resposta (se registrado).
    // Este callback receberá *todas* as linhas processadas por la66_callback.
    if(response_cb) {
      response_cb(data, len);
    }
    
    // 2. Despachar para callbacks específicos baseados no conteúdo da string.
    //    Esta lógica usa 'strstr' para procurar substrings como "+JOIN" ou "ERROR".
    //    É uma forma de "re-despachar" se la66_callback for o único callback registrado
    //    no at-master para múltiplas respostas.
    //    (Alternativamente, at_master poderia chamar handlers específicos como handle_ok,
    //    handle_error para respostas registradas individualmente, e esses handlers
    //    chamarão os callbacks externos.)
    
    if(strstr(data, "+JOIN")) { // Verifica se a string recebida contém "+JOIN"
      if(join_cb) {
        printf("LA66_Callback: Despachando para Join Callback.\n");
        join_cb(data, len); // Invoca o callback de join, se houver um registrado
      }
    } else if(strstr(data, "ERROR")) { // Verifica se a string recebida contém "ERROR"
      if(error_cb) {
        printf("LA66_Callback: Despachando para Error Callback.\n");
        error_cb(data, len); // Invoca o callback de erro, se houver um registrado
      }
    }
    // Você pode adicionar mais blocos 'else if(strstr(...))' aqui para lidar
    // com outras respostas específicas que o LA66 pode enviar (ex: "+EVT:TX", "+EVT:RX").
    
  } else {
    // Mensagem de erro se a função foi chamada com dados inválidos
    printf("LA66_Callback: Erro: Dados nulos ou comprimento zero recebidos.\n");
  }
}
/*---------------------------------------------------------------------------*/
/* Função de inicialização */
static void
init(void)
{
  // at_init(1); // Mantenha comentado ou remova se já estiver dentro do at_init

  // Registra os handlers específicos PRIMEIRO para que o at-master os encontre antes
  at_register(&at_cmd_ok_response, &la66_process, "OK", 2, 64, handle_ok);
  at_register(&at_cmd_error_response, &la66_process, "ERROR", 5, 64, handle_error);
  at_register(&at_cmd_njs_response, &la66_process, "+NJS:", 5, 64, handle_njs); // Exemplo para +NJS:

  // Registra o callback genérico de captura TUDO por ÚLTIMO,
  // para que ele só seja acionado se nenhuma das respostas específicas for encontrada.
  at_register(&at_cmd_la66, PROCESS_CURRENT(), "", 0, 128, la66_callback);

  /* Reset do módulo (envio inicial) */
  at_send(LA66_AT_RESET, strlen(LA66_AT_RESET));
  
  printf("LA66: Driver inicializado e comando ATZ enviado.\n");
  
  // O ctimer abaixo precisa de um callback real se for para uma ação específica
  // ou pode ser removido se for apenas um delay sem ação.
  // ctimer_set(&la66_timer, CLOCK_SECOND * 2, NULL, NULL); 
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
  .init = init,
  .join_network = join_network,
  .send_data = send_data,
  .get_join_status = get_join_status,
  .get_config = get_config,
  .set_response_callback = set_response_callback,
  .set_join_callback = set_join_callback,
  .set_error_callback = set_error_callback
};
/*---------------------------------------------------------------------------*/
/*Confirmacao de erros ou de sucessos*/
static void handle_ok(struct at_cmd *cmd, uint8_t len, char *data) {
    printf("LA66: OK received.\n");

    if (response_cb) {
      response_cb(data, len); // ou response_cb("OK", 2); para ser mais abstrato
    }
// process_post(&la66_process, PROCESS_EVENT_LA66_RESPONSE, (void *)LA66_STATUS_OK);
    // Sinalizar sucesso para o processo principal, talvez via process_post
}
static void handle_error(struct at_cmd *cmd, uint8_t len, char *data) {
    printf("LA66: ERROR received.\n");

    if (error_cb) {
      error_cb(data, len);
    }
// process_post(&la66_process, PROCESS_EVENT_LA66_RESPONSE, (void *)LA66_STATUS_ERROR);
    // Sinalizar falha
}
static void handle_njs(struct at_cmd *cmd, uint8_t len, char *data) {
    printf("LA66: NJS response: %.*s\n", len, data);

    // Exemplo de parsing simples e chamada de callback
    if (strstr(data, "+NJS:1")) { // Ou um parsing mais robusto
        printf("LA66: Successfully joined network.\n");
        // Atualizar estado interno do driver para "unido"
        if (join_cb) {
            join_cb(data, len); // Ou apenas um status simples
        }
        // process_post(&la66_process, PROCESS_EVENT_LA66_RESPONSE, (void *)LA66_STATUS_JOINED);
    } else {
        printf("LA66: Not joined (status: %.*s).\n", len, data);
        // Atualizar estado interno para "não unido"
    }
    // Analisar o status de join e atualizar o estado interno do driver
}

// Em la66.c, perto das outras callbacks ou funções auxiliares
static void
join_timeout(void *ptr)
{
  printf("LA66: Timeout ao tentar join na rede.\n");
  
  if (error_cb) {
    error_cb("Join Timeout", strlen("Join Timeout")); // Ou uma mensagem mais específica
  }
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
  init(); // <--- Chame a função de inicialização aqui!

  
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