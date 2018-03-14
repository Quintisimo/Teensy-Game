#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <macros.h>
#include <sprite.h>
#include <string.h>
#include "lcd_model.h"
#include "usb_serial.h"
#include "cab202_adc.h"

typedef enum {false, true} bool;

#define FREQ (8000000.0)
#define PRESCALE (64.0)
#define XC (LCD_X / 2)
#define YC (LCD_Y / 2)
#define IN_RANGE(value, min, max) (value < max && value > min)

Sprite person;
#define PERSON_HEIGHT (13)
#define PERSON_WIDTH (10)
#define PERSON_VELOCITY (0.6)
uint8_t person_bitmap[] = {
	0b00011111, 0b00000000,
	0b00010001, 0b00000000,
	0b00010001, 0b00000000,
	0b00011111, 0b00000000,
	0b00000100, 0b00000000,
	0b00000100, 0b00000000,
	0b01111111, 0b11100000,
	0b00000100, 0b00000000,
	0b00000100, 0b00000000,
	0b00000100, 0b00000000,
	0b00001010, 0b00000000,
	0b00010001, 0b00000000,
	0b00100000, 0b10000000
};

Sprite door;
#define DOOR_HEIGHT (16)
#define DOOR_WIDTH (12)
uint8_t door_bitmap[] = {
	0b11111111, 0b11110000,
	0b10000000, 0b00010000,
	0b10111111, 0b11010000,
	0b10100000, 0b01010000,
	0b10100000, 0b01010000,
	0b10111111, 0b11010000,
	0b10000000, 0b00010000,
	0b10000000, 0b11010000,
	0b10000000, 0b00010000,
	0b10111111, 0b11010000,
	0b10100000, 0b01010000,
	0b10100000, 0b01010000,
	0b10111111, 0b11010000,
	0b10000000, 0b00010000,
	0b10000000, 0b00010000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000
};

Sprite key;
#define KEY_HEIGHT (3)
#define KEY_WIDTH (8)
int key_x;
int key_y;
bool has_key;
bool send_key;
uint8_t key_bitmap[] = {
	0b00000111,
	0b11111101,
	0b10100111,
};

Sprite castle;
#define CASTLE_HEIGHT (YC)
#define CASTLE_WIDTH (LCD_X - 13)
uint8_t castle_bitmap[] = {
	0b11101110, 0b11101110, 0b11101110, 0b11101110, 0b11101110, 0b11101110, 0b11101110, 0b11101110, 0b11101110,
	0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
	0b10111011, 0b10111011, 0b10111011, 0b10111011, 0b10111011, 0b10111011, 0b10111011, 0b10111011, 0b10111010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111110,
};

Sprite monster;
#define MONSTER_HEIGHT (5)
#define MONSTER_WIDTH (8)
#define MONSTER_VELOCITY (0.1)
uint8_t monster_bitmap[] = {
	0b00111100,
	0b01111110,
	0b11011011,
	0b11111111,
	0b11011011,
};

Sprite treasure;
#define TREASURE_HEIGHT (3)
#define TREASURE_WIDTH (5)
bool has_treasure;
bool send_treasure;
uint8_t treasure_bitmap[] = {
	0b00100000,
	0b01110000,
	0b11111000
};

Sprite sheild;
#define SHEILD_HEIGHT (13)
#define SHEILD_WIDTH (11)
bool has_sheild;
bool send_sheild;
bool defence;
uint8_t sheild_bitmap[] = {
	0b11111111, 0b11100000,
	0b11111111, 0b11100000,
	0b11111111, 0b11100000,
	0b11111111, 0b11100000,
	0b11111111, 0b11100000,
	0b11111111, 0b11100000,
	0b11111111, 0b11100000,
	0b11111111, 0b11100000,
	0b01111111, 0b11000000,
	0b00111111, 0b10000000,
	0b00011111, 0b00000000,
	0b00001110, 0b00000000,
	0b00000100, 0b00000000
};

Sprite bomb;
#define BOMB_HEIGHT (7)
#define BOMB_WIDTH (7)
bool has_bomb;
bool send_bomb;
bool exploded;
uint8_t bomb_bitmap[] = {
	0b00010000,
	0b00010000,
	0b00111000,
	0b01111100,
	0b01111100,
	0b01111100,
	0b00111000
};

Sprite crosshair;
#define CROSSHAIR_HEIGHT (5)
#define CROSSHAIR_WIDTH (5)
uint8_t crosshair_bitmap[] = {
	0b00100000,
	0b00100000,
	0b11011000,
	0b00100000,
	0b00100000
};

Sprite bow;
#define BOW_HEIGHT (8)
#define BOW_WIDTH (5)
bool send_bow;
bool has_bow;
uint8_t bow_bitmap[] = {
	0b11000000,
	0b10100000,
	0b10010000,
	0b10001000,
	0b10001000,
	0b10010000,
	0b10100000,
	0b11000000
};

Sprite arrow;
#define ARROW_HEIGHT (2)
#define ARROW_WIDTH (2)
int arrows;
bool arrowed;
bool fired;
uint8_t arrow_bitmap[] = {
	0b11000000,
	0b11000000
};

Sprite vertical_wall;
#define VERTICAL_WALL_HEIGHT (10);
#define VERTICAL_WALL_WIDTH (3);
uint8_t vertical_wall_bitmap[] = {
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
};

Sprite horizontal_wall;
#define HORIZONTAL_WALL_HEIGHT (3);
#define HORIZONTAL_WALL_WIDTH (17);
uint8_t horizontal_wall_bitmap[] = {
	0b11111111, 0b11111111, 0b10000000,
	0b11111111, 0b11111111, 0b10000000,
	0b11111111, 0b11111111, 0b10000000,
};

bool start_game;
bool start_counter;
// bool hidden_level = false;
bool respawn = false;

int countdown = 3;
int score = 0;
int lives = 3;
int level = 0;

int timer = 0;
int timer_2 = 0;

volatile uint32_t overflow_counter = 0;

volatile uint8_t center = 0;
volatile bool center_pressed;

volatile uint8_t up = 0;
volatile bool up_pressed;

volatile uint8_t down = 0;
volatile bool down_pressed;

volatile uint8_t left = 0;
volatile bool left_pressed;

volatile uint8_t right = 0;
volatile bool right_pressed;

void draw_int(uint8_t x, uint8_t y, int value, colour_t colour) {
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "%d", value);
	draw_string(x, y, buffer, colour);
}

void setup(void) {
	set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
	SET_BIT(PINC, 7);
	lcd_clear();

	usb_init();
	while(!usb_configured()) {
		draw_string(XC, 2, "USB Serial is geting ready \r\n", FG_COLOUR);
	}
	if (usb_configured()) usb_serial_send("The game is ready to play \r\n");

	adc_init();

	// SW2
  CLEAR_BIT(DDRF, 6);

	// SW3
  CLEAR_BIT(DDRF, 5);

	// Joystick Center
  CLEAR_BIT(DDRB, 0);

	// Joystick Up
	CLEAR_BIT(DDRD, 1);

	// Joystick Down
	CLEAR_BIT(DDRB, 7);

	// Joystick Left
	CLEAR_BIT(DDRB, 1);

	// Joystick Right
	CLEAR_BIT(DDRD, 0);

	// Left LED
	SET_BIT(DDRB, 2);

	// Right LED
	SET_BIT(DDRB, 3);

  TCCR1A = 0;
  TCCR1B = 3;
  TIMSK1 = 1;

	TCCR0A = 0;
	TCCR0B = 4;
  TIMSK0 = 1;

  sei();
}

bool sprite_collided(Sprite sprite_1, Sprite sprite_2) {
  bool collided = true;

  int sprite_1x = sprite_1.x;
  int sprite_1y = sprite_1.y;
  int sprite_1r = sprite_1x + sprite_1.width - 1;
  int sprite_1b = sprite_1y + sprite_1.height - 1;

  int sprite_2x = sprite_2.x;
  int sprite_2y = sprite_2.y;
  int sprite_2r = sprite_2x + sprite_2.width - 1;
  int sprite_2b = sprite_2y + sprite_2.height - 1;

  if (sprite_1r < sprite_2x) collided = false;
  if (sprite_1b < sprite_2y) collided = false;
  if (sprite_2r < sprite_1x) collided = false;
  if (sprite_2b < sprite_1y) collided = false;

  return collided;
}

void setup_game(void) {
	has_key = false;
	send_key = true;
	send_treasure = true;
	has_sheild = false;
	send_sheild = true;
	defence = false;
	has_bomb = false;
	send_bomb = true;
	exploded = false;
	has_bow = false;
	send_bow = true;
	arrows = 5;
	fired = false;
	arrowed = false;

	int person_x = 0;
	int person_y = 0;
	int monster_x = 0;
	int monster_y = 0;
	int door_x = 0;
	int door_y = 0;
	int treasure_x = 0;
	int treasure_y = 0;
	int sheild_x = 0;
	int sheild_y= 0;
	int bomb_x = 0;
	int bomb_y = 0;
	int bow_x = 0;
	int bow_y = 0;
	int sheild_probability;
	int bomb_probability;
	int bow_probability;

	if (level == 0) {
		monster_x = LCD_X - MONSTER_WIDTH - 5;
		monster_y = CASTLE_HEIGHT + MONSTER_HEIGHT + 4;
		person_x = XC - (PERSON_WIDTH / 2);
		person_y = CASTLE_HEIGHT + (PERSON_HEIGHT / 2);

		if (!respawn) {
			key_x = 5;
			key_y = YC + 8;

			sprite_init(&monster, monster_x, monster_y, MONSTER_WIDTH, MONSTER_HEIGHT, monster_bitmap);
			sprite_init(&door, XC - 5, YC - DOOR_HEIGHT + 3, DOOR_WIDTH, DOOR_HEIGHT, door_bitmap);
			sprite_init(&castle, 6, 2, CASTLE_WIDTH, CASTLE_HEIGHT, castle_bitmap);
			sprite_init(&person, person_x, person_y, PERSON_WIDTH, PERSON_HEIGHT, person_bitmap);
			sprite_init(&key, key_x, key_y, KEY_WIDTH, KEY_HEIGHT, key_bitmap);
		} else {
			sprite_init(&monster, monster_x, monster_y, MONSTER_WIDTH, MONSTER_HEIGHT, monster_bitmap);
			sprite_init(&person, person_x, person_y, PERSON_WIDTH, PERSON_HEIGHT, person_bitmap);
			sprite_init(&key, key_x, key_y, KEY_WIDTH, KEY_HEIGHT, key_bitmap);
		}

	} else if (level == 1) {

			person_x = rand() % ((LCD_X - PERSON_WIDTH - 5) + 1 - 1) + 1;
			person_y = rand() % ((LCD_Y - PERSON_HEIGHT - 5) -1 + 1) + 1;

		if (!respawn) {
			monster_x = rand() % ((LCD_X - MONSTER_WIDTH) + 1 - (XC - MONSTER_WIDTH)) + (XC - MONSTER_WIDTH);
			monster_y = rand() % ((LCD_Y - MONSTER_HEIGHT) + 1 - (YC - MONSTER_HEIGHT)) + (YC - MONSTER_HEIGHT);
			door_x = rand() % ((LCD_X - DOOR_WIDTH) + 1 - (XC - DOOR_WIDTH)) + (XC - DOOR_HEIGHT);
			door_y = rand() % ((YC - DOOR_HEIGHT) + 1 - 10) + 10;
			key_x = rand() % ((XC - KEY_WIDTH) + 1 - 10) + 10;
			key_y = rand() % ((YC - KEY_HEIGHT) + 1 - 10) + 10;
			treasure_x = rand() % ((XC - TREASURE_WIDTH) + 1 - 10) + 10;
			treasure_y = rand() % ((LCD_Y - TREASURE_HEIGHT) + 1 - (YC - TREASURE_HEIGHT)) + (YC - TREASURE_HEIGHT);

			sprite_init(&monster, monster_x, monster_y, MONSTER_WIDTH, MONSTER_HEIGHT, monster_bitmap);
			sprite_init(&door, door_x, door_y, DOOR_WIDTH, DOOR_HEIGHT, door_bitmap);
			sprite_init(&treasure, treasure_x, treasure_y, TREASURE_WIDTH, TREASURE_HEIGHT, treasure_bitmap);
			sprite_init(&person, person_x, person_y, PERSON_WIDTH, PERSON_HEIGHT, person_bitmap);
			sprite_init(&key, key_x, key_y, KEY_WIDTH, KEY_HEIGHT, key_bitmap);

			start_game = true;

		} else {
			sprite_init(&person, person_x, person_y, PERSON_WIDTH, PERSON_HEIGHT, person_bitmap);
			sprite_init(&key, key_x, key_y, KEY_WIDTH, KEY_HEIGHT, key_bitmap);
		}

	} else if (level > 2) {
		if (!respawn) {
			monster_x = rand() % ((LCD_X - MONSTER_WIDTH) + 1 - (XC - MONSTER_WIDTH)) + (XC - MONSTER_WIDTH);
			monster_y = rand() % ((LCD_Y - MONSTER_HEIGHT) + 1 - (YC - MONSTER_HEIGHT)) + (YC - MONSTER_HEIGHT);
			door_x = rand() % ((LCD_X - DOOR_WIDTH) + 1 - (XC - DOOR_WIDTH)) + (XC - DOOR_HEIGHT);
			door_y = rand() % ((YC - DOOR_HEIGHT) + 1 - 10) + 10;
			key_x = rand() % ((XC - KEY_WIDTH) + 1 - 10) + 10;
			key_y = rand() % ((YC - KEY_HEIGHT) + 1 - 10) + 10;
			treasure_x = rand() % ((XC - TREASURE_WIDTH) + 1 - 10) + 10;
			treasure_y = rand() % ((LCD_Y - TREASURE_HEIGHT) + 1 - (YC - TREASURE_HEIGHT)) + (YC - TREASURE_HEIGHT);
			do {
				start_game = false;
				monster_x = rand() % ((LCD_X - MONSTER_WIDTH) + 1 - 1) + 1;
				monster_y = rand() % ((LCD_Y - MONSTER_HEIGHT) + 1 - 1) + 1;
				door_x = rand() % ((LCD_X - DOOR_WIDTH) + 1 - 1) + 1;
				door_y = rand() % ((LCD_Y - DOOR_HEIGHT) + 1 - 1) + 1;
				key_x = rand() % ((LCD_X - KEY_WIDTH) + 1 - 1) + 1;
				key_y = rand() % ((LCD_Y - KEY_HEIGHT) + 1 - 1) + 1;
				treasure_x = rand() % ((LCD_X - TREASURE_WIDTH) + 1 - 1) + 1;
				treasure_y = rand() % ((LCD_Y - TREASURE_HEIGHT) + 1 - 1) + 1;
				sheild_x = rand() % ((LCD_X - SHEILD_WIDTH) + 1 - 1) + 1;
				sheild_y = rand() % ((LCD_Y - SHEILD_HEIGHT) + 1 - 1) + 1;
				bomb_x = rand() % ((LCD_X - BOMB_WIDTH) + 1 - 1) + 1;
				bomb_y = rand() % ((LCD_Y - BOMB_HEIGHT) + 1 - 1) + 1;
				bow_x = rand() % ((LCD_X - BOW_WIDTH) + 1 - 1) + 1;
				bow_y = rand() % ((LCD_Y - BOW_WIDTH) + 1 - 1) + 1;
			} while (monster_x == door_x && monster_x == key_x && monster_x == treasure_x && monster_x == sheild_x && monster_x == bomb_x && monster_x == bow_x && monster_y == door_y && monster_y == key_y && monster_y == treasure_y && monster_y == sheild_y && monster_y == bomb_y && monster_y == bow_y && door_x == key_x && door_x == treasure_x && door_x == sheild_x && door_x == bomb_x && door_x == bow_x && door_y == key_y && door_y == treasure_y && door_y == sheild_y && door_y == bomb_y && door_y == bow_y && key_x == treasure_x && key_x == sheild_x && key_x == bomb_x && key_x == bow_x && key_y == treasure_y && key_y == sheild_y && key_y == bomb_y && key_y == bow_y && treasure_x == sheild_x && treasure_x == bomb_x && treasure_x == bow_x && treasure_y == sheild_y && treasure_y == bomb_y && treasure_y == bow_y && sheild_x == bomb_x && sheild_x == bow_x && sheild_y == bomb_y && sheild_y == bow_y && bomb_x == bow_x && bomb_y == bow_y);

			do {
				person_x = rand() % ((LCD_X - PERSON_WIDTH - 5) + 1 - 1) + 1;
				person_y = rand() % ((LCD_Y - PERSON_HEIGHT - 5) -1 + 1) + 1;
			} while(person_x == monster_x && person_x == door_x && person_x == key_x && person_x == treasure_x && person_x == sheild_x && person_x == bomb_x && person_x == bow_x && person_y == monster_y && person_y == door_y && person_y == key_y && person_y == treasure_y && person_y == sheild_y && person_y == bomb_y && person_y == bow_y);
			sheild_probability = rand() % 100;
			bomb_probability = rand() % 100;
			bow_probability = rand() % 100;

			sprite_init(&monster, monster_x, monster_y, MONSTER_WIDTH, MONSTER_HEIGHT, monster_bitmap);
			sprite_init(&door, door_x, door_y, DOOR_WIDTH, DOOR_HEIGHT, door_bitmap);
			sprite_init(&treasure, treasure_x, treasure_y, TREASURE_WIDTH, TREASURE_HEIGHT, treasure_bitmap);
			sprite_init(&person, person_x, person_y, PERSON_WIDTH, PERSON_HEIGHT, person_bitmap);
			sprite_init(&key, key_x, key_y, KEY_WIDTH, KEY_HEIGHT, key_bitmap);

			if (sheild_probability <= 30) sprite_init(&sheild, sheild_x, sheild_y, SHEILD_WIDTH, SHEILD_HEIGHT, sheild_bitmap);
			if (bomb_probability <= 30) sprite_init(&bomb, bomb_x, bomb_y, BOMB_WIDTH, BOMB_HEIGHT, bomb_bitmap);
			if (bow_probability <= 30) sprite_init(&bow, bow_x, bow_y, BOW_WIDTH, BOW_HEIGHT, bow_bitmap);
			sprite_init(&crosshair, XC, YC + 2, CROSSHAIR_WIDTH, CROSSHAIR_HEIGHT, crosshair_bitmap);
			start_game = true;

		} else {

			do {
				person_x = rand() % ((LCD_X - PERSON_WIDTH - 5) + 1 - 1) + 1;
				person_y = rand() % ((LCD_Y - PERSON_HEIGHT - 5) -1 + 1) + 1;
			} while(person_x == monster_x && person_x == door_x && person_x == key_x && person_x == treasure_x && person_x == sheild_x && person_x == bomb_x && person_x == bow_x && person_y == monster_y && person_y == door_y && person_y == key_y && person_y == treasure_y && person_y == sheild_y && person_y == bomb_y && person_y == bow_y);
			sprite_init(&person, person_x, person_y, PERSON_WIDTH, PERSON_HEIGHT, person_bitmap);
			sprite_init(&key, key_x, key_y, KEY_WIDTH, KEY_HEIGHT, key_bitmap);
		}
	} else if (level == 2) {
		door_x = LCD_X - DOOR_WIDTH - 5;
		door_y = 5;
		key_x = 5;
		key_y = 5;
		monster_x = LCD_X - MONSTER_WIDTH - 5;
		monster_y = CASTLE_HEIGHT + MONSTER_HEIGHT + 4;
		person_x = XC - (PERSON_WIDTH / 2);
		person_y = CASTLE_HEIGHT + (PERSON_HEIGHT / 2);
		sheild_x = 10;
		sheild_y = 20;
		bomb_x = 30;
		bomb_y = 20;
		bow_x = 50;
		bow_y = 20;
		sprite_init(&door, door_x, door_y, DOOR_WIDTH, DOOR_HEIGHT, door_bitmap);
		sprite_init(&key, key_x, key_y, KEY_WIDTH, KEY_HEIGHT, key_bitmap);
		sprite_init(&monster, monster_x, monster_y, MONSTER_WIDTH, MONSTER_HEIGHT, monster_bitmap);
		sprite_init(&person, person_x, person_y, PERSON_WIDTH, PERSON_HEIGHT, person_bitmap);
		sprite_init(&sheild, sheild_x, sheild_y, SHEILD_WIDTH, SHEILD_HEIGHT, sheild_bitmap);
		sprite_init(&bomb, bomb_x, bomb_y, BOMB_WIDTH, BOMB_HEIGHT, bomb_bitmap);
		sprite_init(&bow, bow_x, bow_y, BOW_WIDTH, BOW_HEIGHT, bow_bitmap);
		sprite_init(&crosshair, XC, YC + 2, CROSSHAIR_WIDTH, CROSSHAIR_HEIGHT, crosshair_bitmap);
	}
}

ISR(TIMER1_OVF_vect) {
	overflow_counter++;
	if (start_counter) {
		countdown--;

		if (timer < 60) {
			timer++;
		} else if (timer_2 < 60) {
			timer = 0;
			timer_2++;
		}
	}

	if (start_game) {
		double time = (overflow_counter * 65536.0 + TCNT1) * PRESCALE / FREQ;
		char time_text[30];
		sprintf(time_text, "Time: %f ", time);
		usb_serial_send(time_text);

		char score_text[30];
		sprintf(score_text, "Score: %i ", score);
		usb_serial_send(score_text);

		char level_text[30];
		sprintf(level_text, "Floor: %i ", level);
		usb_serial_send(level_text);

		char person_x_text[30];
		sprintf(person_x_text, "Player X: %f ", (double) person.x);
		usb_serial_send(person_x_text);

		char person_y_text[30];
		sprintf(person_y_text, "Player Y: %f ", (double) person.y);
		usb_serial_send(person_y_text);

		char lives_text[30];
		sprintf(lives_text, "Lives: %i \r\n", lives);
		usb_serial_send(lives_text);
	}
}

ISR(TIMER0_OVF_vect) {
	int bit_mask = 0b00000111;

  center = center << 1;
  center = center & bit_mask;
  center = center | BIT_VALUE(PINB, 0);

	if (center == bit_mask) {
		center_pressed = true;
	} else if (center == 0) {
		center_pressed = false;
	}

	up = up << 1;
	up = up & bit_mask;
	up = up | BIT_VALUE(PIND, 1);

	if (up == bit_mask) {
		up_pressed = true;
	} else {
		up_pressed = false;
	}

	down = down << 1;
	down = down & bit_mask;
	down = down | BIT_VALUE(PINB, 7);

	if (down == bit_mask) {
		down_pressed = true;
	} else {
		down_pressed = false;
	}

	left = left << 1;
	left = left & bit_mask;
	left = left | BIT_VALUE(PINB, 1);

	if (left == bit_mask) {
		left_pressed = true;
	} else {
		left_pressed = false;
	}

	right = right << 1;
	right = right & bit_mask;
	right = right | BIT_VALUE(PIND, 0);

	if (right == bit_mask) {
		right_pressed = true;
	} else {
		right_pressed = false;
	}
}

void draw_border(void) {
	draw_line(0, 0, LCD_X, 0, FG_COLOUR);
	draw_line(0, 0, 0, LCD_Y, FG_COLOUR);
	draw_line(0, LCD_Y - 1, LCD_X - 1, LCD_Y - 1, FG_COLOUR);
	draw_line(LCD_X - 1, 0, LCD_X - 1, LCD_Y, FG_COLOUR);
}

void draw_sprites(void) {

	if (level == 0) sprite_draw(&castle);

	if (level > 1) {
		sprite_draw(&sheild);
		sprite_draw(&bomb);
		sprite_draw(&bow);
	}

	if (level > 0) sprite_draw(&treasure);

	sprite_draw(&door);
	sprite_draw(&monster);
	sprite_draw(&person);
	sprite_draw(&key);

	if (has_bomb || has_bow) sprite_draw(&crosshair);
	if (fired) sprite_draw(&arrow);
}

void show_sprites(bool visibility) {
	person.is_visible = visibility;
	door.is_visible = visibility;
	key.is_visible = visibility;
	if (!defence && !exploded && !arrowed) monster.is_visible = visibility;
	if (level == 0) castle.is_visible = visibility;
	if (!defence) sheild.is_visible = visibility;
	if (!has_treasure) treasure.is_visible = visibility;
	if (!has_bomb && !exploded) bomb.is_visible = visibility;
	if (!has_bow) bow.is_visible = visibility;
}

void player_status(int prev_state) {
	clear_screen();
	show_sprites(true);

	if (center_pressed != prev_state) {
		prev_state = center_pressed;

		if (prev_state) {
			show_sprites(false);
			draw_string(15, 5, "Score:", FG_COLOUR);
			draw_int(45, 5, score, FG_COLOUR);
			draw_string(15, 15, "Lives:  ", FG_COLOUR);
			draw_int(45, 15, lives, FG_COLOUR);
			draw_string(15, 25, "Floor:", FG_COLOUR);
			draw_int(45, 25, level, FG_COLOUR);
			draw_string(15, 35, "Time :", FG_COLOUR);

			if (timer_2 < 10) {
				draw_int(45, 35, 0, FG_COLOUR);
				draw_int(50, 35, timer_2, FG_COLOUR);
			} else {
				draw_int(45, 35, timer_2, FG_COLOUR);
			}

			draw_char(55, 35, ':', FG_COLOUR);

			if (timer < 10) {
				draw_int(60, 35, 0, FG_COLOUR);
				draw_int(65, 35, timer, FG_COLOUR);
			} else {
				draw_int(60, 35, timer, FG_COLOUR);
			}
		}
	}
}

void player_movement(int prev_state) {

	if (up_pressed != prev_state) {
		prev_state = up_pressed;
		int castle_bottom = 2;
		if (level == 0) castle_bottom = YC + 2;

		if (prev_state && person.y > castle_bottom && !sprite_collided(person, door)) {

			if (has_key) {
				key.dy = PERSON_VELOCITY;
				key.y -= key.dy;
			}

			if (has_sheild) {
				sheild.dy = PERSON_VELOCITY;
				sheild.y -= sheild.dy;
			}

			if (has_bomb) {
				bomb.dy = PERSON_VELOCITY;
				bomb.y -= bomb.dy;
			}

			if (has_bow) {
				bow.dy = PERSON_VELOCITY;
				bow.y -= bow.dy;
			}
			person.dy = PERSON_VELOCITY;
			person.y -= person.dy;
		}

	} else if (down_pressed != prev_state && ((person.y + PERSON_HEIGHT) < (LCD_Y - 2)) && !sprite_collided(person, door)) {
		prev_state = down_pressed;

		if (prev_state) {

			if (has_key) {
				key.dy = PERSON_VELOCITY;
				key.y += key.dy;
			}

			if (has_sheild) {
				sheild.dy = PERSON_VELOCITY;
				sheild.y += sheild.dy;
			}

			if (has_bomb) {
				bomb.dy = PERSON_VELOCITY;
				bomb.y += bomb.dy;
			}

			if (has_bow) {
				bow.dy = PERSON_VELOCITY;
				bow.y += bow.dy;
			}
			person.dy = PERSON_VELOCITY;
			person.y += person.dy;
		}

	} else if (left_pressed != prev_state && person.x > 2 && !sprite_collided(person, door)) {
		prev_state = left_pressed;

		if (prev_state) {

			if (has_key) {
				key.dx = PERSON_VELOCITY;
				key.x -= key.dx;
			}

			if (has_sheild) {
				sheild.dx = PERSON_VELOCITY;
				sheild.x -= sheild.dx;
			}

			if (has_bomb) {
				bomb.dx = PERSON_VELOCITY;
				bomb.x -= bomb.dx;
			}

			if (has_bow) {
				bow.dx = PERSON_VELOCITY;
				bow.x -= bow.dx;
			}
			person.dx = PERSON_VELOCITY;
			person.x -= person.dx;
		}

	} else if (right_pressed != prev_state && ((person.x + PERSON_WIDTH) < (LCD_X - 2)) && !sprite_collided(person, door)) {
		prev_state = right_pressed;

		if (prev_state) {

			if (has_key) {
				key.dx = PERSON_VELOCITY;
				key.x += key.dx;
			}

			if (has_sheild) {
				sheild.dx = PERSON_VELOCITY;
				sheild.x += sheild.dx;
			}

			if (has_bomb) {
				bomb.dx = PERSON_VELOCITY;
				bomb.x += bomb.dx;
			}

			if (has_bow) {
				bow.dx = PERSON_VELOCITY;
				bow.x += bow.dx;
			}
			person.dx = PERSON_VELOCITY;
			person.x += person.dx;
		}
	} else if (sprite_collided(person, door) && !has_key && level > 0) {
		person.x -= 1;
		person.y -= 1;
	}
}

void follow_person(void) {
	double dx = person.x - monster.x;
	double dy = person.y - monster.y;
	double dist = sqrt(dx * dx + dy * dy);
	dx = (dx * MONSTER_VELOCITY) / dist;
	dy = (dy * MONSTER_VELOCITY) / dist;

	if (sprite_collided(monster, door)) {
		monster.x -= 1;
		monster.y -= 1;
	}

	monster.dx = dx;
	monster.dy = dy;
	monster.x += monster.dx;
	monster.y += monster.dy;
}

void arrow_fired(void) {
	double dx = crosshair.x - arrow.x;
	double dy = crosshair.y - arrow.y;
	double dist = sqrt(dx * dx + dy * dy);
	dx = (dx * (PERSON_VELOCITY * 2)) / dist;
	dy = (dy * (PERSON_VELOCITY * 2)) / dist;

	arrow.dx = dx;
	arrow.dy = dy;
	arrow.x += arrow.dx;
	arrow.y += arrow.dy;
}

// void monster_movement(void) {
//   int x = monster.x;
// 	int y = monster.y;
//   double dx = monster.dx;
//   double dy = monster.dy;
// 	int castle_bottom = 0;
// 	if (level == 0) castle_bottom = YC + 2;
//
//   if (x <= 0 || sprite_collided(monster, door)) {
//     dx = fabs(dx);
//   } else if (x >= (LCD_X - monster.width)) {
//     dx = -fabs(dx);
//   }
//
// 	if (y <= 0 || sprite_collided(monster, door)) {
// 		dy = fabs(dy);
// 	} else if (y >= (LCD_Y - monster.height - castle_bottom)) {
// 		dy = -fabs(dy);
// 	}
//
//   if (dx != monster.dx || dy != monster.dy) {
//     monster.x -= monster.dx;
// 		monster.y -= monster.dy;
//     monster.dx = dx;
// 		monster.dy = dy;
//   }
// }

void crosshair_movement(void) {
	int left_adc = adc_read(0);
	int right_adc = adc_read(1);
	int castle_bottom = 1;

	if (level == 0) castle_bottom = YC + 2;
	crosshair.x = (double) left_adc * (LCD_X - crosshair.width) / 1024;
	crosshair.y = (double) right_adc * (LCD_Y - crosshair.height) / 1024;
}

void process(void) {

  if ((BIT_IS_SET(PINF, 6) || BIT_IS_SET(PINF, 5))) {
		start_counter = true;
	}

	if (lives == 0) {
		clear_screen();
		draw_string(20, 5, "GAME OVER", FG_COLOUR);
		draw_string(20, 20, "Score:", FG_COLOUR);
		draw_int(50, 20, score, FG_COLOUR);
		draw_string(20, 35, "Floor:", FG_COLOUR);
		draw_int(50, 35, level, FG_COLOUR);
		start_game = false;

		if (BIT_IS_SET(PINF, 6) || BIT_IS_SET(PINF, 5)) {
			countdown = 3;
			score = 0;
			lives = 3;
			level = 0;
			timer = 0;
			timer_2 = 0;
			arrows = 5;
			start_counter = false;
			clear_screen();
	    draw_string(XC - 36, YC - 10, "Quintus Cardozo", FG_COLOUR);
	    draw_string(XC - 18, YC, "n9703578", FG_COLOUR);
			respawn = false;
			setup_game();
		}
	}

  if (!start_counter) {
		clear_screen();
    draw_string(XC - 36, YC - 10, "Quintus Cardozo", FG_COLOUR);
    draw_string(XC - 18, YC, "n9703578", FG_COLOUR);
  } else {
    if (countdown > 0) {
			clear_screen();
      draw_int(XC - 3, YC - 4, countdown, FG_COLOUR);

    } else if (countdown == 0) {
			clear_screen();
			start_game = true;
		}
	}

	if (start_game) {
		static int prev_state = 0;
		player_status(prev_state);
		player_movement(prev_state);

		if (!center_pressed) {
			CLEAR_BIT(PORTB, 2);
			CLEAR_BIT(PORTB, 3);
			draw_border();

			if (sprite_collided(person, key)) {
				has_key = true;
				key.x = person.x - KEY_WIDTH;
				key.y = person.y + (PERSON_HEIGHT / 2);

				if (send_key) {
					usb_serial_send("Player has pick up the key \r\n");
					send_key = false;
				}
			}

			if (sprite_collided(person, sheild) && sheild.is_visible) {
				has_sheild = true;
				sheild.x = person.x - SHEILD_WIDTH;
				sheild.y = person.y;

				if (send_sheild) {
					usb_serial_send("Player has picked up sheild \r\n");
					send_sheild = false;
				}

				if (has_bomb) {
					bomb.x = person.x + 20;
					has_bomb = false;
				}

				if (has_bow) {
					bow.x = person.x + 20;
					has_bow = false;
				}
			}

			if (sprite_collided(person, treasure) && treasure.is_visible) {
				score += 10;
				treasure.is_visible = false;
				has_treasure = true;

				if (send_treasure) {
					usb_serial_send("Player picked up treasure \r\n");
					send_treasure = false;
				}
			}

			if (sprite_collided(person, monster) && monster.is_visible) {

				if (has_key) {
					key_x = key.x;
					key_y = key.y;
					has_key = false;
				}

				if (has_sheild) {
					monster.is_visible = false;
					sheild.is_visible = false;
					defence = true;
					has_sheild = false;
					usb_serial_send("Player killed monster with sheild \r\n");
				} else {
					lives -= 1;
					respawn = true;
					setup_game();
				}

				usb_serial_send("Player has died\r\n");
			}

			if (sprite_collided(person, door) && has_key) {
				level += 1;
				score += 100;
				has_key = false;

				show_sprites(false);
				draw_string(20, YC - 10, "Floor:", FG_COLOUR);
				draw_int(50, YC - 10, level, FG_COLOUR);
				draw_string(20, YC, "Score:", FG_COLOUR);
				draw_int(50, YC, score, FG_COLOUR);
				show_screen();
				_delay_ms(2000);

				char next_level[40];
				sprintf(next_level, "Player has moved to %i floor \r\n", level);
				usb_serial_send(next_level);
				respawn = false;
				setup_game();
			}

			if (sprite_collided(person, bomb) && bomb.is_visible) {
				has_bomb = true;
				bomb.x = person.x - BOMB_WIDTH;
				bomb.y = person.y + (PERSON_HEIGHT / 2);

				if (send_bomb) {
					usb_serial_send("Player picked up bomb \r\n");
					send_bomb = false;
				}

				if (has_sheild) {
					sheild.x = person.x + 20;
					has_sheild = false;
				}

				if (has_bow) {
					bow.x = person.x + 20;
					has_bow = false;
				}
			}

			if (sprite_collided(person, bow) && bow.is_visible) {
				has_bow = true;
				bow.x = person.x - BOW_WIDTH;
				bow.y = person.y + (PERSON_HEIGHT / 2) - (BOW_HEIGHT / 2);

				if (send_bow) {
					usb_serial_send("Player picked up bow \r\n");
					send_bow = false;
				}

				if (has_sheild) {
					sheild.x = person.x + 20;
					has_sheild = false;
				}

				if (has_bomb) {
					bow.x = person.x + 20;
					has_bow = false;
				}
			}

			if ((BIT_IS_SET(PINF, 6) || BIT_IS_SET(PINF, 5))) {

				if (has_bomb) {
					if (person.x < crosshair.x && crosshair.x < LCD_X - CROSSHAIR_WIDTH) {
						bomb.x = person.x + PERSON_WIDTH + 5;
					} else if (person.x > crosshair.x && crosshair.x > 1) {
						bomb.x = person.x - PERSON_WIDTH - 5;
					}
					draw_sprites();
					show_screen();

					_delay_ms(2000);
					bomb.is_visible = false;
					monster.is_visible = false;
					exploded = true;
					has_bomb = false;
					score += 10;
					SET_BIT(PORTB, 2);
					SET_BIT(PORTB, 3);
				}

				if (has_bow) {
					sprite_init(&arrow, 35, 30, ARROW_WIDTH, ARROW_HEIGHT, arrow_bitmap);
					if (person.x < crosshair.x) {
						arrow.x = person.x + 10;
					} else if (person.x > crosshair.x) {
						arrow.x = person.x - 10;
					}
					arrows -= 1;
					fired = true;
				}
			}

			if (fired) {
				arrow_fired();

				if (arrows == 0) {
					bow.is_visible = false;
					arrow.is_visible = false;
					has_bow = false;
				}
				sprite_draw(&arrow);
			}

			if (sprite_collided(monster, arrow) && arrow.is_visible && monster.is_visible) {
				usb_serial_send("Player killed monster with bow \r\n");
				score += 10;
				arrow.is_visible = false;
				monster.is_visible = false;
				arrowed = true;
			}

			crosshair_movement();
			follow_person();
			// monster_movement();
			draw_sprites();
		}
	}
	// draw_int(10, 20, treasure.x, FG_COLOUR);
	// draw_int(10, 30, person.x, FG_COLOUR);
	show_screen();
}

int main(void) {
	setup();
	setup_game();

	for (;;) {
		process();
		_delay_ms(80);
	}

	return 0;
}
