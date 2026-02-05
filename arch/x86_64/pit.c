#include <arch/x86_64/pit.h>
#include <arch/x86_64/ports.h>
#include <ui/console.h>

volatile uint32_t pit_ticks = 0;

void pit_init(uint32_t frequency) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("[PIT] Initializing...\n");
    
    if (frequency == 0) frequency = PIT_DEFAULT_DIV;
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    // Configure PIT
    outb(PIT_COMMAND, PIT_CHANNEL0 | PIT_LOBYTE | PIT_HIBYTE | PIT_MODE3);
    outb(PIT_CHANNEL0_DATA, divisor & 0xFF);
    outb(PIT_CHANNEL0_DATA, (divisor >> 8) & 0xFF);
    
    pit_ticks = 0;
    
    console_write("[PIT] Initialized at ");
    //console_write_dec(frequency);
    console_write(" Hz\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

uint32_t pit_get_ticks(void) {
    return pit_ticks;
}

void pit_sleep(uint32_t ms) {
    uint32_t target = pit_ticks + ms;
    while (pit_ticks < target) {
        __asm__ volatile("hlt");
    }
}

void pit_set_frequency(uint32_t frequency) {
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    outb(PIT_COMMAND, PIT_CHANNEL0 | PIT_LOBYTE | PIT_HIBYTE | PIT_MODE3);
    outb(PIT_CHANNEL0_DATA, divisor & 0xFF);
    outb(PIT_CHANNEL0_DATA, (divisor >> 8) & 0xFF);
}

void pit_handler(void) {
    pit_ticks++;
}
