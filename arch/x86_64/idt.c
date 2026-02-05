#include <arch/x86_64/idt.h>
#include <arch/x86_64/pic.h>
#include <arch/x86_64/pit.h>
#include <arch/x86_64/ports.h>
#include <ui/console.h>
#include <drivers/input/keyboard.h>

// Static Variables
static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

// Assembly Function Declarations
extern void isr0(void); extern void isr1(void); extern void isr2(void); extern void isr3(void);
extern void isr4(void); extern void isr5(void); extern void isr6(void); extern void isr7(void);
extern void isr8(void); extern void isr9(void); extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);
extern void isr20(void); extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);

extern void irq0(void); extern void irq1(void); extern void irq2(void); extern void irq3(void);
extern void irq4(void); extern void irq5(void); extern void irq6(void); extern void irq7(void);
extern void irq8(void); extern void irq9(void); extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void); extern void irq15(void);

// Internal Function Declarations
static void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);
static const char* get_exception_name(uint8_t int_no);
static void dump_registers(registers_t* regs);
static void* get_isr_address(int i);
static void* get_irq_address(int i);

// Internal Function Definitions

static void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_mid = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}

static const char* get_exception_name(uint8_t int_no) {
    static const char* exceptions[32] = {
        "Divide by zero", "Debug", "NMI", "Breakpoint", "Overflow",
        "Bound range exceeded", "Invalid opcode", "Device not available",
        "Double fault", "Coprocessor overrun", "Invalid TSS", "Segment not present",
        "Stack-segment fault", "General protection fault", "Page fault", "Reserved",
        "Floating point", "Alignment check", "Machine check", "SIMD floating point",
        "Virtualization", "Control protection", "Reserved", "Reserved",
        "Reserved", "Reserved", "Reserved", "Reserved", "Hypervisor injection",
        "VMM communication", "Security", "Reserved"
    };
    return (int_no < 32) ? exceptions[int_no] : "Unknown";
}

static void dump_registers(registers_t* regs) {
    console_write("Registers:\n");
    console_write("RAX: "); console_write_hex64(regs->rax); console_write("  ");
    console_write("RBX: "); console_write_hex64(regs->rbx); console_write("  ");
    console_write("RCX: "); console_write_hex64(regs->rcx); console_write("\n");
    console_write("RDX: "); console_write_hex64(regs->rdx); console_write("  ");
    console_write("RSI: "); console_write_hex64(regs->rsi); console_write("  ");
    console_write("RDI: "); console_write_hex64(regs->rdi); console_write("\n");
    console_write("RIP: "); console_write_hex64(regs->rip); console_write("\n");
    console_write("RSP: "); console_write_hex64(regs->rsp); console_write("\n");
}

static void* get_isr_address(int i) {
    static void* isr_table[32] = {
        (void*)isr0, (void*)isr1, (void*)isr2, (void*)isr3,
        (void*)isr4, (void*)isr5, (void*)isr6, (void*)isr7,
        (void*)isr8, (void*)isr9, (void*)isr10, (void*)isr11,
        (void*)isr12, (void*)isr13, (void*)isr14, (void*)isr15,
        (void*)isr16, (void*)isr17, (void*)isr18, (void*)isr19,
        (void*)isr20, (void*)isr21, (void*)isr22, (void*)isr23,
        (void*)isr24, (void*)isr25, (void*)isr26, (void*)isr27,
        (void*)isr28, (void*)isr29, (void*)isr30, (void*)isr31
    };
    return (i >= 0 && i < 32) ? isr_table[i] : NULL;
}

static void* get_irq_address(int i) {
    static void* irq_table[16] = {
        (void*)irq0, (void*)irq1, (void*)irq2, (void*)irq3,
        (void*)irq4, (void*)irq5, (void*)irq6, (void*)irq7,
        (void*)irq8, (void*)irq9, (void*)irq10, (void*)irq11,
        (void*)irq12, (void*)irq13, (void*)irq14, (void*)irq15
    };
    return (i >= 0 && i < 16) ? irq_table[i] : NULL;
}

// Public Function Definitions

void idt_init(void) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_CYAN);
    console_write("[IDT] Initializing...\n");
    
    // Setup IDT Pointer
    idt_ptr.base = (uint64_t)&idt;
    idt_ptr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
    
    // Clear IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Install Exception Handlers (0-31)
    for (int i = 0; i < 32; i++) {
        void* isr_addr = get_isr_address(i);
        if (isr_addr) {
            idt_set_gate(i, (uint64_t)isr_addr, KERNEL_CS, 
                        IDT_FLAG_PRESENT | IDT_FLAG_INT_GATE);
        }
    }
    
    // Install IRQ Handlers (32-47)
    for (int i = 0; i < 16; i++) {
        void* irq_addr = get_irq_address(i);
        if (irq_addr) {
            idt_set_gate(32 + i, (uint64_t)irq_addr, KERNEL_CS,
                        IDT_FLAG_PRESENT | IDT_FLAG_INT_GATE);
        }
    }
    
    // Load IDT
    idt_load((uint64_t)&idt_ptr);
    
    // Initialize Subsystems
    pic_init();
    pit_init(1000);
    
    // Enable Interrupts
    __asm__ volatile("sti");
    
    console_write("[IDT] Initialized with 256 entries\n");
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

void isr_handler(registers_t* regs) {
    console_set_color_preset(CONSOLE_COLOR_PRESET_RED);
    
    if (regs->int_no < 32) {
        console_write("\n[ISR] Exception: ");
        console_write(get_exception_name(regs->int_no));
        console_write(" (");
        console_write_dec(regs->int_no);
        console_write(")\n");
        
        if (regs->int_no == 14) {
            console_write("Page fault at address: ");
            uint64_t fault_address;
            __asm__ volatile("mov %%cr2, %0" : "=r"(fault_address));
            console_write_hex64(fault_address);
            console_write("\n");
        }
        
        console_write("Error code: ");
        console_write_hex64(regs->err_code);
        console_write("\n");
        
        dump_registers(regs);
        
        if (regs->int_no <= 14) {
            console_write("System halted!\n");
            while (1) __asm__ volatile("cli; hlt");
        }
    }
    
    console_set_color_preset(CONSOLE_COLOR_PRESET_CLASSIC);
}

void irq_handler(registers_t* regs) {
    // Handle PIT
    if (regs->int_no == 32) {
        pit_handler();
    }
    // Handle Keyboard
    else if (regs->int_no == 33) {
        keyboard_handler();
    }
    
    // Send EOI to PIC
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

uint32_t get_timer_ticks(void) {
    return pit_get_ticks();
}

void sleep_ms(uint32_t ms) {
    uint32_t target = get_timer_ticks() + ms;
    while (get_timer_ticks() < target) {
        __asm__ volatile("pause");
    }
}
