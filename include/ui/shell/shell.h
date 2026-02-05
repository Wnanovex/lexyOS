#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include <stddef.h>

#define SHELL_BUFFER_SIZE 256
#define SHELL_HISTORY_SIZE 10

void shell_init(void);
void shell_handle_char(char c);
void shell_handle_special_key(uint8_t scancode);
void shell_print_prompt(void);
void shell_update(void);
void shell_draw(void);

#endif // SHELL_H
