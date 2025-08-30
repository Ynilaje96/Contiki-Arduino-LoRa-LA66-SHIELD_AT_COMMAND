#ifndef LA66_H_
#define LA66_H_

#include "contiki.h"

/* Eventos */
extern process_event_t PROCESS_EVENT_LA66_RESPONSE;

/* Estrutura do driver */
struct la66_driver {
  void (* init)(void);
  int (* join_network)(uint8_t mode);
  int (* send_data)(uint8_t port, uint8_t confirm, const char *data);
  int (* get_join_status)(void);
  int (* get_config)(void);
};

/* Instância do driver */
extern const struct la66_driver LA66_DRIVER;

/* Processo */
PROCESS_NAME(la66_process);

#endif /* LA66_H_ */