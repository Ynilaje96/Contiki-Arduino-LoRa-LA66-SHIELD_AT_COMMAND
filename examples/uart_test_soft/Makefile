CONTIKI_PROJECT = uart_test_soft

APPS = uart_test_soft
# Este nome DEVE corresponder a `CONTIKI_PROJECT` e ao nome do arquivo .c principal

# Adicione suas bibliotecas auxiliares aqui
PROJECT_SOURCEFILES += software_uart.c
# Este é o arquivo .c que contém as funções soft_uart_init, etc.

TARGET = arduino
ARDUINO_MODEL = uno

CONTIKI = ../..

all: $(CONTIKI_PROJECT).$(TARGET)

include $(CONTIKI)/Makefile.include
