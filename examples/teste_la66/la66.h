#ifndef LA66_H_
#define LA66_H_

#include "contiki.h"

/* Tipos de callback */
typedef void (*la66_response_callback_t)(char *data, uint16_t len);

/* Declaração do processo LA66, para que outros arquivos possam iniciá-lo */
PROCESS_NAME(la66_process); 

/* Estrutura do driver */
struct la66_driver {
  void (* init)(void);
  int (* join_network)(uint8_t mode);
  int (* send_data)(uint8_t port, uint8_t confirm, const char *data);
  int (* get_join_status)(void);
  int (* get_config)(void);
  
  // Novas funções para callbacks
  void (* set_response_callback)(la66_response_callback_t cb);
  void (* set_join_callback)(la66_response_callback_t cb);
  void (* set_error_callback)(la66_response_callback_t cb);
};

/* Instância do driver */
extern const struct la66_driver LA66_DRIVER;

#endif /* LA66_H_ */