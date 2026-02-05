#include <ui/shell/shell.h>
#include <ui/shell/shell_commands.h>
#include <ui/shell/shell_history.h>
#include <lib/string/string.h>
#include <ui/console.h>
#include <drivers/input/keyboard.h>
#include <ui/tty/tty.h>
#include <fs/vfs.h>

static void redraw_line_from_cursor(int tty_num) {
    tty_t* tty = &ttys[tty_num];
    console_save_cursor_pos();
    for (uint32_t i = tty->cursor_position; i < tty->buffer_index; i++) {
        console_putchar(tty->command_buffer[i]);
    }
    console_putchar(' ');
    console_restore_cursor_pos();
}

void shell_handle_special_key(uint8_t scancode) {
    int tty_num = tty_get_current();
    if (tty_num < 0 || tty_num >= MAX_TTYS) return;
    
    switch (scancode) {
        case KEY_UP:
        case KEY_DOWN:
            handle_history_navigation(tty_num, scancode);
            break;
        case KEY_LEFT:
            if (ttys[tty_num].cursor_position > 0) {
                ttys[tty_num].cursor_position--;
                console_move_cursor_left();
            }
            break;
        case KEY_RIGHT:
            if (ttys[tty_num].cursor_position < ttys[tty_num].buffer_index) {
                ttys[tty_num].cursor_position++;
                console_move_cursor_right();
            }
            break;
        case KEY_HOME:
            while (ttys[tty_num].cursor_position > 0) {
                ttys[tty_num].cursor_position--;
                console_move_cursor_left();
            }
            break;
        case KEY_END:
            while (ttys[tty_num].cursor_position < ttys[tty_num].buffer_index) {
                ttys[tty_num].cursor_position++;
                console_move_cursor_right();
            }
            break;
        case KEY_DELETE:
            if (ttys[tty_num].cursor_position < ttys[tty_num].buffer_index) {
                for (int i = ttys[tty_num].cursor_position; i < (int)(ttys[tty_num].buffer_index - 1); i++) {
                    ttys[tty_num].command_buffer[i] = ttys[tty_num].command_buffer[i + 1];
                }
                ttys[tty_num].buffer_index--;
                ttys[tty_num].command_buffer[ttys[tty_num].buffer_index] = '\0';
                redraw_line_from_cursor(tty_num);
            }
            break;
    }
}

void shell_handle_char(char c) {
    int tty_num = tty_get_current();
    if (tty_num < 0 || tty_num >= MAX_TTYS) return;
    tty_t* tty = &ttys[tty_num];
    
    // Handle Ctrl+C
    if (c == 3) {
        console_write("^C\n");
        tty->buffer_index = 0;
        tty->cursor_position = 0;
        tty->command_buffer[0] = '\0';
        history_index[tty_num] = -1;
        shell_print_prompt();
        return;
    }

    // Handle navigation shortcuts
    if (c == 1) {  // Ctrl+A
        while (tty->cursor_position > 0) {
            tty->cursor_position--;
            console_move_cursor_left();
        }
        return;
    }
    if (c == 5) {  // Ctrl+E
        while (tty->cursor_position < tty->buffer_index) {
            tty->cursor_position++;
            console_move_cursor_right();
        }
        return;
    }
    if (c == 11) {  // Ctrl+K
        while (tty->cursor_position < tty->buffer_index) {
            console_delete_char_at_cursor();
            console_putchar(' ');
            tty->buffer_index--;
        }
        tty->command_buffer[tty->cursor_position] = '\0';
        return;
    }
    if (c == 21) {  // Ctrl+U
        while (tty->cursor_position > 0) {
            tty->cursor_position--;
            console_move_cursor_left();
        }
        while (tty->buffer_index > 0) {
            console_delete_char_at_cursor();
            console_putchar(' ');
            tty->buffer_index--;
        }
        tty->command_buffer[0] = '\0';
        tty->cursor_position = 0;
        return;
    }

    // Handle Tab
    if (c == '\t') {
        handle_tab_completion(tty_num);
        return;
    }

    // Handle backspace
    if (c == '\b' || c == 127) {
        if (tty->cursor_position > 0) {
            for (int i = tty->cursor_position - 1; i < (int)(tty->buffer_index - 1); i++) {
                tty->command_buffer[i] = tty->command_buffer[i + 1];
            }
            tty->buffer_index--;
            tty->cursor_position--;
            tty->command_buffer[tty->buffer_index] = '\0';
            console_move_cursor_left();
            redraw_line_from_cursor(tty_num);
        }
        return;
    }

    // Handle enter
    if (c == '\n' || c == '\r') {
        tty->command_buffer[tty->buffer_index] = '\0';
        console_write("\n");
    
        // Save the FULL command line to history BEFORE parsing
        char* full_cmd = tty->command_buffer;
        while (*full_cmd == ' ') full_cmd++;  // Skip leading spaces
    
        if (*full_cmd) {
            add_to_history(tty_num, full_cmd);  // Save full line including args
        }
    
    // Now parse command and args
        char* cmd = tty->command_buffer;
        while (*cmd == ' ') cmd++;
    
        char* args = cmd;
        while (*args && *args != ' ') args++;
        if (*args) {
            *args = '\0';
            args++;
            while (*args == ' ') args++;
        }
  

        if (*cmd) {
             
            // Command execution moved to shell_commands.c
            if (strcmp(cmd, "help") == 0) cmd_help();
            else if (strcmp(cmd, "clear") == 0) cmd_clear();
            else if (strcmp(cmd, "about") == 0) cmd_about();
            else if (strcmp(cmd, "lfetch") == 0) cmd_lfetch();
            else if (strcmp(cmd, "version") == 0) cmd_version();
            else if (strcmp(cmd, "uptime") == 0) cmd_uptime();
            else if (strcmp(cmd, "echo") == 0) cmd_echo(args);
            else if (strcmp(cmd, "colors") == 0) cmd_colors();
            else if (strcmp(cmd, "history") == 0) cmd_history(tty_num);
            else if (strcmp(cmd, "cute-girl") == 0) cmd_cute_girl();
            else if (strcmp(cmd, "miko") == 0) cmd_miko();
            else if (strcmp(cmd, "snake") == 0) cmd_snake();
            else if (strcmp(cmd, "tetris") == 0) cmd_tetris();
            else if (strcmp(cmd, "meminfo") == 0) cmd_meminfo();
            else if (strcmp(cmd, "ls") == 0) cmd_ls(args);
            else if (strcmp(cmd, "cat") == 0) cmd_cat(args);
            else if (strcmp(cmd, "cd") == 0) cmd_cd(args);
            else if (strcmp(cmd, "pwd") == 0) cmd_pwd();
            else {
                console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
                console_write("\nUnknown command: '");
                console_write(cmd);
                console_write("'\nType 'help' for available commands.\n");
                console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
            }
        }
        
        tty->buffer_index = 0;
        tty->cursor_position = 0;
        tty->command_buffer[0] = '\0';
        history_index[tty_num] = -1;
        shell_print_prompt(); 
        return;
    }

    // Handle printable characters
    if (c >= 32 && c <= 126 && tty->buffer_index < SHELL_BUFFER_SIZE - 1) {
        if (tty->cursor_position > tty->buffer_index)
            tty->cursor_position = tty->buffer_index;
        
        for (uint32_t i = tty->buffer_index; i > tty->cursor_position; i--) {
            tty->command_buffer[i] = tty->command_buffer[i - 1];
        }
        
        tty->command_buffer[tty->cursor_position] = c;
        tty->buffer_index++;
        tty->cursor_position++;
        tty->command_buffer[tty->buffer_index] = '\0';
        
        console_save_cursor_pos();
        for (uint32_t i = tty->cursor_position - 1; i < tty->buffer_index; i++) {
            console_putchar(tty->command_buffer[i]);
        }
        console_restore_cursor_pos();
        console_move_cursor_right();
        
        history_index[tty_num] = -1;
    }
}

void shell_print_prompt(void) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_MATRIX);
    console_write("root");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    console_write("@");
    console_set_colors(100, 100, 255, 0, 0, 0);
    console_write("lexyOS");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    console_write(":$ ");
}

void shell_update(void) {}
void shell_draw(void) {}

void shell_init(void) {
    init_shell_history();  // Initialize history arrays
    
    console_clear();
    console_set_color_preset(CONSOLE_COLOR_PRESET_MATRIX);
    console_write("\n  ╔═══════════════════════════════════════╗\n");
    console_write("  ║                                       ║\n");
    console_write("  ║           Welcome to lexyOS           ║\n");
    console_write("  ║              Version 0.1.0            ║\n");
    console_write("  ║                                       ║\n");
    console_write("  ╚═══════════════════════════════════════╝\n\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    
    console_write("Type 'help' for available commands.\n");
    console_write("Use TAB for completion, UP/DOWN for history.\n");
    console_write("Press Alt+F1 to Alt+F6 to switch TTYs.\n\n");
    
    // Register TTYs 0-5 as shell (keeping your original limit)
    for (int i = 0; i < 6; i++) {
        tty_register(i, TTY_MODE_SHELL, 
                     shell_update,
                     shell_draw,
                     shell_handle_char,
                     shell_handle_special_key);
    }
    
    shell_print_prompt();
}
