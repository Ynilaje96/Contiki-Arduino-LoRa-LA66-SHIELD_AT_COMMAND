/*
 * LA66 LoRaWAN Driver for Contiki
 * Versão completa com todas as declarações necessárias
 */

#ifndef LA66_DRIVER_H
#define LA66_DRIVER_H

#include "contiki.h"
#include "process.h"  // Adicionado para PROCESS_CURRENT()

// Estados do LA66
enum la_states {
  LA_OFF,
  LA_READY,
  LA_JOINED,
  LA_DATA_READY,
  LA_SENT,
  LA_ERROR
};

// Comandos/Eventos
enum la_commands {
  LA_INIT,
  LA_JOIN,
  LA_SEND,
  LA_CHECK_STATUS,
  LA_GET_CONFIG,
  LA_RESET,
  LA_CURRENT_STATE,
  LA_AUTO_JOIN_SEND
};

// Estrutura do driver
struct la_driver {
  void (* init)(void);
  int (* join_network)(void);
  int (* send_data)(void);
  int (* get_join_status)(void);
  int (* get_config)(void);
  int (* reset)(void);
  int (* set_appeui)(const char *eui);
  int (* set_appkey)(const char *key);
  int (* set_deveui)(const char *eui);
  int (* auto_join_send)(void);
  void (* register_callbacks)(void);
};

// Declaração do driver
extern const struct la_driver LA_DRIVER;

// Protótipos das funções de callback (adicionados)
void handle_ok_response(struct at_cmd *cmd, uint8_t len, char *data);
void handle_error_response(struct at_cmd *cmd, uint8_t len, char *data);
void handle_njs_response(struct at_cmd *cmd, uint8_t len, char *data);
void handle_version_response(struct at_cmd *cmd, uint8_t len, char *data);
void handle_join_accepted_response(struct at_cmd *cmd, uint8_t len, char *data);

#endif /* LA66_DRIVER_H */