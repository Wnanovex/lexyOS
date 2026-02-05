#include <arch/x86_64/irq.h>
#include <arch/x86_64/pic.h>
#include <ui/console.h>

static void (*irq_handlers[16])(registers_t*) = {0};

void irq_init(void) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("[IRQ] Initializing...\n");
    
    // Clear handlers
    for (int i = 0; i < 16; i++) {
        irq_handlers[i] = 0;
    }
    
    // Enable IRQs
    pic_clear_mask(0);  // PIT
    pic_clear_mask(1);  // Keyboard
    
    console_write("[IRQ] Handlers initialized\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

void irq_install_handler(int irq, void (*handler)(registers_t*)) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void irq_uninstall_handler(int irq) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = 0;
    }
}
