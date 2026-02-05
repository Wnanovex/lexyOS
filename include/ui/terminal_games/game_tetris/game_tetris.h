#ifndef GAME_TETRIS_H
#define GAME_TETRIS_H

#include <stdint.h>

void tetris_init(void);
void tetris_update(void);
void tetris_draw(void);
void tetris_input(char c);
void tetris_special_input(uint8_t scancode);
int tetris_is_running(void);

#endif
