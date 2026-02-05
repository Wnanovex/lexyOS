#include <ui/shell/shell_history.h>
#include <ui/shell/shell.h>
#include <lib/string/string.h>
#include <ui/console.h>
#include <drivers/input/keyboard.h>
#include <ui/tty/tty.h>


static int starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// Global history arrays (using REAL MAX_TTYS from tty.h)
char command_history[MAX_TTYS][SHELL_HISTORY_SIZE][SHELL_BUFFER_SIZE];
int history_count[MAX_TTYS] = {0};
int history_index[MAX_TTYS] = {-1};
char temp_buffer[MAX_TTYS][SHELL_BUFFER_SIZE];

// Available commands for tab completion
static const char* available_commands[] = {
    "help", "clear", "about", "lfetch", "version", "uptime", "echo", "colors",
    "cute-girl", "history", "reboot", "miko", "snake", "tetris", "meminfo", NULL
};

void init_shell_history(void) {
    for (int i = 0; i < MAX_TTYS; i++) {
        history_count[i] = 0;
        history_index[i] = -1;
        for (int j = 0; j < SHELL_HISTORY_SIZE; j++) {
            command_history[i][j][0] = '\0';
        }
        temp_buffer[i][0] = '\0';
    }
}

void add_to_history(int tty_num, const char* cmd) {
    if (tty_num < 0 || tty_num >= MAX_TTYS) return;
    if (!cmd || cmd[0] == '\0') return;
    
    if (history_count[tty_num] > 0 && strncmp(command_history[tty_num][0], cmd, SHELL_BUFFER_SIZE) == 0) {
        return;
    }
    
    // Shift history down
    for (int i = SHELL_HISTORY_SIZE - 1; i > 0; i--) {
        strcpy(command_history[tty_num][i], command_history[tty_num][i-1]);
    }
    
    strcpy(command_history[tty_num][0], cmd);
    
    if (history_count[tty_num] < SHELL_HISTORY_SIZE) {
        history_count[tty_num]++;
    }
}

void handle_history_navigation(int tty_num, uint8_t scancode) {
    tty_t* tty = &ttys[tty_num];
    
    if (scancode == KEY_UP) {
        if (history_count[tty_num] == 0) return;
        
        if (history_index[tty_num] == -1) {
            strcpy(temp_buffer[tty_num], tty->command_buffer);
            history_index[tty_num] = 0;
        } else if (history_index[tty_num] < history_count[tty_num] - 1) {
            history_index[tty_num]++;
        } else {
            return;
        }
    } 
    else if (scancode == KEY_DOWN) {
        if (history_index[tty_num] == -1) return;
        
        if (history_index[tty_num] == 0) {
            strcpy(tty->command_buffer, temp_buffer[tty_num]);
            history_index[tty_num] = -1;
        } else {
            history_index[tty_num]--;
            strcpy(tty->command_buffer, command_history[tty_num][history_index[tty_num]]);
        }
    }
    
    // Clear current line
    while (tty->buffer_index > 0) {
        console_backspace();
        tty->buffer_index--;
    }
    
    if (scancode == KEY_UP && history_index[tty_num] >= 0) {
        strcpy(tty->command_buffer, command_history[tty_num][history_index[tty_num]]);
    }
    
    tty->buffer_index = strlen(tty->command_buffer);
    tty->cursor_position = tty->buffer_index;
    console_write(tty->command_buffer);
}


// Helper: Complete command names
static void complete_command(int tty_num, const char* partial) {
    tty_t* tty = &ttys[tty_num];
    
    int match_count = 0;
    const char* first_match = NULL;
    
    for (int i = 0; available_commands[i] != NULL; i++) {
        if (starts_with(available_commands[i], partial)) {
            match_count++;
            if (first_match == NULL) {
                first_match = available_commands[i];
            }
        }
    }
    
    if (match_count == 0) return;
    
    if (match_count == 1) {
        // Auto-complete
        while (tty->buffer_index > 0) {
            console_backspace();
            tty->buffer_index--;
        }
        strcpy(tty->command_buffer, first_match);
        tty->buffer_index = strlen(first_match);
        tty->cursor_position = tty->buffer_index;
        console_write(tty->command_buffer);
    } else {
        // Show matches
        console_write("\n");
        console_set_colors(100, 100, 255, 0, 0, 0);
        for (int i = 0; available_commands[i] != NULL; i++) {
            if (starts_with(available_commands[i], partial)) {
                console_write("  ");
                console_write(available_commands[i]);
                console_write("\n");
            }
        }
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
        shell_print_prompt();
        console_write(tty->command_buffer);
    }
}

// Helper: Complete filesystem paths
static void complete_filesystem_path(int tty_num, const char* partial) {
    tty_t* tty = &ttys[tty_num];
    
    // Get current directory
    vfs_node_t* dir = vfs_get_cwd();
    if (!dir) return;
    
    // Count matches
    int match_count = 0;
    vfs_node_t* first_match = NULL;
    char matches[32][256];  // Store up to 32 matches
    int match_idx = 0;
    
    uint32_t index = 0;
    vfs_node_t* node;
    
    while ((node = vfs_readdir(dir, index++)) != NULL && match_idx < 32) {
        if (starts_with(node->name, partial)) {
            match_count++;
            if (first_match == NULL) {
                first_match = node;
            }
            strcpy(matches[match_idx++], node->name);
        }
    }
    
    if (match_count == 0) return;
    
    if (match_count == 1) {
        // Auto-complete - replace the argument part
        // First, find where the command ends
        int cmd_end = 0;
        while (cmd_end < (int)tty->buffer_index && tty->command_buffer[cmd_end] != ' ') {
            cmd_end++;
        }
        
        // Skip spaces after command
        while (cmd_end < (int)tty->buffer_index && tty->command_buffer[cmd_end] == ' ') {
            cmd_end++;
        }
        
        // Clear from cmd_end to end of line
        while (tty->buffer_index > (uint32_t)cmd_end) {
            console_backspace();
            tty->buffer_index--;
        }
        
        // Add the completed filename
        const char* completion = first_match->name;
        for (int i = 0; completion[i] != '\0'; i++) {
            tty->command_buffer[tty->buffer_index++] = completion[i];
            console_putchar(completion[i]);
        }
        
        // Add trailing slash for directories
        if (first_match->type == VFS_DIRECTORY) {
            tty->command_buffer[tty->buffer_index++] = '/';
            console_putchar('/');
        }
        
        tty->command_buffer[tty->buffer_index] = '\0';
        tty->cursor_position = tty->buffer_index;
        
    } else {
        // Show matches
        console_write("\n");
        console_set_colors(100, 100, 255, 0, 0, 0);
        
        for (int i = 0; i < match_idx; i++) {
            // Find the node to check if it's a directory
            vfs_node_t* match_node = vfs_finddir(dir, matches[i]);
            
            if (match_node) {
                if (match_node->type == VFS_DIRECTORY) {
                    console_set_colors(100, 255, 255, 0, 0, 0);  // Cyan for dirs
                    console_write("  ");
                    console_write(matches[i]);
                    console_write("/\n");
                } else {
                    console_set_colors(200, 200, 200, 0, 0, 0);  // White for files
                    console_write("  ");
                    console_write(matches[i]);
                    console_write("\n");
                }
            }
        }
        
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
        shell_print_prompt();
        console_write(tty->command_buffer);
    }
}

void handle_tab_completion(int tty_num) {
    if (tty_num < 0 || tty_num >= MAX_TTYS) return;
    tty_t* tty = &ttys[tty_num];
    
    if (tty->buffer_index == 0) return;
    
    // Parse input to find command and argument
    char cmd[256] = {0};
    char arg[256] = {0};
    int cmd_len = 0;
    int arg_len = 0;
    int parsing_arg = 0;
    
    // Extract command and argument
    for (uint32_t i = 0; i < tty->buffer_index; i++) {
        if (tty->command_buffer[i] == ' ') {
            if (cmd_len > 0) {
                parsing_arg = 1;
            }
        } else {
            if (!parsing_arg) {
                if (cmd_len < 255) cmd[cmd_len++] = tty->command_buffer[i];
            } else {
                if (arg_len < 255) arg[arg_len++] = tty->command_buffer[i];
            }
        }
    }
    cmd[cmd_len] = '\0';
    arg[arg_len] = '\0';
    
    // Check if we should complete files (for file-based commands)
    int complete_files = 0;
    if (cmd_len > 0 && parsing_arg) {
        const char* file_commands[] = {"ls", "cat", "cd", NULL};
        for (int i = 0; file_commands[i] != NULL; i++) {
            if (strcmp(cmd, file_commands[i]) == 0) {
                complete_files = 1;
                break;
            }
        }
    }
    
    if (complete_files) {
        // File/directory completion
        complete_filesystem_path(tty_num, arg);
    } else {
        // Command completion (existing behavior)
        complete_command(tty_num, tty->command_buffer);
    }
}
