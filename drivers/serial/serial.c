#include <drivers/serial/serial.h>

static int serial_initialized = 0;

/* I/O helpers */
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Check if transmit buffer is empty */
static int serial_transmit_empty(void)
{
    return inb(SERIAL_COM1 + 5) & 0x20;
}

void serial_init(void)
{
    outb(SERIAL_COM1 + 1, 0x00);    // Disable interrupts
    outb(SERIAL_COM1 + 3, 0x80);    // Enable DLAB
    outb(SERIAL_COM1 + 0, 0x03);    // Divisor low  (38400 baud)
    outb(SERIAL_COM1 + 1, 0x00);    // Divisor high
    outb(SERIAL_COM1 + 3, 0x03);    // 8 bits, no parity, one stop
    outb(SERIAL_COM1 + 2, 0xC7);    // Enable FIFO, clear, 14-byte threshold
    outb(SERIAL_COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set

    serial_initialized = 1;
}

int serial_is_initialized(void)
{
    return serial_initialized;
}

void serial_putchar(char c)
{
    if (!serial_initialized)
        return;

    while (!serial_transmit_empty())
        ;

    outb(SERIAL_COM1, c);
}

void serial_write(const char* str)
{
    while (*str) {
        if (*str == '\n')
            serial_putchar('\r');
        serial_putchar(*str++);
    }
}

void serial_write_hex64(uint64_t value)
{
    static const char* hex = "0123456789ABCDEF";
    serial_write("0x");

    for (int i = 60; i >= 0; i -= 4) {
        serial_putchar(hex[(value >> i) & 0xF]);
    }
}

void serial_write_dec(uint64_t value)
{
    char buf[21];
    int i = 0;

    if (value == 0) {
        serial_putchar('0');
        return;
    }

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i--)
        serial_putchar(buf[i]);
}

