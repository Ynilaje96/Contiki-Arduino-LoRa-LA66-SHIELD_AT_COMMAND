CONTIKI_PROJECT = teste_la66
CONTIKI = ../..

TARGET = arduino
ARDUINO_MODEL = uno

PROJECT_SOURCEFILES += software_uart_serial_line.c \
                       la66.c \
                       at-master.c


MODULES += $(CONTIKI)/core/dev/serial-line

all: $(CONTIKI_PROJECT).$(TARGET)

include $(CONTIKI)/Makefile.include
