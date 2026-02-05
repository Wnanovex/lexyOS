#include <ui/shell/shell_commands.h>
#include <ui/shell/shell_history.h>
#include <ui/console.h>
#include <arch/x86_64/idt.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <ui/terminal_games/game_snake/game_snake.h>
#include <ui/terminal_games/game_tetris/game_tetris.h>
#include <ui/tty/tty.h>
#include <fs/vfs.h>

void cmd_help(void) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN); 
    console_write("\n╔════════════════ Available Commands ════════════════╗\n");
    console_set_colors(200, 200, 0, 0, 0, 0);
    console_write("  help       - Show this help message\n");
    console_write("  clear      - Clear the screen\n");
    console_write("  about      - About lexyOS\n");
    console_write("  lfetch     - Info lexyOS\n");
    console_write("  version    - Show OS version\n");
    console_write("  uptime     - Show system uptime\n");
    console_write("  echo       - Echo text\n");
    console_write("  colors     - Show available colors\n");
    console_write("  cute-girl  - Display ASCII art\n");
    console_write("  history    - Show command history\n");
    console_write("  reboot     - Reboot system\n");
    console_write("  miko       - Display special art\n");
    console_write("  snake      - Play Snake game\n");
    console_write("  tetris     - Play Tetris game\n");
    console_write("  meminfo    - Show memory information\n");
    console_write("\nTip: Use TAB for command completion\n");
    console_write("     Use UP/DOWN arrows for command history\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("╚════════════════════════════════════════════════════╝\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);  
}

void cmd_clear(void) {
    console_clear();
}

void cmd_about(void) {
    console_set_colors(0, 150, 255, 0, 0, 0);
    console_write("\n╔══════════════════════════════╗\n");
    console_write("║        lexyOS v0.1.0         ║\n");
    console_write("╚══════════════════════════════╝\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    console_write("A simple 64-bit operating system\n");
    console_write("Written in C and Assembly\n");
    console_write("Features: Framebuffer, IDT, Keyboard, Shell\n");
    console_write("          Multi-TTY, Tab Completion, Command History\n");
}

void cmd_lfetch(void) {
    console_set_colors(0, 150, 255, 0, 0, 0);
    console_write("\n");
    console_write(" /$$                                /$$$$$$   /$$$$$$ \n"); 
    console_write("| $$                               /$$__  $$ /$$__  $$\n");
    console_write("| $$  /$$$$$$  /$$   /$$ /$$   /$$| $$  \\ $$| $$  \\__/\n");
    console_write("| $$ /$$__  $$|  $$ /$$/| $$  | $$| $$  | $$|  $$$$$$ \n");
    console_write("| $$| $$$$$$$$ \\  $$$$/ | $$  | $$| $$  | $$ \\____  $$\n");
    console_write("| $$| $$_____/  >$$  $$ | $$  | $$| $$  | $$ /$$  \\ $$\n");
    console_write("| $$|  $$$$$$$ /$$/\\  $$|  $$$$$$$|  $$$$$$/|  $$$$$$/\n");
    console_write("|__/ \\_______/|__/  \\__/ \\____  $$ \\______/  \\______/ \n");
    console_write("                         /$$  | $$                    \n");
    console_write("                        |  $$$$$$/                    \n");
    console_write("                         \\______/                     \n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    console_write("\n");
    console_write("lexyOS v0.1.0 (x86_64)\n");
    console_write("Simple operating system\n");
}

void cmd_version(void) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_MATRIX);
    console_write("\nlexyOS v0.1.0\n");
    console_write("Kernel: x86-64 long mode\n");
    console_write("Build: December 2025\n");
    console_write("Shell: Advanced with tab completion\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

void cmd_uptime(void) {
    uint32_t ticks = get_timer_ticks();
    uint32_t seconds = ticks / 18;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    console_write("\nUptime: ");
    
    if (hours > 0) {
        console_putchar('0' + (hours / 10));
        console_putchar('0' + (hours % 10));
        console_write("h ");
    }
    
    console_putchar('0' + ((minutes % 60) / 10));
    console_putchar('0' + ((minutes % 60) % 10));
    console_write("m ");
    
    console_putchar('0' + ((seconds % 60) / 10));
    console_putchar('0' + ((seconds % 60) % 10));
    console_write("s\n");
}

void cmd_echo(const char* args) {
    console_write("\n");
    if (*args) {
        console_write(args);
    }
    console_write("\n");
}

void cmd_colors(void) {
    console_write("\n╔══════════════ Color Demo ══════════════╗\n");
    
    console_set_colors(255, 0, 0, 0, 0, 0);
    console_write("  ■ Red Text\n");
    
    console_set_colors(0, 255, 0, 0, 0, 0);
    console_write("  ■ Green Text\n");
    
    console_set_colors(0, 0, 255, 0, 0, 0);
    console_write("  ■ Blue Text\n");
    
    console_set_colors(255, 255, 0, 0, 0, 0);
    console_write("  ■ Yellow Text\n");
    
    console_set_colors(255, 0, 255, 0, 0, 0);
    console_write("  ■ Magenta Text\n");
    
    console_set_colors(0, 255, 255, 0, 0, 0);
    console_write("  ■ Cyan Text\n");
    
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    console_write("╚════════════════════════════════════════╝\n");
}

void cmd_history(int tty_num) {
    console_write("\nCommand History:\n");
    
    if (history_count[tty_num] == 0) {
        console_write("  (empty)\n");
        return;
    }
    
    for (int i = 0; i < history_count[tty_num]; i++) {
        console_write("  ");
        console_putchar('0' + ((i + 1) / 10));
        console_putchar('0' + ((i + 1) % 10));
        console_write(". ");
        console_write(command_history[tty_num][i]);
        console_write("\n");
    }
}

void cmd_cute_girl(void) {
    console_set_scale(1);
    console_set_color_preset(CONSOLE_COLOR_PRESET_AMBER);
    console_write("\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⡤⣖⣤⣶⣿⣿⣿⣿⣿⣭⡶⠶⠒⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠈⠉⠁⠒⠤⠀⡔⠄⠀⠂\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣤⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⡴⢋⣥⣾⣿⣿⣿⣿⣿⣿⡿⠛⠁⢀⣠⠔⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀⠛⠓⢤⡀⠀⠀⢀⣀⠈⠂⠉⠀\n");
    console_write("⠀⠀⠀⢀⠀⠀⠀⠀⠀⠀⠘⣿⣀⢻⡟⠉⣷⠀⠀⠀⠀⠀⠀⢀⡤⠎⠁⣀⣿⣿⣿⣿⣿⣿⣿⠟⢉⣠⣶⡾⠋⠀⠀⠀⠀⣀⣠⡤⢶⡾⠶⠀⠀⠀⠀⠀⠉⠶⣽⣾⣿⣷⣄⡀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣶⣬⣺⡟⠀⠀⠀⠀⢀⡴⠋⢀⣤⣾⣿⣿⣿⣿⣿⣿⡿⣣⣴⡿⠟⠁⠀⠀⣠⣤⣶⡿⠟⢉⣴⠋⠁⠀⠀⢀⡞⠀⠀⣦⠀⠙⢿⣿⣿⣿⣿⣦\n");
    console_write("⠀⠀⠀⠀⠀⠰⡄⠀⠀⠀⠀⠀⠀⠀⠈⠁⠀⠀⠀⢀⣴⣯⣶⣿⣿⣿⡿⣽⣿⣿⣿⣿⣿⣿⢿⠋⡀⣀⣤⣾⠿⣿⡿⢋⣴⣾⠟⠀⠀⠀⠀⣴⠋⠀⠀⠀⢹⣦⠠⢪⣻⣿⣿⣿⣿\n");
    console_write("⠀⠀⠀⠀⠀⠀⠻⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣾⣿⣿⣿⣿⣻⢋⡞⣿⣿⣿⣿⣿⣿⣿⣷⣾⡿⠟⠉⣱⡾⣯⡶⣿⡿⠃⠀⠀⠀⢀⡼⠃⢠⣆⠀⠀⣇⣿⡆⡜⡏⢿⣿⣿⣿\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⢷⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣿⣿⣿⣿⡟⠁⠀⠁⣰⣿⣿⣿⣿⣿⣿⡿⠛⠁⠀⢠⣾⣿⠟⢡⣾⣿⠅⠀⠀⠀⣠⡿⠁⣰⣿⢸⢸⡆⣿⣿⣿⣼⣼⡈⣿⣿⣿\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠈⡇⠀⠀⠀⠀⠀⠀⠀⣴⣿⠟⣹⣿⡏⠀⠀⠀⢰⣿⣿⣿⣿⡿⠟⠉⠀⠀⠀⣰⣿⡿⠃⢠⣾⣿⠞⠀⠀⡴⣷⣿⠅⠀⣿⣿⢸⣾⡇⢻⣿⣿⣿⣿⣇⠸⣿⣿\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⣿⠀⠀⠀⠀⠀⠀⢰⡿⠁⣸⣿⡟⢠⢀⢠⢀⣿⣿⣿⡿⠋⠀⠀⠀⠀⠀⢰⣿⠟⠀⢠⣿⣿⡟⠀⢀⣼⣿⣿⡿⠀⢰⣿⣿⡟⣿⣇⢸⣿⣿⣿⣿⣿⠀⣿⣿\n");
    console_write("⠀⠀⠀⠀⢰⡾⢶⣀⣥⣤⡀⠀⠀⠀⢰⡟⠁⣴⣿⣿⣧⡟⣾⢋⣼⣿⡿⠋⠀⠀⠀⠀⠀⠀⢠⣿⠏⠀⠀⣼⣿⣿⠀⢀⣾⣿⣳⡿⠁⠀⡇⣿⣿⣿⣿⣿⡸⣿⣿⣿⣿⣿⡀⢻⡏\n");
    console_write("⠀⠀⠀⠀⢸⣇⠀⠻⣇⣸⡇⠀⠀⢀⡞⠀⣼⢿⣿⣿⣿⣹⡟⣾⡿⠋⠑⠦⠤⠤⣀⣀⣀⠤⣾⡿⠂⠀⢸⣿⣿⡇⢀⣾⡿⢱⡿⠁⠀⠐⠁⣿⣿⣿⣿⣿⣇⣿⣿⣿⣿⣿⡇⢸⡇\n");
    console_write("⠀⠀⠀⠀⠀⠙⢷⣴⠟⠋⠀⠀⠀⡾⠀⢰⠃⢈⣿⣿⣿⣿⣿⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⣄⡏⠀⠀⠀⢸⣿⡿⢀⣾⡿⣇⡾⠁⠀⠀⠀⠀⢹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⢸⡇\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠃⢰⠇⠀⠸⢸⣿⣿⣿⢋⡀⠀⠀⠀⢀⠀⠀⠀⠀⠀⠸⠃⠀⠀⠀⢸⣿⡇⣾⡿⠁⡸⢣⡀⠀⠀⠀⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⢸⡇\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢺⣠⠏⢀⡀⠀⣸⣿⣿⡏⢀⣹⣶⣶⣶⣿⣯⡒⢆⡀⠀⠀⠀⠀⠀⠀⢸⣿⢳⡿⠀⢰⠇⠀⠙⢦⡀⢀⠀⠀⠸⣿⣿⣿⡿⣿⣿⣿⣿⣿⡃⣼⠁\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡴⠋⣠⠞⠀⣼⢿⣿⣿⣱⡿⠋⠁⢠⣴⣿⣿⣿⣧⡀⠀⠀⠀⠀⠀⠀⠸⣿⣿⠃⠀⠀⠀⠀⠀⠀⠙⢮⡄⠀⠀⢻⣿⣿⣿⢿⣿⣿⣿⣿⠀⣿⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡾⣁⠞⠁⢀⣾⣿⢸⣿⡿⡿⠀⠀⣰⣿⣿⣿⣿⡟⢻⡇⠀⠀⠀⠀⠀⠀⠀⢻⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠲⣀⠈⢿⣿⣿⠈⣿⣿⣿⣿⢠⣿⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⠟⠁⢀⣴⣿⣿⣿⣾⣿⣿⠃⠀⢠⡟⠉⠙⠛⢻⣷⣿⡇⠀⠀⠀⠀⠀⠀⠀⠸⠃⠀⠀⠀⠀⢠⣤⣀⠀⡀⠀⠀⠀⠙⠚⢿⣿⣧⠸⣿⣿⣿⣾⣷⢰\n");
    console_write("⠀⠀⠀⠀⠀⠀⢀⣴⡿⢛⡥⢀⣴⣿⣿⣿⠟⠀⢹⣿⣧⠀⠀⠘⢇⠀⣀⡀⣸⣿⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡠⣿⣿⣷⣿⣤⡀⠀⠀⠀⠈⢿⣿⣆⠸⣿⣿⣿⣿⣾\n");
    console_write("⠀⠀⠀⢀⣠⠾⠋⢉⣴⣫⣶⣿⣿⡿⠋⠁⠀⠀⠀⣿⣿⡄⠀⠈⢿⡀⠈⠉⢁⡼⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣾⣿⣿⣿⣿⣿⣿⣷⣧⡀⠀⠈⢻⣿⣆⠹⣿⣿⣿⣿\n");
    console_write("⠀⠀⠀⠈⢱⣄⣾⣯⣿⣿⠟⠋⠀⠀⠀⠀⠀⣠⣼⠃⠙⢷⠀⠐⠚⠿⠖⠚⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⣿⠛⠿⢿⣿⡏⢙⡏⠻⣿⣆⠀⠀⠻⣿⣷⣿⣿⣿⣿\n");
    console_write("⡀⠀⠀⣠⣿⣿⣿⠟⣹⣵⣤⣤⣤⣤⣤⣤⡼⢿⡏⠀⠀⠀⠙⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⡃⠀⠀⠀⠀⢘⣿⣿⠇⠀⠹⣿⡆⠀⠀⢸⣏⣿⣿⣿⣿\n");
    console_write("⣡⣴⣾⣻⣿⠟⣩⣾⣿⣿⣿⣿⣿⣿⣿⡏⠀⣸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢉⣳⠀⢤⣀⣠⠞⢉⡿⠀⠀⠀⢹⣷⠀⢀⣾⣾⣿⣿⣿⣿\n");
    console_write("⣿⣿⢣⣿⣗⣼⣿⣿⡿⠿⢿⡟⠛⠛⠛⣷⠞⠋⠛⢦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣧⠀⠀⠀⠀⣠⠟⠀⠀⠀⠀⣸⠇⢀⣾⣾⣿⢿⣿⣿⣿\n");
    console_write("⣿⣿⠠⣿⠿⣿⡁⠈⠀⠀⠀⠑⣄⣠⠞⠁⠀⠀⠀⣀⡹⠶⠤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⢿⣒⠖⠋⠁⠀⠀⠀⠀⠞⠁⣰⣿⣿⠟⢡⣿⡿⠋⠀\n");
    console_write("⡋⠙⣸⠃⠀⠈⠻⣦⠀⢀⡴⣶⠟⠁⠀⠀⣀⡴⠚⠁⠀⠀⠀⠙⣆⠀⠀⠀⢦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣾⡿⢟⡥⣺⣿⠟⠀⠊⠀\n");
    console_write("⠀⠀⠃⠀⠀⠀⠀⢈⡿⠋⣰⠇⠀⠀⠐⠚⠁⠀⠀⠀⠀⣠⠴⠛⠋⠓⣆⠀⠈⢇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⣛⣻⠵⠚⢁⡴⠟⠁⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⣰⠏⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⣠⠖⠋⠀⠀⠀⠀⠀⢸⣦⠀⠈⠳⠤⠴⠖⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⠊⠉⠉⠀⢀⣤⠶⠭⠄⠀⠀⠀⠀⠀⢀\n");
    console_write("⠀⠀⠀⠀⠀⡼⠁⠀⠀⠀⠘⢦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⠞⠉⠈⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣶⠋⠁⠀⠀⠀⠀⢀⣀⣀⠐⠃\n");
    console_write("⠀⠀⠀⠀⡼⠁⠀⠀⠀⠀⠀⡎⠳⣄⠀⠀⠀⠀⠀⠀⠀⠀⠖⠋⠀⢀⠀⠀⢸⠀⠀⠀⡤⠤⠤⣤⡴⠒⠒⠒⢦⡖⠒⠲⠤⣄⣀⡀⠀⣀⣤⣾⡋⠉⠓⠶⠖⠚⠛⠉⠁⠀⠀⠀⢀\n");
    console_write("⠀⠀⠀⡸⠁⠀⠀⠀⠀⠀⠀⡇⠀⠈⢣⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠿⣦⡞⠀⢠⡞⠀⠀⠀⠈⡇⠀⠀⠀⢸⠇⠀⠀⠀⡇⠈⠉⠙⢿⣿⣿⣿⣆⠀⠀⠀⠀⢠⠀⠀⣠⣀⣠⣿\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⡇⠀⡇⠀⢸⡓⢤⡀⠀⠀⠀⠀⠀⠀⢀⡴⢻⠉⢹⣿⠇⠀⠀⠀⠀⡇⠀⠀⠀⡸⠀⠀⠀⢰⡇⠀⠀⠀⢸⣿⣿⡿⠟⠳⠤⠖⠶⠤⣤⣴⣿⣌⢿⣿\n");
    console_write("⠀⠀⠀⠀⠀⠀⢀⠴⠋⠉⠁⡇⠀⠀⠀⠀⠇⠀⠉⠑⠶⢤⡤⠴⠞⠉⠄⣼⣠⠾⠿⠀⠀⠀⠀⠀⠃⠀⠀⠀⠃⠀⠀⠀⠸⠁⠀⠀⢰⠟⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠦⡹\n");
    console_write("⠠⠂⠀⠀⠀⠀⢸⠀⠀⠀⠀⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣲⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⢆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡏⠘⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⠃⠀⠈⠳⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠁⠀⠀⠀⠀⠀⠙⠓⠤⢤⣤⣀⣀⣀⣀⣀⣠⡤⢖⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠁⠀⠀⠀⠀⠀⠀⠀⠀⣀⠀⠀⠀⠀⢀⠀⠀⢀⡔⠉⠀⠀⢀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_set_scale(2);
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

void cmd_miko(void) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣠⣤⣤⣤⣀⣀⡀⠀⠀⠀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⢄⢀⠀⡀⣠⣴⣾⣿⠿⠟⠛⠛⠛⠿⠿⠿⠽⠿⠿⠿⠿⠿⢶⣦⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⡿⡵⣯⣾⣿⣿⠟⠉⢀⣤⠶⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠿⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣿⢫⣮⢿⣿⡿⠛⠁⠠⠔⢋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠈⠻⣷⣦⠠⣃⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣾⢟⣵⡿⢻⡿⠋⠀⠀⣠⢴⡾⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠁⠀⠀⠈⢻⣷⣏⢫⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⣿⢸⣿⣠⠋⠀⡠⣪⢞⡵⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⣿⣧⡻⣷⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡟⡙⢾⣿⣸⡟⠁⠀⢠⠞⠁⠋⠀⠀⡴⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣿⣷⡹⣷⡄⠀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠁⡇⢸⣿⠋⠀⢀⡶⠃⠀⠀⢠⠃⠜⠁⠀⣀⠀⠀⠀⠀⢀⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠨⣿⠏⣿⣇⡀⠀⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡏⠀⡄⡼⠁⠀⢀⣾⠁⠀⠀⠀⡜⡸⠀⠀⣰⢻⠀⠀⠀⠀⢸⠉⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣆⠀⠀⠀⠀⠀⢿⢸⣿⢻⢱⡄⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠁⢀⡟⠀⢠⢃⣾⠇⠀⠀⠀⢰⠁⠀⠀⣰⢃⢸⡄⡄⠀⠀⢸⢀⠘⢧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⡄⢸⠀⠀⠀⢸⣾⠃⠈⠈⡇⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡎⢠⠞⠀⡰⠃⣼⠏⠀⠀⠀⠀⡜⠀⠀⢰⠃⠉⠸⡇⣧⠀⠀⢸⠈⢦⠘⢷⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⢱⡀⠁⠀⠀⢸⡿⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⣵⠃⠀⢰⠁⣼⠏⠀⠀⠀⠀⢀⠇⠀⢠⠟⠒⠒⠴⣷⣿⡀⠀⢸⠂⠀⠱⣄⠻⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⢧⠀⠀⠀⠈⠀⠀⠀⠀⠣⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⣠⡾⢋⠀⢀⠇⣸⠏⠀⠀⠀⠀⠀⡸⠄⢀⠎⠀⠀⠀⠀⠸⣇⢣⠀⢸⡄⠀⠀⠈⡴⠞⣆⠀⠀⠀⠀⠀⠀⠀⠀⠘⡆⠀⠀⠀⠀⠀⠀⠀⢰⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⢀⡾⢟⡇⡘⠃⡜⣼⣯⠀⠀⠀⠀⠀⠀⡇⢀⡞⠀⠀⠀⠀⠀⠀⢻⠈⢧⠸⡇⠀⠀⠀⠀⠀⠙⢆⠀⠀⠀⠀⠀⠀⠀⠀⢹⡀⠀⠀⠀⠀⠀⠀⢸⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠐⠉⠀⡸⠀⠃⢰⣻⣯⠇⠀⠀⠀⠀⠀⢸⢁⣾⣤⣀⠀⠀⠀⠀⠀⠀⠀⠈⢷⡇⠀⠀⠀⠀⠀⠀⠈⠧⡀⠀⢠⠀⠀⠀⠀⠀⢧⠀⠀⠀⠀⠀⠀⢸⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⢀⣷⢣⠎⠀⠀⠀⠀⠀⠀⢹⡞⠀⠉⠛⠿⣶⣬⡒⠄⠀⠀⠀⠈⠋⠀⠀⠀⣀⡀⠀⣀⣀⣼⣦⡀⢇⠀⠀⠀⠀⠸⣧⠀⠀⠀⠀⠀⢸⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⣼⣯⠏⢀⣀⡀⠀⠀⠀⠀⢸⡇⢰⠿⠿⠿⠟⠛⠛⠃⠀⠀⠀⠀⠀⠀⠀⣯⣴⣾⡿⠛⠛⠉⠉⠳⡞⡆⠀⢀⣀⢰⠻⣆⠀⠀⠀⠀⠈⠀⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⢸⠀⠀⣿⠋⠀⡜⠀⡇⠀⠀⠀⠀⢸⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠙⠛⠿⢷⣶⠀⡼⠀⠹⣀⣺⠈⡏⠇⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⠀⡏⠀⠜⠁⠀⢀⠃⢰⠃⠀⣰⡆⠀⢸⠉⢣⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⢧⣄⣴⠋⢿⡀⢹⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⢠⠇⠀⠀⠀⠀⡞⠀⢸⢀⡞⠁⣸⠀⢸⣄⠀⢣⡀⠀⠀⠀⠀⠀⡤⠤⠤⢄⣀⠀⠀⠀⠀⠀⠀⠀⢀⠞⡟⡎⡧⠨⠀⠀⡇⠈⡇⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⢸⠀⠀⠀⠀⢠⠇⢀⣿⠏⠀⡰⢹⠀⢸⠈⠳⣤⣥⣄⣀⣀⣀⡀⠣⡀⠀⢀⠔⠀⠀⠀⠀⠀⣀⡴⠃⣼⡇⢠⠛⡄⠀⠀⢡⠀⢺⠀⠀⠀⠀⠀⠀⢰⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⠀⡞⠀⠀⠀⠀⡼⠀⢸⠃⠀⢠⠃⠀⡆⢸⠀⠀⠈⠉⢻⣿⣿⣿⣿⡇⠈⠉⠁⠀⠀⠀⣠⠶⠾⠧⠄⠐⣻⢠⠃⠀⠱⡀⠀⠘⡄⠈⣆⠀⠀⠀⠀⠀⢸⠀⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⠀⣰⣣⠤⠎⠉⠽⠁⠀⠘⠀⡲⠏⠀⠀⢱⢸⠀⠀⠀⠀⠀⠘⠀⣷⢄⠑⠦⠤⠒⢂⣩⡮⡞⠀⠀⠀⠀⢠⣧⠃⠀⠀⠀⢣⠠⠀⠃⠀⠘⠍⠙⢦⠤⣕⠾⡄⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⠀⢰⢹⡀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠀⣄⡀⠈⢿⠀⠀⠀⣰⢞⠇⠀⠘⠀⠙⣢⣠⠖⠉⠈⠀⠀⠈⢢⣀⠀⠰⠃⠀⠀⢀⡴⡞⠉⠀⠀⠀⠀⠀⠀⠈⠀⢸⠎⡇⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⢀⣾⣄⢧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣝⠢⡌⠀⠀⢸⣿⡆⠀⠀⠀⠀⢀⡨⠶⢀⡀⠀⣠⢦⡀⣸⣿⠀⠀⠀⣠⢞⣩⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠞⡼⢱⠀⠀⠀⠀\n");
    console_write("⠀⠀⠀⣼⣿⣾⣷⣍⠲⢄⡀⠀⠀⠀⠀⠀⢀⣾⣿⡷⣌⢢⡀⣾⠛⣿⡀⠀⠀⣠⡏⠀⠀⠀⢱⣘⢧⣠⢿⡏⢻⡧⢠⠟⡵⢿⣿⣧⠀⠀⠀⠀⠀⠀⣀⡤⢞⣡⣾⣾⣌⣆⠀⠀⠀\n");
    console_write("⠀⠀⢰⣿⣿⣿⣿⣿⣿⣶⣬⡑⠲⢤⣀⣠⣾⣿⣿⠃⠈⢢⢻⡇⠀⠙⠷⠞⠋⠉⢷⠀⠀⢠⡋⠛⠻⠻⠟⠁⠈⣻⢇⠞⠁⣹⣿⣿⣷⣄⣀⠤⠖⣋⣥⣶⣿⣿⣿⣿⣿⣼⡄⠀⠀\n");
    console_write("⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣬⣌⣉⠙⠛⠓⠒⠚⣠⠏⠀⠀⠀⠀⠀⠀⢸⠀⠀⢸⡀⠀⠀⠀⠀⠀⠀⢿⡘⠒⠒⠚⠛⠋⣉⣨⣤⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀\n");
    console_write("⢀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣭⣽⠿⣹⠁⠀⠀⠀⠀⠀⠀⠀⢸⠀⠀⠸⡇⠀⠀⠀⠀⠀⠀⠀⠸⣿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢧⠀\n");
    console_write("⢾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋⡴⡏⠀⠀⠀⠀⠀⠀⠀⠀⡟⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠈⣿⡄⠙⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣯⢇\n");
    console_write("⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋⢠⢰⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⡇⠀⢠⣀⣀⣀⣀⣦⠀⢹⣧⠀⠈⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣮\n");
    console_write("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠏⠀⠀⠘⣿⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⡇⠀⠀⠉⠉⠉⠉⠁⠀⠀⢿⣶⠀⠀⠈⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
    console_write("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠁⠀⠀⠀⢰⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠁⠀⠀⠀⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠀⠀⠀⠀⠈⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

void cmd_snake(void) {
    // Change current TTY to snake game mode
    tty_change_mode(TTY_MODE_GAME,
                   snake_update,
                   snake_draw,
                   snake_input,
                   snake_special_input);
    snake_init();
}

void cmd_tetris(void) {
    // Change current TTY to tetris game mode
    tty_change_mode(TTY_MODE_GAME,
                   tetris_update,
                   tetris_draw,
                   tetris_input,
                   tetris_special_input);
    tetris_init();
}

void cmd_meminfo(void) {
    console_write("\n╔══════════════ Memory Information ══════════════╗\n");
    
    // Physical Memory
    console_write("  Physical Memory:\n");
    console_write("    Total:  ");
    uint64_t total_mb = pmm_get_total_memory() / (1024 * 1024);
    if (total_mb >= 1000) console_putchar('0' + (total_mb / 1000));
    if (total_mb >= 100) console_putchar('0' + ((total_mb / 100) % 10));
    if (total_mb >= 10) console_putchar('0' + ((total_mb / 10) % 10));
    console_putchar('0' + (total_mb % 10));
    console_write(" MB\n");
    
    console_write("    Used:   ");
    uint64_t used_mb = pmm_get_used_memory() / (1024 * 1024);
    if (used_mb >= 1000) console_putchar('0' + (used_mb / 1000));
    if (used_mb >= 100) console_putchar('0' + ((used_mb / 100) % 10));
    if (used_mb >= 10) console_putchar('0' + ((used_mb / 10) % 10));
    console_putchar('0' + (used_mb % 10));
console_write(" MB\n");
    
    console_write("    Free:   ");
    uint64_t free_mb = pmm_get_free_memory() / (1024 * 1024);
    if (free_mb >= 1000) console_putchar('0' + (free_mb / 1000));
    if (free_mb >= 100) console_putchar('0' + ((free_mb / 100) % 10));
    if (free_mb >= 10) console_putchar('0' + ((free_mb / 10) % 10));
    console_putchar('0' + (free_mb % 10));
    console_write(" MB\n\n");
    
    // Heap Memory
    console_write("  Kernel Heap:\n");
    console_write("    Used:   ");
    size_t heap_used_kb = heap_get_used() / 1024;
    if (heap_used_kb >= 10000) console_putchar('0' + (heap_used_kb / 10000));
    if (heap_used_kb >= 1000) console_putchar('0' + ((heap_used_kb / 1000) % 10));
    if (heap_used_kb >= 100) console_putchar('0' + ((heap_used_kb / 100) % 10));
    if (heap_used_kb >= 10) console_putchar('0' + ((heap_used_kb / 10) % 10));
    console_putchar('0' + (heap_used_kb % 10));
    console_write(" KB\n");
    
    console_write("    Free:   ");
    size_t heap_free_kb = heap_get_free() / 1024;
    if (heap_free_kb >= 10000) console_putchar('0' + (heap_free_kb / 10000));
    if (heap_free_kb >= 1000) console_putchar('0' + ((heap_free_kb / 1000) % 10));
    if (heap_free_kb >= 100) console_putchar('0' + ((heap_free_kb / 100) % 10));
    if (heap_free_kb >= 10) console_putchar('0' + ((heap_free_kb / 10) % 10));
    console_putchar('0' + (heap_free_kb % 10));
    console_write(" KB\n");
    
    console_write("╚════════════════════════════════════════════════╝\n");
}


void cmd_ls(const char* args) {
    vfs_node_t* dir;
    
    if (args && args[0] != '\0') {
        // List specific directory
        dir = vfs_get_node(args);
        if (!dir) {
            console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
            console_write("\nDirectory not found: ");
            console_write(args);
            console_write("\n");
            console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
            return;
        }
    } else {
        // List current directory
        dir = vfs_get_cwd();
    }
    
    if (!dir || dir->type != VFS_DIRECTORY) {
        console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
        console_write("\nNot a directory\n");
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
        return;
    }
    
    console_write("\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("Directory listing:\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    console_write("╔════════════════════════════════════════════════════╗\n");
    
    uint32_t index = 0;
    vfs_node_t* node;
    int count = 0;
    
    while ((node = vfs_readdir(dir, index++)) != NULL) {
        count++;
        
        // Show type indicator
        if (node->type == VFS_DIRECTORY) {
            console_set_colors(100, 100, 255, 0, 0, 0);
            console_write("  [DIR]  ");
        } else {
            console_set_colors(200, 200, 200, 0, 0, 0);
            console_write("  [FILE] ");
        }
        
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
        console_write(node->name);
        
        // Show size for files
        if (node->type == VFS_FILE) {
            console_write(" (");
            
            // Print size
            uint32_t size = node->size;
            if (size >= 1024 * 1024) {
                console_putchar('0' + (size / (1024 * 1024)));
                console_write(" MB");
            } else if (size >= 1024) {
                uint32_t kb = size / 1024;
                if (kb >= 1000) console_putchar('0' + (kb / 1000));
                if (kb >= 100) console_putchar('0' + ((kb / 100) % 10));
                if (kb >= 10) console_putchar('0' + ((kb / 10) % 10));
                console_putchar('0' + (kb % 10));
                console_write(" KB");
            } else {
                if (size >= 1000) console_putchar('0' + (size / 1000));
                if (size >= 100) console_putchar('0' + ((size / 100) % 10));
                if (size >= 10) console_putchar('0' + ((size / 10) % 10));
                console_putchar('0' + (size % 10));
                console_write(" B");
            }
            console_write(")");
        }
        
        console_write("\n");
    }
    
    console_write("╚════════════════════════════════════════════════════╝\n");
    console_write("Total: ");
    if (count >= 10) console_putchar('0' + (count / 10));
    console_putchar('0' + (count % 10));
    console_write(" items\n");
}

void cmd_cat(const char* args) {
    if (!args || args[0] == '\0') {
        console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
        console_write("\nUsage: cat <filename>\n");
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
        return;
    }
    
    // Open file
    int fd = vfs_open(args, O_RDONLY);
    if (fd < 0) {
        console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
        console_write("\nFile not found: ");
        console_write(args);
        console_write("\n");
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
        return;
    }
    
    console_write("\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("╔══════════════ ");
    console_write(args);
    console_write(" ══════════════╗\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    
    // Read and display file contents
    char buffer[512];
    int bytes_read;
    
    while ((bytes_read = vfs_read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        console_write(buffer);
    }
    
    console_write("\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("╚═══════════════════════════════════════════════════╝\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    
    vfs_close(fd);
}

void cmd_cd(const char* args) {
    if (!args || args[0] == '\0') {
        // Go to root
        vfs_set_cwd(vfs_get_root());
        console_write("\nChanged to root directory\n");
        return;
    }
    
    vfs_node_t* node = vfs_get_node(args);
    
    if (!node) {
        console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
        console_write("\nDirectory not found: ");
        console_write(args);
        console_write("\n");
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
        return;
    }
    
    if (node->type != VFS_DIRECTORY) {
        console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
        console_write("\nNot a directory: ");
        console_write(args);
        console_write("\n");
        console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
        return;
    }
    
    vfs_set_cwd(node);
    console_write("\nChanged directory to: ");
    console_write(node->name);
    console_write("\n");
}

void cmd_pwd(void) {
    char path[256];
    vfs_get_cwd_path(path, sizeof(path));
    
    console_write("\nCurrent directory: ");
    console_set_colors(100, 100, 255, 0, 0, 0);
    console_write(path);
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
    console_write("\n");
}
