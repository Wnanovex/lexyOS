#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include <stdint.h>

void cmd_help(void);
void cmd_clear(void);
void cmd_about(void);
void cmd_lfetch(void);
void cmd_version(void);
void cmd_uptime(void);
void cmd_echo(const char* args);
void cmd_colors(void);
void cmd_history(int tty_num);
void cmd_cute_girl(void);
void cmd_miko(void);
void cmd_snake(void);
void cmd_tetris(void);
void cmd_meminfo(void);


void cmd_ls(const char* args);
void cmd_cat(const char* args);
void cmd_cd(const char* args);
void cmd_pwd(void);

#endif // SHELL_COMMANDS_H
