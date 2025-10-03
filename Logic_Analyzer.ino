/*
  By Fahad Alduraibi
  2025
  Version: 1.0
  Logicuino - Arduino Logic analyzer
  using an Arduino (ATmega328P) board
  Pins 2–9 are used as Channels 1-7
  - Uses Timer1 for fixed sampling rate
  - Uses circular buffer in RAM
  - Sends data via hardware UART
*/

#define SAMPLE_RATE 80000    // samples per second (80 kHz)
#define BAUD_RATE   1000000   // UART baud rate
#define BUFFER_SIZE 1024      // must be power of 2 (e.g., 256, 512)

// declared these as 'volatile' since they are accessed by the interrupt
volatile uint8_t buffer[BUFFER_SIZE];
volatile uint16_t head = 0;
volatile uint16_t tail = 0;

void setup() {
  // Configure pins 2–9 as input
  DDRD &= ~0b11111100;  // PD2–PD7 input
  DDRB &= ~0b00000011;  // PB0–PB1 input

  Serial.begin(BAUD_RATE);

  // --- Timer1 setup ---
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;

  // Compare value for fixed sample rate
  uint16_t prescaler = 8;
  OCR1A = (F_CPU / (prescaler * SAMPLE_RATE)) - 1;

  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS11);    // prescaler = 8
  TIMSK1 |= (1 << OCIE1A);  // enable compare match interrupt
  interrupts();
}

/* The interrupt routine:
1- Read the two port register (we need 8 bits = 8 digital inputs)
    The atmega does not have one port register with 8 usable pins, so we have to use two ports.
    * Port D has 8pins but pin 0 & 1 are used for Serial Rx & Tx, which we are using to transmit.

2- Mask and Shift the needed bits into the variable 'value'
    6 bits from port D, and two bits from port B

3- Push the value into the circular buffer (to be read in the loop function)
*/
ISR(TIMER1_COMPA_vect) {
  uint8_t portD = PIND;   // read PORTD (pins 0–7)
  uint8_t portB = PINB;   // read PORTB (pins 8–13)

  // Pack PD2–PD7 (bits 2–7) and PB0–PB1 (bits 0–1) into one byte
  uint8_t value = (portD & 0b11111100) >> 2;  // PD2..PD7 -> bits 0..5
  value |= (portB & 0b00000011) << 6;         // PB0..PB1 -> bits 6..7

  // Push to buffer (lock-free, single producer/consumer)
  uint16_t nextHead = (head + 1) & (BUFFER_SIZE - 1);
  if (nextHead != tail) {
    buffer[head] = value;
    head = nextHead;
  }
}

// Just read the available data from the buffer and send it over serial
void loop() {
  while (tail != head) {
    Serial.write(buffer[tail]);
    tail = (tail + 1) & (BUFFER_SIZE - 1);
  }
}
