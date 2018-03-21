
#ifndef MAX7219_H
#define MAX7219_H

// ----------------------------------------------------------------------------

#include <stdint.h>
#include <avr/io.h>

// ----------------------------------------------------------------------------

#ifndef MAX7219_DIN
#define MAX7219_DIN		PD0	// DI,	Pin 3 on LED8x8 Board
#endif
#ifndef MAX7219_CS
#define MAX7219_CS		PD1	// CS,	Pin 4 on LED8x8 Board
#endif
#ifndef MAX7219_CLK
#define MAX7219_CLK		PD2	// CLK,	Pin 5 on LED8x8 Board
#endif

#define SCROLL________ 0b00000000

#define SCROLL_______X 0b00000001
#define SCROLL______XX 0b00000011
#define SCROLL_____XXX 0b00000111
#define SCROLL____XXXX 0b00001111
#define SCROLL___XXXXX 0b00011111

#define SCROLLX_______ 0b10000000
#define SCROLLX______X 0b10000001
#define SCROLLX_____XX 0b10000011
#define SCROLLX____XXX 0b10000111
#define SCROLLX___XXXX 0b10001111

#define SCROLLXX______ 0b11000000
#define SCROLLXX_____X 0b11000001
#define SCROLLXX____XX 0b11000011
#define SCROLLXX___XXX 0b11000111

#define SCROLLXXX_____ 0b11100000
#define SCROLLXXX____X 0b11100001
#define SCROLLXXX___XX 0b11100011

#define SCROLLXXXX____ 0b11110000
#define SCROLLXXXX___X 0b11110001

#define SCROLLXXXXX___ 0b11111000

#define SCROLL____X___ 0b00001000
#define SCROLLX___X___ 0b10001000
#define SCROLL___X____ 0b00010000
#define SCROLL___X___X 0b00010001
#define SCROLL___XX___ 0b00011000

#define SCROLL_X_X_X_X 0b01010101
#define SCROLLX_X_X_X_ 0b10101010
#define SCROLLXXXXXXXX 0b11111111
// ----------------------------------------------------------------------------

void max7219_byte(uint8_t data);
void max7219_word(uint8_t address,  uint8_t data);
void max7219_init(void);
void max7219_row(uint8_t address,uint8_t data);

// ----------------------------------------------------------------------------

void max7219b_out(void);
void max7219b_set(uint8_t x, uint8_t y);
void max7219b_clr(uint8_t x, uint8_t y);
void max7219b_row(uint8_t row, uint8_t y);
void max7219bs_scheduler_userfunc(uint32_t scheduler_tick);

// ----------------------------------------------------------------------------
#endif
