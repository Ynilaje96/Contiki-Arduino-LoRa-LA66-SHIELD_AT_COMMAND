#ifndef F_CPU
#define F_CPU 16000000UL  // ou 8000000UL, dependendo do clock do Arduino
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serial-line.h"
#include "software_uart_serial_line.h"

// Estado de recepção
typedef enum {
    RX_IDLE, RX_START_BIT, RX_DATA_BITS, RX_STOP_BIT_WAIT
} rx_state_t;

static volatile rx_state_t rx_current_state = RX_IDLE;
static volatile uint8_t rx_data_buffer;
static volatile uint8_t rx_bit_index;

// Buffer circular de recepção
static volatile uint8_t rx_buffer[SOFT_UART_RX_BUFFER_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;
volatile uint8_t soft_uart_rx_overflow = 0;

void soft_uart_init(void) {
    DDRB &= ~(1 << SOFT_UART_RX_PIN_BIT);
    DDRB |= (1 << SOFT_UART_TX_PIN_BIT);
    SOFT_UART_TX_HIGH();

    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT3);

    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS10);
    TIMSK1 &= ~(1 << OCIE1A);
}

void soft_uart_write_byte(uint8_t data) {
    cli();
    SOFT_UART_TX_LOW();
    _delay_us(SOFT_UART_BIT_PERIOD_US);

    for(uint8_t i = 0; i < 8; i++) {
        if(data & 0x01) SOFT_UART_TX_HIGH();
        else SOFT_UART_TX_LOW();
        _delay_us(SOFT_UART_BIT_PERIOD_US);
        data >>= 1;
    }

    SOFT_UART_TX_HIGH();
    _delay_us(SOFT_UART_BIT_PERIOD_US);
    sei();
}

int soft_uart_write_string(const char *str) {
    if(str == NULL) return -1;
    while(*str) soft_uart_write_byte(*str++);
    return 0;
}

uint8_t soft_uart_read_buffer(void) {
    if (rx_head == rx_tail) return 0;
    uint8_t c = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % SOFT_UART_RX_BUFFER_SIZE;
    return c;
}

ISR(PCINT0_vect) {
    if (!SOFT_UART_RX_READ() && rx_current_state == RX_IDLE) {
        rx_current_state = RX_START_BIT;
        PCICR &= ~(1 << PCIE0);
        OCR1A = SOFT_UART_CYCLES_PER_BIT + SOFT_UART_CYCLES_HALF_BIT;
        TCNT1 = 0;
        TIMSK1 |= (1 << OCIE1A);
    }
}

ISR(TIMER1_COMPA_vect) {
    OCR1A = SOFT_UART_CYCLES_PER_BIT;

    switch(rx_current_state) {
        case RX_START_BIT:
            rx_data_buffer = 0;
            rx_bit_index = 0;
            rx_current_state = RX_DATA_BITS;
            break;

        case RX_DATA_BITS:
            if(rx_bit_index < 8) {
                if(SOFT_UART_RX_READ()) {
                    rx_data_buffer |= (1 << rx_bit_index);
                }
                rx_bit_index++;
            }
            if(rx_bit_index >= 8) {
                rx_current_state = RX_STOP_BIT_WAIT;
            }
            break;

        case RX_STOP_BIT_WAIT:
            if(SOFT_UART_RX_READ()) {
                uint8_t next_head = (rx_head + 1) % SOFT_UART_RX_BUFFER_SIZE;
                if (next_head != rx_tail) {
                    rx_buffer[rx_head] = rx_data_buffer;
                    rx_head = next_head;
                    serial_line_input_byte(rx_data_buffer);
                } else {
                    soft_uart_rx_overflow = 1;
                }
            }
            rx_current_state = RX_IDLE;
            TIMSK1 &= ~(1 << OCIE1A);
            PCICR |= (1 << PCIE0);
            break;

        default:
            rx_current_state = RX_IDLE;
            TIMSK1 &= ~(1 << OCIE1A);
            PCICR |= (1 << PCIE0);
            break;
    }
}
