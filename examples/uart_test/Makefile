CONTIKI_PROJECT = uart_test

APPS = uart_test_process # Nome do processo principal no uart_test.c

# Adicione suas bibliotecas auxiliares aqui
PROJECT_SOURCEFILES += uart_avr.c

# Remova ou comente as linhas que incluem uart_avr.c ou at_command.c se existirem
# # PROJECT_SOURCEFILES += uart_avr.c
# # PROJECT_SOURCEFILES += at_command.c

TARGET = arduino
ARDUINO_MODEL = uno

CONTIKI = ../..

all: $(CONTIKI_PROJECT).$(TARGET)

include $(CONTIKI)/Makefile.include
