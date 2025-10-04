/*
  Author: Fahad Alduraibi
  Version: 1.1 (2025)
  Logicuino - Arduino Logic analyzer
  using an Arduino (ATmega328P) board.

  Features:
  - 6 digital channels on pins 2–7 (PD2–PD7)
  - Top 2 bits = 2-bit rolling counter (0..3) for sync
  - Fixed sample rate via Timer1 CTC mode
  - Circular buffer for captured samples
  - Direct UART register writes for high-speed TX

  Notes:
  - BUFFER_SIZE must be power of 2
  - BAUD_RATE must be high enough to keep up with SAMPLE_RATE
*/

#define SAMPLE_RATE 100000  // samples per second (Hz)
/*
  If using the origianl Arduino Uno with 'ATmega16U2' as 
  Serial-to-USB emulator I found that 70KHz is a safe value
  with few dropped samples, higher values as not stable.

  However, if the board has other converters such as
  'CH340G' then a faster speed might be possible.

  When testing CH340G, a rate of 100KHz was not missing any sample.
*/
#define BAUD_RATE 1000000  // UART baud rate
#define BUFFER_SIZE 1024   // must be power of 2 (e.g., 256, 512)

uint8_t buffer[BUFFER_SIZE];

// declared these as 'volatile' since they are accessed by the interrupt
volatile uint16_t head = 0;
volatile uint16_t tail = 0;
volatile uint8_t sample_counter = 0;  // A counter to enhance packet syncronization with the viewer

// --- UART Init ---
void uart_init() {
  uint16_t ubrr = (F_CPU / (16UL * BAUD_RATE)) - 1;
  UBRR0H = (ubrr >> 8) & 0xFF;
  UBRR0L = ubrr & 0xFF;
  UCSR0B = (1 << TXEN0);                       // TX enable only
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);      // 8N1
}

// --- UART Write (direct register) ---
inline void uart_write(uint8_t d) {
  while (!(UCSR0A & (1 << UDRE0))); // wait until ready
  UDR0 = d;
}

// --- Timer1 Init for fixed sampling ---
void timer1_init(uint32_t rate) {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;

  uint16_t prescaler = 8;
  OCR1A = (F_CPU / (prescaler * rate)) - 1;

  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS11);    // prescaler = 8
  TIMSK1 |= (1 << OCIE1A);  // enable compare interrupt
  interrupts();
}

/* The interrupt routine:
1- Read the port register (we need 6 bits = 6 digital inputs) and shift it into 'value'

2- Shift in the counter two bits

3- Push the value into the circular buffer (to be read in the loop function)
*/
ISR(TIMER1_COMPA_vect) {
  // Read PD2–PD7, shift into bits 0..5
  uint8_t value = (PIND >> 2) & 0x3F;

  // Add counter in bits 6–7
  value |= (sample_counter & 0x03) << 6;

  // Push to circular buffer
  uint16_t next = (head + 1) & (BUFFER_SIZE - 1);
  if (next != tail) {
    buffer[head] = value;
    head = next;
  }

  // Increment counter mod 4
  sample_counter = (sample_counter + 1) & 0x03;
}

void setup() {
  // Set Pins 2–7 as input
  DDRD &= ~0b11111100;

  uart_init();
  timer1_init(SAMPLE_RATE);
}

// Read the available data from the buffer and send it over serial
void loop() {
  while (tail != head) {
    uart_write(buffer[tail]);
    tail = (tail + 1) & (BUFFER_SIZE - 1);
  }
}
