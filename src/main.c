#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include "util/delay.h"
#include "usart.h"
#include "bit.h"

#define START_BTN PC0
#define RESET_BTN PC1

void init_timer1();
void check_btns();

volatile int num_seconds;
volatile bool start_pressed;
volatile bool reset_pressed;

volatile uint8_t display[] = {
  0b00111111,
  0b00111111,
  0b00111111,
  0b00111111,
};

volatile uint8_t digits[] = {
  0b00111111,
  0b00000110,
  0b01011011,
  0b01001111,
  0b01100110,
  0b01101101,
  0b01111101,
  0b00000111,
  0b01111111,
  0b01101111,
};

ISR(TIMER1_COMPA_vect) {
  num_seconds++;
}


int main(void) {
  DDRD = 0xFF;
  DDRB = 0xFF;
  bitClear(DDRC, START_BTN);
  bitClear(DDRC, RESET_BTN);
  // enable pullup resistor
  bitSet(PORTC, START_BTN);
  bitSet(PORTC, RESET_BTN);

  init_timer1();
  sei();

  int pos = 0;
  while (1) {
    _delay_us(50);
    PORTB |= 0b00001111;
    display[3] = digits[num_seconds % 10];
    display[2] = digits[(num_seconds / 10) % 6];
    display[1] = digits[(num_seconds / 60) % 10];
    display[0] = digits[(num_seconds / 60)/10 % 10];
    display[1] |= 0b10000000;
    PORTD = display[pos];
    bitClear(PORTB, pos);
    pos = (pos+1) % 4;

    check_btns();
  }
}

void init_timer1() {
  bitSet(TIMSK1, OCIE1A);
  bitSet(TCCR1B, WGM12);
  OCR1A = 62500;
}

void check_btns() {

  if (!bitRead(PINC, PC0) || !bitRead(PINC, PC1)) {
    if (start_pressed || reset_pressed) {
      return;
    }
    _delay_us(25);
    if (!bitRead(PINC, PC0)) {
      start_pressed = true;
      bitInverse(TCCR1B, CS12);
      bitInverse(PORTB, PB5);
    }

    if (!bitRead(PINC, PC1)) {
      reset_pressed = true;
      num_seconds = 0;
    }
  } else {
    start_pressed = false;
    reset_pressed = false;
  }
}
