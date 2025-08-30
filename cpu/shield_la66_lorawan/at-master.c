/*
 * Copyright (c) 2015, Zolertia - http://www.zolertia.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
 
#include "contiki.h"
#include "contiki-lib.h"
#include "at-master.h"
#include "la66.h"


#include "software_uart_serial_line.h" //Minha Alteracao
#include "serial-line.h" // Minha Alteracao

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

process_event_t at_cmd_received_event; // <--- Adicione esta linha se não tiver
// Declaração da lista de comandos AT

static uint8_t at_uart; // <-- ADICIONE ESTA LINHA AQUI!

process_event_t at_cmd_received_event; // Definição global do evento
// ...
PROCESS(at_process, "AT Master Process"); // Declaração do processo
// ...
/*---------------------------------------------------------------------------*/
// Este bloco DEVE ESTAR em at-master.c e completo
PROCESS_THREAD(at_process, ev, data)
{
  PROCESS_BEGIN();

  // ... (código do thread, incluindo tratamento de serial_line_event_message) ...

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
// ... (o resto do seu at-master.c, incluindo at_init e at_register)



LIST_STRUCT(at_cmd_list); // <--- ADICIONE ESTA LINHA!



// ... (restante dos includes) ...

// ... (LIST e process_event_t at_cmd_received_event) ...

// Remover esta variável estática at_uart se você só vai usar a Soft UART
// static uint8_t at_uart = 0; // Se remover, adapte at_init e at_send

//PROCESS(at_process, "AT process");

// ... (PROCESS_THREAD at_process) ...

uint8_t
at_send(char *s, uint8_t len)
{
  uint8_t i = 0;
  while(s && *s != 0) {
    if(i >= len) {
      break;
    }
    // MODIFICAÇÃO AQUI: Chamar sua Soft UART
    soft_uart_write_byte(*s++);
    i++;
  }
  return i;
}

void at_init(uint8_t uart_sel)
{
  static uint8_t inited = 0;
  if(!inited) {
    list_init(at_cmd_list);
    at_cmd_received_event = process_alloc_event(); // Aloca o evento aqui
    inited = 1;

    //at_uart = uart_sel;
    //uart_init(at_uart);
    //uart_set_input(at_uart, serial_line_input_byte);
    serial_line_init();

    process_start(&at_process, NULL); // Inicia o processo
    printf("AT: Started (%u)\n", at_uart); // Use printf
  }
}

// ... (at_register) ...