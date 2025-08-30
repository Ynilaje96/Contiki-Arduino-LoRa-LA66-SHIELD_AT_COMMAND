#include "contiki.h"
#include "at-master.h"        // Módulo de comandos AT
#include "software_uart_serial_line.h"        // Sua Software UART (D2/D3)
#include "serial-line.h"  // Módulo serial-line
#include "la66.h"             // Seu driver LA66
#include <stdio.h>            // Para printf
#include <string.h>           // Para strlen, strstr

/*---------------------------------------------------------------------------*/
// Processo principal da sua aplicação
PROCESS(soft_uart_serial_line_test_process, "Software UART Serial Line Test Process");
AUTOSTART_PROCESSES(&soft_uart_serial_line_test_process); // Garante que este processo inicie automaticamente
/*---------------------------------------------------------------------------*/

// Mapeamento para comando AT+VER=? (exemplo)
#define LA66_AT_GET_VERSION   "AT+VER=?\r\n" // Verifique no datasheet do LA66 qual é o comando real para versão!

// Variáveis para registrar callbacks de resposta específicas
static struct at_cmd at_cmd_ok_response;
static struct at_cmd at_cmd_error_response;
static struct at_cmd at_cmd_njs_response_callback_struct; // Callback para resposta de status de join
static struct at_cmd at_cmd_ver_response_callback_struct; // Callback para resposta de versão


// Variáveis de estado global para o LA66 (opcional, pode ser movido para la66.c)
static volatile uint8_t la66_joined_status = 0; // 0 = not joined, 1 = joined

/*---------------------------------------------------------------------------*/
// Funções de callback para respostas AT específicas

// Callback para a resposta "OK"
static void
handle_ok_response(struct at_cmd *cmd, uint8_t len, char *data)
{
  printf("APP: LA66 respondeu OK.\n");
  // Aqui você pode adicionar lógica para avançar um estado, se necessário
}

// Callback para a resposta "ERROR"
static void
handle_error_response(struct at_cmd *cmd, uint8_t len, char *data)
{
  printf("APP: LA66 respondeu ERROR: %.*s\n", len, data);
  // Aqui você pode adicionar lógica para tratar erros, como tentar novamente
}

// Callback para a resposta do comando AT+NJS=? (Status de Join)
static void
handle_njs_response(struct at_cmd *cmd, uint8_t len, char *data)
{
  printf("APP: LA66 Status de Join (AT+NJS=?): %.*s\n", len, data);
  if (strstr(data, "+NJS:1")) { // Verifique o formato exato da resposta no datasheet
    la66_joined_status = 1;
    printf("APP: LA66 está conectado à rede LoRaWAN.\n");
  } else {
    la66_joined_status = 0;
    printf("APP: LA66 NÃO está conectado à rede LoRaWAN.\n");
  }
}

// Callback para a resposta do comando AT+VER=? (Versão do Firmware)
static void
handle_version_response(struct at_cmd *cmd, uint8_t len, char *data)
{
  printf("APP: LA66 Versão do Firmware (AT+VER=?): %.*s\n", len, data);
  // Você pode armazenar a versão em uma variável aqui, se precisar
}

// Callback para a resposta do comando AT+JOIN (Se o LA66 responder com "+JOIN: Accepted")
static void
handle_join_accepted_response(struct at_cmd *cmd, uint8_t len, char *data)
{
  printf("APP: LA66 JOIN ACCEPTED: %.*s\n", len, data);
  la66_joined_status = 1; // Marca como conectado
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(soft_uart_serial_line_test_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  // 1. Inicializar a Software UART (D2/D3)
  soft_uart_init();
  printf("Software UART (D2/D3) inicializada com %u baud.\n", (unsigned int)SOFT_UART_BAUD_RATE);

  // 2. Inicializar o módulo serial-line (necessário para o at-master)
  serial_line_init();
  printf("Contiki serial-line process iniciado.\n");

  // 3. Inicializar o at-master (sem um uart_sel específico, pois já está configurado para soft_uart)
  at_init(0); // O '0' é um placeholder para o uart_sel, pois at-master.c foi modificado para não usá-lo

  // 4. Registrar as callbacks de resposta específicas no at-master
  // Use o processo 'la66_process' para receber os eventos das callbacks
  at_register(&at_cmd_ok_response, &la66_process, "OK", strlen("OK"), 64, handle_ok_response);
  at_register(&at_cmd_error_response, &la66_process, "ERROR", strlen("ERROR"), 64, handle_error_response);
  at_register(&at_cmd_njs_response_callback_struct, &la66_process, "+NJS:", strlen("+NJS:"), 64, handle_njs_response);
  at_register(&at_cmd_ver_response_callback_struct, &la66_process, "+VER:", strlen("+VER:"), 64, handle_version_response); // Ajuste "+VER:" para a resposta real do LA66
  at_register(NULL, &la66_process, "+JOIN: Accepted", strlen("+JOIN: Accepted"), 64, handle_join_accepted_response); // Callback para join aceito

  printf("AT Master inicializado e callbacks registradas.\n");

  // 5. Iniciar o driver LA66 (que enviará o ATZ)
  LA66_DRIVER.init();
  printf("LA66 Driver iniciado. Enviando ATZ para reset...\n");

  // Configura um timer para enviar comandos periodicamente
  etimer_set(&et, CLOCK_SECOND * 10); // A cada 10 segundos

  while(1) {
    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER) {
      if(etimer_expired(&et)) {
        printf("\n--- Novo ciclo de comandos ---\n");

        // 1. Consultar status de join
        LA66_DRIVER.get_join_status();
        etimer_set(&et, CLOCK_SECOND * 2); // Espera um pouco antes do próximo comando

      }
    }
    // Opcional: Você pode adicionar mais lógica aqui para reagir a outros eventos do LA66_DRIVER
    // Por exemplo, se o LA66_process postar um evento para indicar que a inicialização está completa.
    else if (ev == PROCESS_EVENT_LA66_RESPONSE) {
        // Isso é tratado pelas callbacks específicas registradas
        // Mas se houver uma callback genérica em la66.c para tudo que não for casado,
        // ela postaria aqui.
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/