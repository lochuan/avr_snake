#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "max7219.h"
#define F_CPU 16000000UL

typedef unsigned char uchar;
typedef struct Node{
    uint8_t x;
    uint8_t y;

    struct Node* next;
}node_t;

typedef struct Food{
	uint8_t x;
	uint8_t y;
}food_t;

typedef enum {up, down, left, right} Directions;
uint8_t scrolls_index1 = 0;
uint8_t screen_buffer1[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t buffer_row1;

volatile uint8_t tot_overflow;
volatile uint8_t speed = 40;
volatile uint8_t collision = 0;
volatile uint8_t running = 0;
uint8_t scores = 0;
node_t* head = NULL;
food_t* food = NULL;
Directions direction = up;
const uchar digit[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67 };

const uint8_t scrolls[100] PROGMEM = {
  SCROLL________,
  SCROLL________,
  SCROLL________,
  SCROLL________,
  SCROLLX______X,
  SCROLLX______X,
  SCROLLXX____XX,
  SCROLLXX____XX,
  SCROLLXXX___XX,
  SCROLLXXX___XX,
  SCROLLXX____XX,
  SCROLLX______X,
  SCROLL________,
  SCROLL________,
  SCROLL_______X,
  SCROLL______XX,
  SCROLL_____XXX,
  SCROLL____XXXX,
  SCROLL___XXXXX,
  SCROLLX___XXXX,
  SCROLLXX___XXX,
  SCROLLXXX___XX,
  SCROLLXXXX___X,
  SCROLLXXXXX___,
  SCROLLXXXX___X,
  SCROLLXXX___XX,
  SCROLLXX___XXX,
  SCROLLX___XXXX,
  SCROLL___XXXXX,
  SCROLLX___XXXX,
  SCROLLXX___XXX,
  SCROLLXX____XX,
  SCROLLX______X,
  SCROLLX______X,
  SCROLL________,
  SCROLL________,
  SCROLL________,
  SCROLL____X___,
  SCROLL____X___,
  SCROLL____X___,
  SCROLL___XX___,
  SCROLL___XX___,
  SCROLL___XX___,
  SCROLL____X___,
  SCROLL____X___,
  SCROLL____X___,
  SCROLL________,
  SCROLL________,
  SCROLL________,
  SCROLLX______X,
  SCROLLX______X,
  SCROLLXX____XX,
  SCROLLXX____XX,
  SCROLLX______X,
  SCROLLX______X,
  SCROLL________,
  SCROLL________,
  SCROLL________,
  SCROLL___X____,
  SCROLL___X____,
  SCROLL___X____,
  SCROLL___XX___,
  SCROLL___XX___,
  SCROLL___XX___,
  SCROLL___X____,
  SCROLL___X____,
  SCROLL___X___X,
  SCROLL___X___X,
  SCROLL___X___X,
  SCROLL___X____,
  SCROLL____X___,
  SCROLL____X___,
  SCROLL____X___,
  SCROLL________,
  SCROLL________,
  SCROLLX______X,
  SCROLLXX____XX,
  SCROLLXX___XXX,
  SCROLLX____XXX,
  SCROLL____XXXX,
  SCROLL___XXXXX,
  SCROLL___XXXXX,
  SCROLL___XXXXX,
  SCROLLX___XXXX,
  SCROLLXX___XXX,
  SCROLLXX____XX,
  SCROLLX______X,
  SCROLL________,
  SCROLL________,
  SCROLLXXXXXXXX,
  SCROLL________,
  SCROLLXXXXXXXX,
  SCROLL________,
  SCROLLXXXXXXXX,
  SCROLL________,
  SCROLLXXXXXXXX,
  SCROLL________,
  SCROLLXXXXXXXX,
  SCROLL________,
  SCROLLXXXXXXXX
};

void init_snake(int init_x, int init_y);
void push_head(node_t* head2go);
void pop_tail();
void get_direction();
void init_food(void);
void generate_food(void);
void move(void);
void output(void);
void timer1_init();
void check_collision(void);
void button_init();
void control_fnd(uchar *nums);
void keep_fnd(uchar *nums, int sec);
void init_fnd();
void print_scores(int cnt);
void reset_game();
void screen_saver();

ISR(TIMER1_OVF_vect)
{
    tot_overflow++;
  
    // 30 overflows = 1 seconds delay (approx.)
    if (tot_overflow >= speed) 
    {
        if(running) move();
        tot_overflow = 0;   // reset overflow counter
    }
}
ISR(INT5_vect)
{
    if(running == 1) running = 0;
    else running = 1;
}
int main(void)
{
    init_fnd();
    button_init();
    max7219_init();
    timer1_init();

    init_snake(rand()%8, rand()%8);
    init_food();
    while(1){
        if(running){
          output();
          print_scores(scores);
          get_direction();
          _delay_ms(2);
          check_collision();
          while(collision){
              reset_game();
              running = 0;
          }
        }
        if(!running){
          screen_saver();
          print_scores(scores);
        }
    }
}
void print_scores(int cnt) {
  int i, tmp = cnt;
  uchar nums[4] = {0,};
  for(i=3;i>=0;i--) {
    nums[i] = tmp % 10;
    tmp /= 10;
  }

  keep_fnd(nums, 1);
}
void control_fnd(uchar *nums) {
  int i = 0x08;
  int j = 0;

  for(; i>0; i= i >>1, j++) {
    PORTG = i;
    PORTC = digit[nums[j]];
    _delay_ms(1);
  }
}

void keep_fnd(uchar *nums, int sec) {
  int cnt = sec*250;
  while(cnt--) {
    control_fnd(nums);
  }
}
void init_fnd() {
  DDRC = 0xff;
  DDRG = 0x0f;

}
void button_init()
{
    DDRF = 0x00;
    EICRB = 0x0A;
    EIMSK = 0x30;
}
void pop_tail()
{
    node_t* current_head = head;
    
    while(current_head->next->next != NULL){
        current_head = current_head->next;
    }
    free(current_head->next);
    current_head->next = NULL;
}

void push_head(node_t* head2go)
{
   node_t* current_head = head;
   head2go->next = current_head;
   head = head2go; 
}

void init_snake(int init_head_x, int init_head_y)
{
   head = (node_t*)malloc(sizeof(node_t));
   head->x = init_head_x;
   head->y = init_head_y;
   head->next = NULL;
   node_t* tail = (node_t*)malloc(sizeof(node_t));
   tail->x = init_head_x - 1;
   tail->y = init_head_y;
   head->next = tail;
   tail->next = NULL;
}

void get_direction()
{
  if((!(PINF & (1<<PINF0))) && direction != down){
            direction = up;
        }
        if((!(PINF & (1<<PINF1))) && direction != up){
            direction = down;
        }
        if((!(PINF & (1<<PINF2))) && direction != right){
            direction = left;
        }
        if((!(PINF & (1<<PINF3))) && direction != left){
            direction = right;
        }
}

void init_food(void)
{
  food = (food_t*)malloc(sizeof(node_t));
  food->x = 5;
  food->y = 6;
}

void generate_food(void)
{
  food->x = rand()%8;
  food->y = rand()%8;
}
void reset_game()
{
  node_t* temp_head;
  scores = 0;
  while(head != NULL){
    temp_head = head;
    head = head->next;
    free(temp_head);
  }
  init_snake(rand()%8, rand()%8);
  init_food();
  speed = 40;
  collision = 0;
}
void move(void)
{
  node_t* temp_head = (node_t*)malloc(sizeof(node_t));
  int x = head->x;
  int y = head->y;

  switch(direction){
    case up:
      x += 1;
      break;
    case down:
      x -= 1;
      break;
    case left:
      y -= 1;
      break;
    case right:
      y += 1;
      break;
    default:
      break;
  }
  if(x > 7) x = 0;
  if(y > 7) y = 0;
  if(x < 0) x = 7;
  if(y < 0) y = 7;
  temp_head->x = x;
  temp_head->y = y;
  push_head(temp_head);

  if(head->x == food->x && head->y == food->y){
    generate_food();
    scores += 5;
    speed -= 3;
    if(speed < 3) speed = 3;
  }
  else{
    pop_tail();
  }
}

void output(void)
{
  max7219b_out();
  node_t* current_head = head;
  int x, y;

  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      max7219b_clr(i, j);
    }
  }
  while(current_head != NULL){
    max7219b_set(current_head->x, current_head->y);
    current_head = current_head->next;
  }
    max7219b_set(food->x, food->y);
}
void timer1_init()
{
    TCCR1B |= (1 << CS11);
  
    // initialize counter
    TCNT1 = 0;
  
    // enable overflow interrupt
    TIMSK |= (1 << TOIE1);
  
    // enable global interrupts
    sei();
  
    // initialize overflow counter variable
    tot_overflow = 0;
}

void check_collision(void)
{
    node_t* current_head = head;
    uint8_t x = current_head->x;
    uint8_t y = current_head->y;

    do{
        current_head = current_head->next;
        if(current_head != NULL && current_head->x == x && current_head->y == y) collision = 1;
    }while(current_head != NULL);
}

void screen_saver(){

  for (uint8_t screen_row = 8; screen_row >= 1; screen_row--) {
      buffer_row1 = screen_row - 1;
      // Output the buffer
      max7219_row(screen_row, screen_buffer1[buffer_row1]);

      // Scroll the row down
      if (buffer_row1 > 0) {
        screen_buffer1[buffer_row1] = screen_buffer1[buffer_row1 - 1];
      } else {
        screen_buffer1[buffer_row1] = pgm_read_byte(&scrolls[scrolls_index1++]);
        if (scrolls_index1 >= sizeof(scrolls) - 1) scrolls_index1 = 0;
      }
    }

    _delay_ms(10);

}
