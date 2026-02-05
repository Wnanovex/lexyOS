#include <ui/terminal_games/game_snake/game_snake.h>
#include <ui/console.h>
#include <drivers/input/keyboard.h>
#include <ui/tty/tty.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/irq.h>

#define GRID_WIDTH 40
#define GRID_HEIGHT 20
#define MAX_SNAKE_LENGTH 800
#define SNAKE_UPDATE_INTERVAL 10

typedef struct {
    int x, y;
} point_t;

static point_t snake[MAX_SNAKE_LENGTH];
static int snake_length;
static point_t food;
static int dx, dy;
static uint32_t score;  
static int game_over;
static int game_running = 0;
static int initialized = 0;
static int needs_redraw = 1;
static int update_counter = 0;

// random number generation using timer
static void spawn_food(void) {
    uint32_t seed = get_timer_ticks();
    
    do {
        // Linear Congruential Generator for better randomness
        seed = seed * 1103515245 + 12345;
        food.x = (seed % (GRID_WIDTH - 2)) + 1;
        
        seed = seed * 1103515245 + 12345;
        food.y = (seed % (GRID_HEIGHT - 2)) + 1;
        
        // Check if food spawns on snake
        int on_snake = 0;
        for (int i = 0; i < snake_length; i++) {
            if (snake[i].x == food.x && snake[i].y == food.y) {
                on_snake = 1;
                break;
            }
        }
        
        if (!on_snake) {
            break;
        }
    } while (1);
}

static void draw_full_screen(void) {
    // Set cursor to top-left
    console_set_cursor_pos(0, 0);
    
    // Draw title
    console_set_color_preset(CONSOLE_COLOR_PRESET_MATRIX);
    console_write("╔════════════════════════════════════════╗\n");
    console_write("║            SNAKE GAME                  ║\n");
    console_write("╚════════════════════════════════════════╝\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    
    // Draw score
    console_write("Score: ");
    
    // Print score with proper digits
    if (score >= 10000) console_putchar('0' + (score / 10000));
    if (score >= 1000) console_putchar('0' + ((score / 1000) % 10));
    if (score >= 100) console_putchar('0' + ((score / 100) % 10));
    if (score >= 10) console_putchar('0' + ((score / 10) % 10));
    console_putchar('0' + (score % 10));
    
    console_write("   Press ESC to quit\n\n");
    
    // Draw top border
    console_write("╔");
    for (int x = 1; x < GRID_WIDTH - 1; x++) {
        console_write("═");
    }
    console_write("╗\n");
    
    // Draw game area
    for (int y = 1; y < GRID_HEIGHT - 1; y++) {
        console_write("║");
        
        for (int x = 1; x < GRID_WIDTH - 1; x++) {
            int is_snake = 0;
            int is_head = 0;
            
            // Check if this position is part of snake
            for (int i = 0; i < snake_length; i++) {
                if (snake[i].x == x && snake[i].y == y) {
                    is_snake = 1;
                    if (i == 0) is_head = 1;
                    break;
                }
            }
            
            // Check if this position is food
            if (food.x == x && food.y == y) {
                console_set_colors(255, 0, 0, 0, 0, 0);
                console_write("*");
                console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
            } else if (is_snake) {
                if (is_head) {
                    console_set_colors(0, 255, 0, 0, 0, 0);
                    console_write("@");
                } else {
                    console_set_colors(0, 200, 0, 0, 0, 0);
                    console_write("o");
                }
                console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
            } else {
                console_write(" ");
            }
        }
        
        console_write("║\n");
    }
    
    // Draw bottom border
    console_write("╚");
    for (int x = 1; x < GRID_WIDTH - 1; x++) {
        console_write("═");
    }
    console_write("╝\n");
    
    if (game_over) {
        console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
        console_write("\n    GAME OVER! Press R to restart or ESC to quit\n");
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    } else {
        console_write("\nControls: Arrow keys to move\n");
    }
}

void snake_init(void) {
    snake_length = 3;
    snake[0].x = GRID_WIDTH / 2;
    snake[0].y = GRID_HEIGHT / 2;
    snake[1].x = snake[0].x - 1;
    snake[1].y = snake[0].y;
    snake[2].x = snake[1].x - 1;
    snake[2].y = snake[1].y;
    
    dx = 1;
    dy = 0;
    score = 0;
    game_over = 0;
    game_running = 1;
    initialized = 0;
    needs_redraw = 1;
    update_counter = 0;
    
    spawn_food();
}

void snake_update(void) {
    if (!game_running) return;
    
    // Only update every SNAKE_UPDATE_INTERVAL calls to slow down the snake
    update_counter++;
    if (update_counter < SNAKE_UPDATE_INTERVAL) {
        return;
    }
    update_counter = 0;
    
    if (game_over) {
        needs_redraw = 1;
        return;
    }
    
    // Move snake
    point_t new_head;
    new_head.x = snake[0].x + dx;
    new_head.y = snake[0].y + dy;
    
    // Check collisions with walls
    if (new_head.x <= 0 || new_head.x >= GRID_WIDTH - 1 ||
        new_head.y <= 0 || new_head.y >= GRID_HEIGHT - 1) {
        game_over = 1;
        needs_redraw = 1;
        return;
    }
    
    // Check self collision (skip head)
    for (int i = 1; i < snake_length; i++) {
        if (snake[i].x == new_head.x && snake[i].y == new_head.y) {
            game_over = 1;
            needs_redraw = 1;
            return;
        }
    }
    
    // Move body (start from tail)
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = new_head;
    
    // Check food collision
    if (snake[0].x == food.x && snake[0].y == food.y) {
        score += 10;
        
        // Grow snake
        if (snake_length < MAX_SNAKE_LENGTH) {
            // Add new segment at tail position
            snake[snake_length] = snake[snake_length - 1];
            snake_length++;
        }
        
        spawn_food();
    }
    
    needs_redraw = 1;
}

void snake_draw(void) {
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

void snake_input(char c) {
    if (c == 27) {  // ESC
        game_running = 0;
        // Restore to shell
        tty_restore_to_shell();
    } else if (c == 'r' || c == 'R') {
        if (game_over) {
            snake_init();
        }
    }
}

void snake_special_input(uint8_t scancode) {
    if (game_over) return;
    
    switch (scancode) {
        case KEY_UP:
            if (dy != 1) {  // Can't reverse direction
                dx = 0;
                dy = -1;
            }
            break;
        case KEY_DOWN:
            if (dy != -1) {  // Can't reverse direction
                dx = 0;
                dy = 1;
            }
            break;
        case KEY_LEFT:
            if (dx != 1) {  // Can't reverse direction
                dx = -1;
                dy = 0;
            }
            break;
        case KEY_RIGHT:
            if (dx != -1) {  // Can't reverse direction
                dx = 1;
                dy = 0;
            }
            break;
    }
}

int snake_is_running(void) {
    return game_running;
}
