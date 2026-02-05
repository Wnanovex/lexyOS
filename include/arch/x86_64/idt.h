#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Constants
#define IDT_ENTRIES 256
#define KERNEL_CS 0x08

// Gate types
#define IDT_FLAG_PRESENT     0x80
#define IDT_FLAG_RING0       0x00
#define IDT_FLAG_RING3       0x60
#define IDT_FLAG_TRAP_GATE   0x0F
#define IDT_FLAG_INT_GATE    0x0E
#define IDT_FLAG_TASK_GATE   0x05

// Structure Definitions
typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_ptr_t;

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) registers_t;

// Public Function Declarations
void idt_init(void);
uint32_t get_timer_ticks(void);
void sleep_ms(uint32_t ms);

// Assembly Function Declarations
extern void idt_load(uint64_t idt_ptr);
extern void isr_handler(registers_t* regs);
extern void irq_handler(registers_t* regs);

#endif // IDT_H
