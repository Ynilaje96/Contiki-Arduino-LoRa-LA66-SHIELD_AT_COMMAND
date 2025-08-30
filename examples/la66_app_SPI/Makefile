CONTIKI_PROJECT = la66_app
APPS = la66_app

# Arquivos fonte
PROJECT_SOURCEFILES += uart_la66.c

# Flags
CFLAGS += -DUART_LA66_ENABLE=1
CFLAGS += -DBAUD_RATE=9600
CFLAGS += -DSPI_ENABLE=0  # 1 para habilitar SPI se necessário

# Hardware
TARGET = arduino
ARDUINO_MODEL = uno

# Caminho do Contiki (ajuste conforme necessário)
CONTIKI = ../..

all: $(CONTIKI_PROJECT).$(TARGET)

include $(CONTIKI)/Makefile.include
