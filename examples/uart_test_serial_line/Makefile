CONTIKI_PROJECT = uart_test_serial_line

APPS = uart_test_serial_line
# Nome do processo principal no uart_test_serial_line.c

# Adicione suas bibliotecas auxiliares aqui
PROJECT_SOURCEFILES += software_uart_serial_line.c
PROJECT_SOURCEFILES += serial-line.c


# Adicione o serial-line.c, que Contiki ja possui.
# O caminho padrao eh dev/serial-line.c

TARGET = arduino
ARDUINO_MODEL = uno

CONTIKI = ../..

all: $(CONTIKI_PROJECT).$(TARGET)

include $(CONTIKI)/Makefile.include
