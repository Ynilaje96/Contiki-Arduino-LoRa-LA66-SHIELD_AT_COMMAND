// soft_uart.c
#include "software_uart_serial_line.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay_basic.h> // Para _delay_loop_2 (para TX)

#include "serial-line.h" // Para alimentar o processo serial-line

// --- Variáveis de Estado de Recepção (manuseadas pelas ISRs) ---
// Estado atual da máquina de estados de recepção
typedef enum {
    RX_IDLE = 0,        // Aguardando start bit
    RX_START_BIT,       // Start bit detectado
    RX_DATA_BITS,       // Recebendo bits de dados
    RX_STOP_BIT_WAIT,   // Aguardando fim do último bit de dados para o stop bit
    RX_COMPLETE         // Byte completo recebido
} rx_state_t;

static volatile rx_state_t rx_current_state = RX_IDLE;
static volatile uint8_t rx_data_buffer; // Buffer para construir o byte recebido
static volatile uint8_t rx_bit_index;   // Índice do bit atual sendo recebido (0 a 7)


// --- Funções de Inicialização ---

void soft_uart_init(void) {
    // 1. Configurar Pinos:
    // Pino RX (PB3) como entrada com pull-up
    SOFT_UART_RX_DDR_REG &= ~(1 << SOFT_UART_RX_PIN);
    SOFT_UART_RX_PORT_REG |= (1 << SOFT_UART_RX_PIN); // Habilita pull-up (opcional, mas bom)

    // Pino TX  (PB2) como saída, estado ocioso HIGH
    SOFT_UART_TX_DDR_REG |= (1 << SOFT_UART_TX_PIN_BIT);
    SOFT_UART_TX_HIGH();

    // Correção para PB3 (D11)
    PCICR |= (1 << PCIE0); // Habilita interrupções de mudança de pino para PCINT7:0 (PORTB)
    PCMSK0 |= (1 << SOFT_UART_RX_PCINT_BIT); // SOFT_UART_RX_PCINT_BIT deve ser PCINT3

    // 3. Configurar Timer1 para amostragem de bits de recepção:
    // Usaremos o Timer1 no modo CTC (Clear Timer on Compare Match) para precisão.
    // Prescaler: 1 (sem divisão de clock) para máxima resolução.
    // O valor de OCR1A será o número de ciclos para o período de 1 bit (ou 0.5 bit inicialmente).

    // TCCR1B: Timer/Counter1 Control Register B
    TCCR1B |= (1 << WGM12); // Modo CTC, TOP = OCR1A
    TCCR1B |= (1 << CS10);  // Prescaler de 1 (clk/1)
    // TCCR1A = 0; // Não precisamos de modos de saída OCxA/OCxB

    // Desabilita interrupções do Timer1 inicialmente
    TIMSK1 &= ~((1 << OCIE1A) | (1 << TOIE1)); // Desabilita Output Compare A Match e Overflow Interrupts

    // Habilita as interrupções globais
    sei();
}

// --- Funções de Transmissão (TX) ---

void soft_uart_write_byte(uint8_t data) {
    uint8_t i;

    // Garante que nenhuma interrupção de recepção ocorra durante a transmissão
    // para evitar corrupção de dados ou estado inválido.
    // (Pode ser melhorado para não desabilitar GLOBALMENTE, mas para este caso simples funciona)
    cli(); // Desabilita interrupções globais temporariamente

    // Start bit (LOW)
    SOFT_UART_TX_LOW();
    _delay_loop_2(SOFT_UART_TX_DELAY_LOOP_COUNT);

    // Data bits (8 bits, LSB first)
    for (i = 0; i < 8; i++) {
        if (data & 0x01) {
            SOFT_UART_TX_HIGH(); // Bit 1
        } else {
            SOFT_UART_TX_LOW();  // Bit 0
        }
        _delay_loop_2(SOFT_UART_TX_DELAY_LOOP_COUNT);
        data >>= 1; // Próximo bit
    }

    // Stop bit (HIGH)
    SOFT_UART_TX_HIGH();
    _delay_loop_2(SOFT_UART_TX_DELAY_LOOP_COUNT); // Aguarda o tempo do stop bit

    sei(); // Reabilita interrupções globais
}

void soft_uart_write_string(const char *str) {
    while (*str) {
        soft_uart_write_byte(*str++);
    }
}

// soft_uart.c (TRECHO CORRIGIDO)

// ISR para interrupção de mudança de pino (PCINT0_vect) para PORTB (onde está PB3/D11)
ISR(PCINT0_vect) { // CORRIGIDO: Era PCINT2_vect
    // Verifica se o pino RX está LOW (indicando um start bit) E se estamos no estado ocioso.
    if (!SOFT_UART_RX_READ() && rx_current_state == RX_IDLE) {
        rx_current_state = RX_START_BIT; // Transição para o estado de start bit

        // Desabilita PCINT para evitar múltiplas interrupções durante a recepção do byte
        // CORRIGIDO: Era PCIE2, agora PCIE0 para PORTB
        PCICR &= ~(1 << PCIE0);

        // Configura o Timer1 para o meio do primeiro bit de dados (1.5 tempos de bit)
        OCR1A = SOFT_UART_CYCLES_PER_BIT + SOFT_UART_CYCLES_HALF_BIT;
        TCNT1 = 0; // Zera o contador do Timer1
        TIMSK1 |= (1 << OCIE1A); // Habilita a interrupção de comparação do Timer1 (OCIE1A)
    }
}

// ISR para comparação de Timer1 A (TIMER1_COMPA_vect)
// Esta ISR é acionada quando TCNT1 atinge o valor em OCR1A.
// Usamos isso para amostrar os bits de dados e o stop bit.
ISR(TIMER1_COMPA_vect) {
    // Configura OCR1A para o próximo período de 1 bit completo.
    // Isso garante que as próximas interrupções ocorram no meio dos bits seguintes.
    OCR1A = SOFT_UART_CYCLES_PER_BIT;

    switch (rx_current_state) {
        case RX_START_BIT:
            // Já atrasamos para o meio do primeiro bit de dados.
            // Agora, inicializa o buffer e o índice e transita para o estado de dados.
            rx_data_buffer = 0;
            rx_bit_index = 0;
            rx_current_state = RX_DATA_BITS;
            // Não há 'break' aqui intencionalmente, para que a lógica de 'RX_DATA_BITS' execute imediatamente
            // para amostrar o primeiro bit de dados.

        case RX_DATA_BITS:
            if (rx_bit_index < 8) {
                // Amostra o bit atual no pino RX
                if (SOFT_UART_RX_READ()) {
                    rx_data_buffer |= (1 << rx_bit_index); // Adiciona o bit ao buffer
                }
                rx_bit_index++; // Move para o próximo bit
            }

            if (rx_bit_index == 8) {
                // Todos os 8 bits de dados foram recebidos.
                // Agora, esperamos pelo stop bit antes de finalizar e alimentar o serial-line.
                rx_current_state = RX_STOP_BIT_WAIT;
            }
            break;

        case RX_STOP_BIT_WAIT:
            // Verificamos o estado do stop bit. Ele deve ser HIGH.
            // Se for LOW, indica um erro de framing (opcional de verificar).
            // Neste ponto, o byte está completamente recebido.
            // Alimente o byte para o processo serial-line do Contiki.
            serial_line_input_byte(rx_data_buffer);

            // Reseta o estado para aguardar o próximo byte
            rx_current_state = RX_IDLE;
            TIMSK1 &= ~(1 << OCIE1A);   // Desabilita a interrupção do Timer1
            PCICR |= (1 << PCIE0); // Reabilita PCINT para detectar o próximo start bit no PORTB (onde está D11/PB3)            
            break;

        default:
            // Estado inválido - reinicia para idle e reabilita PCINT
            rx_current_state = RX_IDLE;
            TIMSK1 &= ~(1 << OCIE1A);
            PCICR |= (1 << PCIE0); // Reabilita PCINT para detectar o próximo start bit no PORTB (onde está D11/PB3)
            break;
    }
}