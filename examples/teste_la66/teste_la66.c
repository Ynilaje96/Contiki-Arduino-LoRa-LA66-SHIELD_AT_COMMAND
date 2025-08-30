#include "contiki.h"
#include "at-master.h"             // Para a função at_send()
#include "software_uart_serial_line.h" // Para a entrada de dados da serial do usuário
#include "serial-line.h"           // Para processar linhas da serial
#include "la66.h"                  // O cabeçalho do seu driver LA66
#include <stdio.h>                 // Para printf()
#include <string.h>                // Para strlen(), strstr(), etc.

/*---------------------------------------------------------------------------*/
// Declaração do processo principal da sua aplicação
PROCESS(la66_main_process, "LA66 Main Process");
AUTOSTART_PROCESSES(&la66_main_process);

/*---------------------------------------------------------------------------*/
// Variável de estado para rastrear se o LA66 está conectado à rede LoRaWAN
static volatile uint8_t la66_joined = 0;

/*---------------------------------------------------------------------------*/
// Protótipos das funções de callback da APLICAÇÃO.
// Estas funções serão registradas no driver LA66 para receber notificações.
static void app_on_general_response(char *data, uint16_t len);
static void app_on_join_status(char *data, uint16_t len);
static void app_on_error_response(char *data, uint16_t len);

/*---------------------------------------------------------------------------*/
// Implementações dos callbacks da APLICAÇÃO:

/**
 * @brief Callback chamado para qualquer resposta geral do LA66.
 * Registrado via LA66_DRIVER.set_response_callback.
 * @param data O buffer com a string da resposta.
 * @param len O comprimento da string.
 */
static void
app_on_general_response(char *data, uint16_t len)
{
  printf("APP_CB: Resposta LA66 (genérica): '%.*s'\n", len, data);
  // Você pode adicionar mais lógica aqui se precisar processar todas as respostas.
}

/**
 * @brief Callback chamado para status de união à rede LoRaWAN (+NJS:).
 * Registrado via LA66_DRIVER.set_join_callback.
 * @param data O buffer com a string da resposta +NJS:.
 * @param len O comprimento da string.
 */
static void
app_on_join_status(char *data, uint16_t len)
{
  printf("APP_CB: Status de Join recebido: '%.*s'\n", len, data);
  // Analisa a resposta para ver se o módulo está conectado
  if (strstr(data, "+NJS:1")) { // Assumindo que "+NJS:1" significa conectado
    la66_joined = 1;
    printf("APP_CB: *** Módulo LA66 CONECTADO à rede LoRaWAN! ***\n");
  } else {
    la66_joined = 0;
    printf("APP_CB: Módulo LA66 NÃO conectado (ainda).\n");
  }
}

/**
 * @brief Callback chamado quando o LA66 reporta um erro.
 * Registrado via LA66_DRIVER.set_error_callback.
 * @param data O buffer com a string de erro (ex: "ERROR").
 * @param len O comprimento da string.
 */
static void
app_on_error_response(char *data, uint16_t len)
{
  printf("APP_CB: ERRO LA66 recebido: '%.*s'\n", len, data);
  // Aqui você pode implementar lógicas de recuperação de erro, re-tentativa, etc.
}

/*---------------------------------------------------------------------------*/
// Thread principal do processo da aplicação
PROCESS_THREAD(la66_main_process, ev, data)
{
  PROCESS_BEGIN();

  printf("LA66 Main Process Iniciado\n");

  // 1. INICIAR O PROCESSO DO DRIVER LA66
  // Este passo é crucial! O processo do driver precisa estar rodando.
  process_start(&la66_process, NULL);

  // 2. REGISTRAR OS CALLBACKS DA APLICAÇÃO COM O DRIVER LA66
  // A aplicação usa a API pública do driver (LA66_DRIVER) para configurar
  // suas próprias funções de callback.
  LA66_DRIVER.set_response_callback(app_on_general_response);
  LA66_DRIVER.set_join_callback(app_on_join_status);
  LA66_DRIVER.set_error_callback(app_on_error_response);

  // 3. EXEMPLES DE COMANDOS INICIAIS AO DRIVER LA66 (Opcional)
  // O driver LA66 já envia um RESET na sua inicialização, mas você pode
  // adicionar outros comandos aqui, como tentar o join automaticamente.
  printf("APP: Enviando comando AT+NJM=1\\r\\n para configurar modo OTAA...\n");
  LA66_DRIVER.join_network(1); // Tenta iniciar o processo de join em modo OTAA (mode 1)


  printf("APP: Pronto para receber comandos AT via serial. Digite e pressione Enter.\n");

  // Loop de eventos do processo da aplicação
  while(1) {
    PROCESS_WAIT_EVENT();

    // Evento de linha serial: o usuário digitou algo e pressionou Enter
    if(ev == serial_line_event_message) {
      char *line = (char *)data; // A linha digitada pelo usuário
      printf("APP: Comando do usuário: '%s'\n", line);

      // Constrói o comando AT com o terminador \r\n
      char cmd_buffer[100]; // Buffer para o comando, ajuste o tamanho conforme necessário
      int len_to_send = snprintf(cmd_buffer, sizeof(cmd_buffer), "%s\r\n", line);
      
      if (len_to_send > 0 && len_to_send < sizeof(cmd_buffer)) {
        // Envia o comando construído diretamente para o módulo LA66 via at-master
        at_send(cmd_buffer, len_to_send);
        printf("APP: Comando AT enviado ao LA66.\n");
      } else {
        printf("APP: Erro: Comando muito longo ou vazio.\n");
      }
    }
    // Se você tiver lógica específica no processo LA66_MAIN_PROCESS para reagir
    // a eventos postados pelo driver LA66 (além dos callbacks), trataria aqui:
    // else if (ev == PROCESS_EVENT_LA66_RESPONSE) {
    //   // struct la66_response *resp = (struct la66_response *)data;
    //   printf("APP: Evento interno do driver LA66 recebido (PROCESS_EVENT_LA66_RESPONSE).\n");
    //   // Adicione sua lógica de estado aqui, se necessário.
    // }
  }

  PROCESS_END();
}