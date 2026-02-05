#ifndef GAME_SNAKE_H
#define GAME_SNAKE_H

#include <stdint.h>

void snake_init(void);
void snake_update(void);
void snake_draw(void);
void snake_input(char c);
void snake_special_input(uint8_t scancode);
int snake_is_running(void);

#endif
