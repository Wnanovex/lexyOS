ASM = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

# Compiler flags for kernel
CFLAGS = -ffreestanding -O2 -Wall -Wextra -nostdlib -lgcc -Iinclude \
         -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
         -mno-80387 -msoft-float -mgeneral-regs-only

ASMFLAGS = -f elf64 -w-implicit-abs-deprecated


# Source files
ASM_SOURCES = arch/x86_64/boot.asm arch/x86_64/isr.asm
C_SOURCES = kernel/kernel.c drivers/video/framebuffer.c drivers/serial/serial.c ui/font/font8x8.c \
					  arch/x86_64/pic.c arch/x86_64/pit.c arch/x86_64/irq.c	\
						ui/console.c ui/vga_console.c ui/fb_console.c drivers/video/vga_text.c \
						drivers/input/keyboard.c arch/x86_64/idt.c ui/tty/tty.c \
						ui/terminal_games/game_snake/game_snake.c ui/terminal_games/game_tetris/game_tetris.c \
						lib/string/string.c \
						mm/pmm.c mm/vmm.c mm/heap.c \
						ui/shell/shell.c ui/shell/shell_commands.c ui/shell/shell_history.c \
						fs/vfs.c fs/tarfs.c 

# Object files
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
C_OBJECTS = $(C_SOURCES:.c=.o)
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Targets
TARGET = kernel.elf
ISO_TARGET = dist/lexyOS.iso
INITRD = initrd.tar


.PHONY: all clean iso run debug check-multiboot

all: iso

$(TARGET): $(OBJECTS) linker.ld
	$(LD) -n -o $@ -T linker.ld $(OBJECTS)

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

iso: $(TARGET) $(INITRD)
	mkdir -p isodir/boot/grub
	mkdir -p dist
	cp $(TARGET) isodir/boot/kernel.elf
	cp $(INITRD) isodir/boot/initrd.tar
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(ISO_TARGET) isodir


# Add rule to create initrd
$(INITRD):
	@echo "Creating initrd..."
	@mkdir -p initrd
	@echo "Welcome to lexyOS!" > initrd/welcome.txt
	@echo "This is a test file." > initrd/test.txt
	cd initrd && tar -cf ../$(INITRD) *


run: 
	qemu-system-x86_64 -cdrom $(ISO_TARGET) -serial stdio

debug: iso
	qemu-system-x86_64 -cdrom $(ISO_TARGET) -s -S

clean:
	rm -f $(OBJECTS) $(TARGET) $(ISO_TARGET) $(INITRD)
	rm -rf isodir dist initrd

check-multiboot: $(TARGET)
	grub-file --is-x86-multiboot2 $(TARGET) && echo "Multiboot2 confirmed" || echo "Not multiboot2"
