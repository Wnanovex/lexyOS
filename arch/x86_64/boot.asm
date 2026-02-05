; -----------------------------------------------------------------------------
; Multiboot2 x86_64 boot stub (NASM)
; GRUB -> protected mode -> long mode -> kernel_main
; Identity maps 0–4 GiB using 4 PDs (2 MiB pages)
; -----------------------------------------------------------------------------

%define MULTIBOOT2_MAGIC          0xE85250D6
%define MULTIBOOT2_BOOTLOADER     0x36D76289

%define STACK_SIZE               16384
%define PAGE_SIZE                0x1000

%define PAGING_BASE              0x200000
%define PML4_ADDR                (PAGING_BASE + 0x0000)
%define PDP_ADDR                 (PAGING_BASE + 0x1000)
%define PD0_ADDR                 (PAGING_BASE + 0x2000)
%define PD1_ADDR                 (PAGING_BASE + 0x3000)
%define PD2_ADDR                 (PAGING_BASE + 0x4000)
%define PD3_ADDR                 (PAGING_BASE + 0x5000)

%define PF_PRESENT               0x1
%define PF_WRITABLE              0x2
%define PF_PS                    (1 << 7)

%define CR4_PAE                  (1 << 5)
%define CR0_PG                   (1 << 31)
%define CR0_WP                   (1 << 16)
%define EFER_MSR                 0xC0000080
%define EFER_LME                 (1 << 8)

%define VGA_BUFFER               0xB8000
%define VGA_ATTR_ERR             0x4F

; -----------------------------------------------------------------------------
; Multiboot2 header (must be within first 32 KiB)
; -----------------------------------------------------------------------------
section .multiboot_header
align 8

mb_header_start:
    dd MULTIBOOT2_MAGIC
    dd 0                            ; architecture (i386)
    dd mb_header_end - mb_header_start
    dd -(MULTIBOOT2_MAGIC + (mb_header_end - mb_header_start))

    ; Framebuffer request tag
    align 8
    dw 5                            ; type
    dw 0                            ; flags
    dd 20                           ; size
    dd 1024                         ; width
    dd 768                          ; height
    dd 32                           ; depth

    ; End tag
    align 8
    dw 0
    dw 0
    dd 8

mb_header_end:

; -----------------------------------------------------------------------------
; BSS
; -----------------------------------------------------------------------------
section .bss
align 16
stack_bottom:
    resb STACK_SIZE
stack_top:

align 8
multiboot_info_ptr:
    resq 1

; -----------------------------------------------------------------------------
; 32-bit entry (GRUB enters here)
; -----------------------------------------------------------------------------
section .text
bits 32
global _start

_start:
    cli
    mov esp, stack_top

    ; Save Multiboot2 info pointer
    mov dword [multiboot_info_ptr], ebx
    mov dword [multiboot_info_ptr + 4], 0

    ; Validate Multiboot2 magic
    cmp eax, MULTIBOOT2_BOOTLOADER
    jne fatal_error

    ; Load 64-bit GDT (used for both PM and LM)
    lgdt [gdt_ptr]

    call setup_page_tables
    call enable_long_mode

    ; Far jump enables long mode
    jmp 0x08:long_mode_entry

; -----------------------------------------------------------------------------
; Page tables: PML4 -> PDP -> 4×PD (4 GiB identity map)
; -----------------------------------------------------------------------------
setup_page_tables:
    ; Clear PML4 + PDP + 4 PDs = 6 pages
    mov edi, PAGING_BASE
    mov ecx, 6 * (PAGE_SIZE / 4)
    xor eax, eax
    rep stosd

    ; PML4[0] -> PDP
    mov eax, PDP_ADDR | PF_PRESENT | PF_WRITABLE
    mov [PML4_ADDR], eax

    ; PDP[0..3] -> PDs
    mov eax, PD0_ADDR | PF_PRESENT | PF_WRITABLE
    mov [PDP_ADDR + 0*8], eax
    mov eax, PD1_ADDR | PF_PRESENT | PF_WRITABLE
    mov [PDP_ADDR + 1*8], eax
    mov eax, PD2_ADDR | PF_PRESENT | PF_WRITABLE
    mov [PDP_ADDR + 2*8], eax
    mov eax, PD3_ADDR | PF_PRESENT | PF_WRITABLE
    mov [PDP_ADDR + 3*8], eax

    ; Map 4 GiB using 2 MiB pages
    xor ebx, ebx

%macro MAP_PD 1
    mov edi, %1
    mov ecx, 512
%%loop:
    mov eax, ebx
    shl eax, 21
    or eax, PF_PRESENT | PF_WRITABLE | PF_PS
    mov [edi], eax
    mov dword [edi + 4], 0
    add edi, 8
    inc ebx
    loop %%loop
%endmacro

    MAP_PD PD0_ADDR
    MAP_PD PD1_ADDR
    MAP_PD PD2_ADDR
    MAP_PD PD3_ADDR

    ret

; -----------------------------------------------------------------------------
; Enable long mode
; -----------------------------------------------------------------------------
enable_long_mode:
    mov eax, PML4_ADDR
    mov cr3, eax

    mov eax, cr4
    or eax, CR4_PAE
    mov cr4, eax

    mov ecx, EFER_MSR
    rdmsr
    or eax, EFER_LME
    wrmsr

    mov eax, cr0
    or eax, CR0_PG | CR0_WP
    mov cr0, eax

    ret

; -----------------------------------------------------------------------------
; Fatal error (32-bit VGA)
; -----------------------------------------------------------------------------
fatal_error:
    mov edi, VGA_BUFFER
    mov ah, VGA_ATTR_ERR
    mov al, 'E'
    stosw
    mov al, 'R'
    stosw
    mov al, 'R'
    stosw
.hang:
    cli
    hlt
    jmp .hang

; -----------------------------------------------------------------------------
; 64-bit entry point
; -----------------------------------------------------------------------------
bits 64
default rel
long_mode_entry:
    ; Load data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; ABI-aligned stack
    mov rsp, stack_top
    and rsp, -16
    sub rsp, 8

    ; Pass Multiboot2 info pointer
    mov rdi, [multiboot_info_ptr]

    extern kernel_main
    call kernel_main

.halt:
    cli
    hlt
    jmp .halt

; -----------------------------------------------------------------------------
; GDT (64-bit only)
; -----------------------------------------------------------------------------
section .rodata
align 8

gdt_start:
    dq 0x0000000000000000        ; Null
    dq 0x00209A0000000000        ; 64-bit code
    dq 0x0000920000000000        ; Data
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dq gdt_start

