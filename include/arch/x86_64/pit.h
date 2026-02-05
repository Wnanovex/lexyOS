#ifndef PIT_H
#define PIT_H

#include <stdint.h>

// PIT Ports
#define PIT_CHANNEL0_DATA 0x40
#define PIT_CHANNEL1_DATA 0x41
#define PIT_CHANNEL2_DATA 0x42
#define PIT_COMMAND       0x43

// PIT Commands
#define PIT_BCD_MODE      0x01
#define PIT_MODE0         0x00
#define PIT_MODE2         0x04
#define PIT_MODE3         0x06
#define PIT_LOBYTE        0x10
#define PIT_HIBYTE        0x20
#define PIT_CHANNEL0      0x00
#define PIT_CHANNEL1      0x40
#define PIT_CHANNEL2      0x80

// Constants
#define PIT_FREQUENCY     1193182
#define PIT_DEFAULT_DIV   1000
#define PIT_TICKS_PER_SEC 1000

// Global Variables
extern volatile uint32_t pit_ticks;

// Function Declarations
void pit_init(uint32_t frequency);
void pit_sleep(uint32_t ms);
uint32_t pit_get_ticks(void);
void pit_set_frequency(uint32_t frequency);
void pit_handler(void);

#endif // PIT_H
