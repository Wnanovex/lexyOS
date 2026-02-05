#ifndef SHELL_HISTORY_H
#define SHELL_HISTORY_H

#include <ui/tty/tty.h>
#include <fs/vfs.h>  

#define SHELL_BUFFER_SIZE 256
#define SHELL_HISTORY_SIZE 10

void init_shell_history(void);
void add_to_history(int tty_num, const char* cmd);
void handle_history_navigation(int tty_num, uint8_t scancode);
void handle_tab_completion(int tty_num);

// Global history arrays
extern char command_history[MAX_TTYS][SHELL_HISTORY_SIZE][SHELL_BUFFER_SIZE];
extern int history_count[MAX_TTYS];
extern int history_index[MAX_TTYS];
extern char temp_buffer[MAX_TTYS][SHELL_BUFFER_SIZE];

#endif // SHELL_HISTORY_H
