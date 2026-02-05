#include <ui/terminal_games/game_tetris/game_tetris.h>
#include <ui/console.h>
#include <drivers/input/keyboard.h>
#include <arch/x86_64/irq.h>
#include <ui/tty/tty.h>

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define TETRIS_UPDATE_INTERVAL 20

static uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];
static uint32_t score;  
static int game_over;
static int game_running = 0;
static int initialized = 0;
static int needs_redraw = 1;
static int update_counter = 0;

// Tetromino data
static int current_piece_x;
static int current_piece_y;
static int current_piece_type;

// Check if position is valid for current piece
static int is_valid_position(int x, int y) {
    if (x < 0 || x >= BOARD_WIDTH) {
        return 0;
    }
    if (y < 0 || y >= BOARD_HEIGHT) {
        return 0;
    }
    if (board[y][x]) {
        return 0;
    }
    return 1;
}

// Lock current piece to board
static void lock_piece(void) {
    if (current_piece_x >= 0 && current_piece_x < BOARD_WIDTH &&
        current_piece_y >= 0 && current_piece_y < BOARD_HEIGHT) {
        board[current_piece_y][current_piece_x] = current_piece_type;
    }
}

// Check and clear completed lines
static void check_lines(void) {
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        int line_full = 1;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (!board[y][x]) {
                line_full = 0;
                break;
            }
        }
        
        if (line_full) {
            // Move lines down
            for (int ny = y; ny > 0; ny--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    board[ny][x] = board[ny - 1][x];
                }
            }
            // Clear top line
            for (int x = 0; x < BOARD_WIDTH; x++) {
                board[0][x] = 0;
            }
            score += 100;
            y++; // Check this line again
            needs_redraw = 1;
        }
    }
}

// Spawn new piece
static void spawn_new_piece(void) {
    current_piece_x = BOARD_WIDTH / 2;
    current_piece_y = 0;
    current_piece_type = 1;
    
    // Check if game over (piece can't be placed)
    if (!is_valid_position(current_piece_x, current_piece_y)) {
        game_over = 1;
    }
    needs_redraw = 1;
}

static void draw_full_screen(void) {
    // Set cursor to top-left
    console_set_cursor_pos(0, 0);
    
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("╔════════════════════════════════╗\n");
    console_write("║         TETRIS GAME            ║\n");
    console_write("╚════════════════════════════════╝\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    
    console_write("Score: ");
    
    // Print score with proper digits
    if (score >= 10000) console_putchar('0' + (score / 10000));
    if (score >= 1000) console_putchar('0' + ((score / 1000) % 10));
    if (score >= 100) console_putchar('0' + ((score / 100) % 10));
    if (score >= 10) console_putchar('0' + ((score / 10) % 10));
    console_putchar('0' + (score % 10));
    
    console_write("\n\n");
    
    // Draw board
    console_write("╔");
    for (int x = 0; x < BOARD_WIDTH; x++) {
        console_write("══");
    }
    console_write("╗\n");
    
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        console_write("║");
        for (int x = 0; x < BOARD_WIDTH; x++) {
            // Check if current piece is here
            if (x == current_piece_x && y == current_piece_y && !game_over) {
                console_set_colors(255, 255, 0, 0, 0, 0);
                console_write("██");
                console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
            } else if (board[y][x]) {
                console_set_colors(0, 255, 255, 0, 0, 0);
                console_write("██");
                console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
            } else {
                console_write("  ");
            }
        }
        console_write("║\n");
    }
    
    console_write("╚");
    for (int x = 0; x < BOARD_WIDTH; x++) {
        console_write("══");
    }
    console_write("╝\n");
    
    console_write("\nControls: Arrows/ASD=Move, Space=Drop, ESC=Quit\n");
    
    if (game_over) {
        console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
        console_write("\nGAME OVER! Press R to restart or ESC to quit\n");
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    } else {
        console_write("\n");
    }
}

void tetris_init(void) {
    // Clear board
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            board[y][x] = 0;
        }
    }
    
    score = 0;
    game_over = 0;
    game_running = 1;
    initialized = 0;
    needs_redraw = 1;
    update_counter = 0;
    
    spawn_new_piece();
}

void tetris_update(void) {
    if (!game_running) return;
    
    // Only update every TETRIS_UPDATE_INTERVAL calls to slow down
    update_counter++;
    if (update_counter < TETRIS_UPDATE_INTERVAL) {
        return;
    }
    update_counter = 0;
    
    if (game_over) {
        needs_redraw = 1;
        return;
    }
    
    // Try to move piece down
    if (is_valid_position(current_piece_x, current_piece_y + 1)) {
        current_piece_y++;
        needs_redraw = 1;
    } else {
        // Can't move down - lock piece
        lock_piece();
        check_lines();
        spawn_new_piece();
    }
}

void tetris_draw(void) {
    if (!initialized) {
        console_clear();
        console_show_cursor(0);
        initialized = 1;
        needs_redraw = 1;
    }
    
    if (needs_redraw) {
        draw_full_screen();
        needs_redraw = 0;
    }
}

void tetris_input(char c) {
    if (c == 27) {  // ESC
        game_running = 0;
        // Restore to shell
        tty_restore_to_shell();
    } else if (c == ' ') {  // Space - hard drop
        if (!game_over) {
            // Drop piece to bottom
            while (is_valid_position(current_piece_x, current_piece_y + 1)) {
                current_piece_y++;
            }
            // Lock it immediately
            lock_piece();
            check_lines();
            score += 20;  // Bonus for hard drop
            spawn_new_piece();
            needs_redraw = 1;
        }
    } else if (c == 'r' || c == 'R') {
        if (game_over) {
            tetris_init();
        }
    } else if (c == 'a' || c == 'A') {  // Left (WASD backup)
        if (!game_over && is_valid_position(current_piece_x - 1, current_piece_y)) {
            current_piece_x--;
            needs_redraw = 1;
        }
    } else if (c == 'd' || c == 'D') {  // Right (WASD backup)
        if (!game_over && is_valid_position(current_piece_x + 1, current_piece_y)) {
            current_piece_x++;
            needs_redraw = 1;
        }
    } else if (c == 's' || c == 'S') {  // Down (WASD backup)
        if (!game_over && is_valid_position(current_piece_x, current_piece_y + 1)) {
            current_piece_y++;
            score += 1;
            needs_redraw = 1;
        }
    }
}

void tetris_special_input(uint8_t scancode) {
    if (game_over) return;
    
    // Use raw hex values for arrow keys
    switch (scancode) {
        case 0x4B:  // Left arrow
            if (is_valid_position(current_piece_x - 1, current_piece_y)) {
                current_piece_x--;
                needs_redraw = 1;
            }
            break;
        case 0x4D:  // Right arrow
            if (is_valid_position(current_piece_x + 1, current_piece_y)) {
                current_piece_x++;
                needs_redraw = 1;
            }
            break;
        case 0x50:  // Down arrow
            if (is_valid_position(current_piece_x, current_piece_y + 1)) {
                current_piece_y++;
                score += 1;
                needs_redraw = 1;
            }
            break;
    }
}

int tetris_is_running(void) {
    return game_running;
}
