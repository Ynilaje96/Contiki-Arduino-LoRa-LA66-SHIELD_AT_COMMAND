CONTIKI_PROJECT = la66_at_commands

# Caminho para a raiz do Contiki
CONTIKI = ../..

# Plataforma e modelo
TARGET = arduino
ARDUINO_MODEL = uno


# Fontes do projeto
PROJECT_SOURCEFILES += at-master.c
PROJECT_SOURCEFILES += la66.c
PROJECT_SOURCEFILES += software_uart_serial_line.c
PROJECT_SOURCEFILES += serial-line.c

CFLAGS += -I$(CONTIKI)/core/dev
CFLAGS += -I$(CONTIKI)/cpu/avr

# ATENÇÃO: serial-line.c e software_uart_serial_line.c são parte do Contiki.
# Não os inclua diretamente a menos que esteja usando versões customizadas.

# Regra padrão do Contiki
all: $(CONTIKI_PROJECT).$(TARGET)

include $(CONTIKI)/Makefile.include
