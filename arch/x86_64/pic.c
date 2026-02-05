#include <arch/x86_64/pic.h>
#include <arch/x86_64/ports.h>
#include <ui/console.h>

void pic_init(void) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("[PIC] Initializing...\n");
    
    // Save current masks
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);
    
    // Initialize PICs
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    // Set vector offsets
    outb(PIC1_DATA, 0x20);  // IRQ 0-7 -> INT 0x20-0x27
    io_wait();
    outb(PIC2_DATA, 0x28);  // IRQ 8-15 -> INT 0x28-0x2F
    io_wait();
    
    // Setup cascade
    outb(PIC1_DATA, 0x04);  // Master has slave at IRQ2
    io_wait();
    outb(PIC2_DATA, 0x02);  // Slave identity
    io_wait();
    
    // Set 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    
    // Restore masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
    
    console_write("[PIC] Initialized\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

void pic_disable(void) {
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}

void pic_set_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t value = inb(port) | (1 << (irq & 7));
    outb(port, value);
}

void pic_clear_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t value = inb(port) & ~(1 << (irq & 7));
    outb(port, value);
}
